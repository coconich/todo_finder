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

static const char* default_symbols[] = 
{
    "@"
};

static const char* default_keywords[] = 
{
    "todo", "cleanup", "bug", "perf", "heap", "nocheckin", "fixme", "hack", "web", "broken"
};

static const char* default_ignore_directories[] = 
{
    ".svn", ".vs", ".git", "bin", "build", "bin-obj", "libs", "deps", "libraries", "dependencies", "dlls", "dll" 
};

static const char* default_ignore_extensions[] = 
{
    ".exe", ".o", ".obj", ".png", ".jpg", ".bmp", ".json", ".xml", ".lib", ".dll" 
};



MessageTable* AllocateMessageTable(UserConfig* user_config)
{
    MessageTable* message_table = (MessageTable*)( malloc(sizeof(MessageTable)) );
    if(!message_table) 
    { 
        LogDebug("AllocateMessageTable, failed to malloc message_table");
        return 0; 
    }
    memset(message_table, 0, sizeof(MessageTable));
    
    // default table
    if(!user_config)
    {
        StringVector_Init(&message_table->symbols);
        StringVector_Init(&message_table->keywords);
        StringVector_Init(&message_table->ignore_directories);
        StringVector_Init(&message_table->ignore_extensions);

        StringVector_PushArray(&message_table->symbols, default_symbols, ArrayCount(default_symbols));
        StringVector_PushArray(&message_table->keywords, default_keywords, ArrayCount(default_keywords));
        StringVector_PushArray(&message_table->ignore_directories, default_ignore_directories, ArrayCount(default_ignore_directories));
        StringVector_PushArray(&message_table->ignore_extensions, default_ignore_extensions, ArrayCount(default_ignore_extensions));
    }
    else
    {
        message_table->symbols = user_config->symbols;
        message_table->keywords = user_config->keywords;
        message_table->ignore_directories = user_config->ignore_directories;
        message_table->ignore_extensions = user_config->ignore_extensions;
        free(user_config);
    }

    if(message_table->symbols.size == 0 || message_table->keywords.size == 0)
    {
        LogDebug("empty symbol or keyword table");
        return 0;
    }

    // allocate each cell in the table
    usize combination_count = message_table->symbols.size * message_table->keywords.size;
    usize bytes_needed = combination_count * sizeof(MessageBucket);
    message_table->message_buckets = (MessageBucket*)(malloc(bytes_needed));
    
    if (!message_table->message_buckets)
    {
        LogDebug("Failed to allocate message table text buffers");
        return 0;
    }
    
    memset(message_table->message_buckets, 0, combination_count * sizeof(MessageBucket));
    
    // init the buckets
    for (usize s = 0; s < message_table->symbols.size; s++)
    {
        for (usize k = 0; k < message_table->keywords.size; k++) 
        {
            usize type_index = s * message_table->keywords.size + k;
            message_table->message_buckets[type_index].keyword = (s32)k;
            message_table->message_buckets[type_index].symbol = (s32)s;
            StringVector_Init(&message_table->message_buckets[type_index].strings);
        }
    }
    return message_table;  
}

void FreeMessageTable(MessageTable* message_table)
{
    if(message_table)
    {
        if(message_table->message_buckets)
        {
            for (usize s = 0; s < message_table->symbols.size; s++)
            {
                for (usize k = 0; k < message_table->keywords.size; k++) 
                {
                    usize type_index = s * message_table->keywords.size + k;
                    StringVector_Free(&message_table->message_buckets[type_index].strings);
                }
            }
            free(message_table->message_buckets);
        }
        StringVector_Free(&message_table->symbols);
        StringVector_Free(&message_table->keywords);
        free(message_table);
    }
    
}


void ProcessUserRequest(MessageTable* message_table, s32 argc, char** argv)
{
    if(!message_table) 
    { 
        Log("null message_table for user request. did you call AllocateMessageTable(user_config)?"); 
        Exit(-1); 
    }

    // @todo:: actually handle the user arguments
    const char* directory = ".";
    ProcessDirectory(message_table, directory);
}

// returns -1 if the file's extension is not in the extension list in the table
static s32 FindIgnoreExtensionIndex(MessageTable* message_table, const char* file)
{
    if(!message_table || !file) { return false; }
    for(s32 i = 0; i < message_table->ignore_extensions.size; ++i)
    {
        const char* extension = message_table->ignore_extensions.data[i];
        if(StringEndsWith(file, extension))
        {
            return i;
        }
    }
    return -1;
}

static s32 FindIgnoreDirectoryIndex(MessageTable* message_table, const char* directory)
{
    if(!message_table || !directory) { return -1; }
    for(s32 i = 0; i < message_table->ignore_directories.size; ++i)
    {
        if(StringCompare(directory, message_table->ignore_directories.data[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

typedef struct ProcessLineResults
{
    s32 symbol_index;
    s32 keyword_index;
    const char* at_pos;
}ProcessLineResults;

ProcessLineResults ProcessLine(MessageTable* message_table, const char* line)
{
    ProcessLineResults results;
    results.symbol_index = -1;
    results.keyword_index = -1;
    results.at_pos = 0;
    if(!message_table) { return results; }
 
    for (usize s = 0; s < message_table->symbols.size; s++) 
    {
        for (usize k = 0; k < message_table->keywords.size; k++) 
        {
            // looking for keyword+pattern, like @todo
            char pattern[64];
            snprintf(pattern, sizeof(pattern), "%s%s", message_table->symbols.data[s], message_table->keywords.data[k]);
            char* found = strstr(line, pattern); // @todo:: write myself, find needle in haystack
            if (found) 
            {
                usize pattern_length = StringLength(pattern);
                char next_char = found[pattern_length];
                bool is_alnum = isalpha(next_char) || isdigit(next_char);
                if (!is_alnum && next_char != '_') 
                {
                    results.symbol_index = (s32)s;
                    results.keyword_index = (s32)k;
                    results.at_pos = found;
                    return results;
                }
            }
        }
    }
    return results;
}

void ProcessFile(MessageTable* message_table, const char* filename) 
{
    FileContents contents = {0};
    usize size = GetFileContents(&contents, filename);

    if (size == 0) 
    {
        LogDebug("File has no content, or failed to read content: %s\n", filename);
        StringVector_PushBack(&message_table->empty_files, filename);
        return;
    }

    const char* current = contents.memory.buffer;
    s32 line_number = 1;
    
    while (current && *current != '\0') 
    {
        // handle line endings
        const char* line_end = current;
        while (*line_end && *line_end != '\n' && *line_end != '\r') { line_end++; }
        
        if (line_end > current) 
        {
            char line[1024];
            usize line_len = line_end - current;
            if (line_len >= sizeof(line)) line_len = sizeof(line) - 1;
            memcpy(line, current, line_len);
            line[line_len] = '\0';
            
            ProcessLineResults results = ProcessLine(message_table, line);
            if (results.at_pos) 
            {
                // Prepare output
                char buffer[1024];
                usize length = StringLength(filename);
                snprintf(buffer, sizeof(buffer), "%-48.*s %4d: %s\n", (s32)(length), filename, line_number, results.at_pos);
                usize type_index = results.symbol_index * message_table->keywords.size + results.keyword_index;
                StringVector_PushBack(&message_table->message_buckets[type_index].strings, buffer);
            }
        }
        
        current = line_end;
        if (*current == '\r') current++;
        if (*current == '\n') current++;
        line_number++;
    }

    FreeFileContents(&contents);
}


void ProcessDirectory(MessageTable* message_table, const char* directory) 
{
    DirectoryIterator directory_iterator = {0};
    DirectoryEntry current_entry = {0};
    
    if (!DirectoryOpen(&directory_iterator, directory)) 
    {
        Log("Failed to open directory: %s\n", directory);
        Exit(-1);
    }
    
    while (DirectoryNextEntry(&directory_iterator, &current_entry)) 
    {
        const char* filename = current_entry.name;
        if(!filename) 
        {
            LogDebug("ProcessDirectory, directory iterator got null file, skipping");
            continue;
        }
        
        
        if (StringCompare(filename, ".") == 0 || 
            StringCompare(filename, "..") == 0 ||
            StringCompare(filename, log_file_name) == 0)
        {
            continue;
        }


        if (current_entry.type == FileType_Directory) 
        {
            s32 directory_ignore_index = FindIgnoreDirectoryIndex(message_table, filename);
            if(directory_ignore_index == -1)
            {        
                ProcessDirectory(message_table, current_entry.path);
            }
            else
            {
                StringVector_PushBack(&message_table->skipped_directories, filename);
            }
            
        } 
        else if (current_entry.type == FileType_File) 
        {
            s32 ignore_extension_index = FindIgnoreExtensionIndex(message_table, filename);
            if(ignore_extension_index == -1)
            {        
                ProcessFile(message_table, current_entry.path);
            }
            else
            {
                StringVector_PushBack(&message_table->skipped_files, filename);
            }
        }
    }
    
    DirectoryClose(&directory_iterator);
}

//=====================================================================================================================
// Output
//=====================================================================================================================
void PrintSearchPatterns(MessageTable* message_table)
{   
    // @todo:: actually print out the combinations not just my basic @ combos
    Log("Finding all instances of [symbol][keyword]:\n\n");
    for(usize i = 0; i < message_table->keywords.size; ++i)
    {
        Log("    @%s\n", message_table->keywords.data[i]);
    }
    Log("\n");
}
void PrintIgnoredDirectories(MessageTable* message_table)
{
    Log("Ignored Directories:\n\n");
    for(usize i = 0; i < message_table->skipped_directories.size; ++i)
    {
        Log("    %s\n", message_table->skipped_directories.data[i]);
    }
    Log("\n");
}
void PrintIgnoredFiles(MessageTable* message_table)
{
    s32 extension_count = message_table->ignore_extensions.size;
    s32* counts = malloc(extension_count * sizeof(s32));
    if(!counts) { return; }
    
    for(s32 i = 0; i < extension_count; ++i)
    {
        counts[i] = 0;
    }
    
    Log("Ignored Files:\n\n");
    for(usize i = 0; i < message_table->skipped_files.size; ++i)
    {
        s32 index = FindIgnoreExtensionIndex(message_table, message_table->skipped_files.data[i]);
        if(index >= 0 && index < extension_count)
        {
            counts[index]++;
        }
    }
    
    for(usize i = 0; i < extension_count; ++i)
    { 
        if(counts[i] > 0)
        {
            Log("    %-16s (%d)\n", message_table->ignore_extensions.data[i],counts[i]);
        }
    }
    Log("\n");
    
    free(counts);
}
void PrintEmptyFiles(MessageTable* message_table)
{
    Log("Empty Files:\n\n");
    for(usize i = 0; i < message_table->empty_files.size; ++i)
    {
        Log("    %s\n", message_table->empty_files.data[i]);
    }
    Log("\n");
}
void PrintMessages(MessageTable* message_table)
{
    for (usize s = 0; s < message_table->symbols.size; s++) 
    {
        for (usize k = 0; k < message_table->keywords.size; k++) 
        {
            usize type_index = s * message_table->keywords.size + k;
            if (message_table->message_buckets[type_index].strings.size == 0) 
            {
                Log
                (
                    "[%s%s]: (no messages)\n", 
                    message_table->symbols.data[s], 
                    message_table->keywords.data[k]
                );
            }
            else
            {
                StringVector_Sort(&message_table->message_buckets[type_index].strings);
                
                Log
                (
                    "[%s%s]: (%d %s)\n\n", 
                    message_table->symbols.data[s], 
                    message_table->keywords.data[k], 
                    message_table->message_buckets[type_index].strings.size,
                    (message_table->message_buckets[type_index].strings.size > 1) ? "messages" : "message"
                );
                
                for (usize i = 0; i < message_table->message_buckets[type_index].strings.size; i++) 
                {
                    Log
                    (
                        "    %s", 
                        message_table->message_buckets[type_index].strings.data[i]
                    );
                }
                Log("\n\n");
            }
        }
    }
}