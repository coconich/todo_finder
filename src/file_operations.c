//=====================================================================================================================
// MIT License
//
// Copyright (c) 2025 Cory Simonich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//=====================================================================================================================

#include "common.h"

bool FileOpen(File* file, const char* filename, const char* mode)
{
    if (!file)
    {
        LogDebug("FileOpen given null File structure.\n"); 
        return false;
    }
    
#ifdef OS_Win32
    errno_t error = fopen_s(&file->fp, filename, mode);
    if (error)
    {
        LogDebug("\n    Failed to open file %s (error: %d)\n", filename, error); 
        file->fp = 0;
        return false;
    }
#else
    file->fp = fopen(filename, mode);
    if (!file->fp) 
    { 
        LogDebug("\n    Failed to open file %s (error: %d)\n", filename, strerror(errno)); 
        return false;
    }
#endif 
    
    StringCopy_NullTerminate(file->path, filename, sizeof(file->path) - 1); 
    StringCopy_NullTerminate(file->mode, mode, sizeof(file->mode) - 1); 
    return true;
}

usize FileWrite(MemoryBuffer* data, usize byte_count, File* file)
{
    if (!file)
    {
        LogDebug("FileWrite given null File structure.\n"); 
        return 0;
    }
    if(!file->fp) 
    { 
        LogDebug("FileWrite, File structure has null FILE ptr.\n"); 
        return 0;
    }

    usize size = fwrite(data->buffer, sizeof(char), byte_count, file->fp);
    return size;
}

usize FileRead(MemoryBuffer* destination, usize byte_count, File* file)
{
    if (!file)
    {
        LogDebug("FileRead given null File structure.\n"); 
        return 0;
    }
    if(!file->fp) 
    { 
        LogDebug("FileRead, File structure has null FILE ptr.\n"); 
        return 0;
    }
    usize size = fread(destination->buffer, sizeof(char), byte_count, file->fp);
    return size;
}

usize FilePuts(const char* string, File* file)
{
    if (!file)
    {
        LogDebug("FilePuts given null File structure.\n"); 
        return 0;
    }
    if(!file->fp) 
    { 
        LogDebug("FilePuts, File structure has null FILE ptr.\n"); 
        return 0;
    }
    if(!string) 
    { 
        LogDebug("FilePuts Bad string parameter.\n"); 
        return 0;
    }

    usize size = fputs(string, file->fp);
    return size;
}

void FileClose(File* file)
{
    if (!file)
    {
        LogDebug("FileClose given null File structure.\n"); 
        return;
    }
    if(!file->fp) 
    { 
        LogDebug("FileClose, File structure has null FILE ptr.\n"); 
        return;
    }
    fclose(file->fp);
}

usize GetFileContents(FileContents* file_contents, const char* filepath) 
{
    usize size = 0;
    File file = { 0, "", "" };
    
    file_contents->memory.buffer = 0;
    file_contents->memory.size = 0;
    file_contents->path[0] = '\0';
    
    bool did_open = FileOpen(&file, filepath, "rb");
    if (!did_open || !file.fp)
    {
        LogDebug("Failed to open file for bulk read\n"); 
        return 0;
    }

    fseek(file.fp, 0, SEEK_END);  
    size = ftell(file.fp);        
    fseek(file.fp, 0, SEEK_SET);  

    if(size == 0) 
    {
        FileClose(&file);
        LogDebug("fseek&ftell says file size == 0\n");
        LogDebug("File path: %s\n", filepath);
        return 0;
    }
    
    Allocate(&file_contents->memory, size + 1);  
    usize read = FileRead(&file_contents->memory, size, &file);
    if(read != size) 
    {
        LogDebug("Failed to read entire file contents");
        LogDebug("Requested %lld , Got %lld", size, read);
        FileClose(&file);
        Free(&file_contents->memory);
        return 0;
    }
    
    file_contents->memory.buffer[size] = '\0'; 
    file_contents->memory.size = size;
    StringCopy_NullTerminate(file_contents->path, filepath, MaxPath - 1);
    FileClose(&file);
    return read;
}

void FreeFileContents(FileContents* file_contents) 
{
    if(file_contents && file_contents->memory.buffer) 
    {
        Free(&file_contents->memory);
        file_contents->memory.buffer = 0;
        file_contents->memory.size = 0;
    }
}



bool DirectoryOpen(DirectoryIterator* directory_iterator, const char* directory_name) 
{
    memset(directory_iterator, 0, sizeof(DirectoryIterator));
    
#ifdef OS_Win32
    snprintf(directory_iterator->text_buffer, sizeof(directory_iterator->text_buffer), "%s\\*", directory_name);
    directory_iterator->handle = FindFirstFileA(directory_iterator->text_buffer, &directory_iterator->find_data);
    directory_iterator->first_file = true;
    if (directory_iterator->handle == INVALID_HANDLE_VALUE) { return false; }
#else
    directory_iterator->dir = opendir(directory_name);
    if (!directory_iterator->dir) { return false; }
    StringCopy_NullTerminate(directory_iterator->text_buffer, directory_name, sizeof(directory_iterator->text_buffer) - 1);
#endif

    return true;
}

bool DirectoryNextEntry(DirectoryIterator* directory_iterator, DirectoryEntry* entry) 
{
    if (!entry) return false;

#ifdef OS_Win32
    
    if (directory_iterator->first_file) 
    { 
        directory_iterator->first_file = false; 
    } 
    else 
    {
        if (!FindNextFileA(directory_iterator->handle, &directory_iterator->find_data)) 
        {
            return false;
        }
    }

    entry->name = directory_iterator->find_data.cFileName;
    
    // Build full path
    char* last_slash = strrchr(directory_iterator->text_buffer, '\\');
    if (last_slash) 
    {
        *last_slash = '\0'; // Remove the "*" part
        snprintf(entry->path, sizeof(entry->path), "%s\\%s",directory_iterator->text_buffer, entry->name);
        *last_slash = '\\'; // Restore the pattern
    } 
    else 
    {
        snprintf(entry->path, sizeof(entry->path), "%s\\%s", directory_iterator->text_buffer, entry->name);
    }

    if (directory_iterator->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
    {
        entry->type = FileType_Directory;
    } 
    else 
    {
        entry->type = FileType_File;
    }

#else

    directory_iterator->entry = readdir(directory_iterator->dir);
    if (!directory_iterator->entry) 
    {
        return false;
    }

    entry->name = directory_iterator->entry->d_name;
    snprintf(entry->path, sizeof(entry->path), "%s/%s", directory_iterator->text_buffer, entry->name);

    // Determine file type
    struct stat statbuf;
    if (stat(entry->path, &statbuf) == 0) 
    {
        if (S_ISDIR(statbuf.st_mode)) 
        {
            entry->type = FileType_Directory;
        } 
        else if (S_ISREG(statbuf.st_mode)) 
        {
            entry->type = FileType_File;
        } 
        else 
        {
            entry->type = FileType_Other;
        }
    } 
    else 
    {
        entry->type = FileType_Other;
    }
#endif

    return true;
}

void DirectoryClose(DirectoryIterator* directory_iterator) 
{
#ifdef OS_Win32
    if (directory_iterator->handle != INVALID_HANDLE_VALUE) 
    {
        FindClose(directory_iterator->handle);
        directory_iterator->handle = INVALID_HANDLE_VALUE;
    }
#else
    if (directory_iterator->dir) 
    {
        closedir(directory_iterator->dir);
        directory_iterator->dir = NULL;
    }
#endif
}