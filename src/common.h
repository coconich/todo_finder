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

#ifndef COMMON_H
#define COMMON_H

//=====================================================================================================================
// Cross Platform Defines
//=====================================================================================================================
#ifdef _WIN32
    #define OS_Win32
    #include <windows.h>
    #include <direct.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif


//=====================================================================================================================
// stdlib
//=====================================================================================================================
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <stdarg.h> 
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>


//=====================================================================================================================
// Basics
//=====================================================================================================================
typedef int32_t s32;
typedef uint32_t u32;
typedef size_t usize;

#define ArrayCount(array) (sizeof(array)/sizeof(array[0]))
#define MaxPath 4096

// array usage:: GenericQuickSort(&array, 0, ArrayCount(array) - 1, sizeof(array[0]), MyCustomComparatorFunction);
// vec   usage:: GenericQuickSort(vec->ptr, 0, vec->size - 1, sizeof(vec->ptr[0]), MyCustomComparatorFunction);
void GenericQuickSort(void* ptr, usize low, usize high, usize object_size, s32 (*Compare)(const void*, const void*));

//=====================================================================================================================
// Exit
//=====================================================================================================================
// registers a function pointer void func(void) to be called at program exit
// essentially global destructors in the form of a function call
// functions called in FILO order
typedef void(*ExitFunction)(); 
#define AtExit(function) RegisterExitFunction(function, #function, __FILE__)
#define Exit(code) Terminate(code, __FILE__)
void RegisterExitFunction(ExitFunction function, const char *name, const char *file);
void Terminate(s32 exit_code, const char *file);

//=====================================================================================================================
// Logger
//=====================================================================================================================
static const char* log_file_name = "todo_output.txt";
void LogMessage(const char* format, ...);
#define Log(format, ...) LogMessage(format, ##__VA_ARGS__) 
#ifdef NDEBUG
    #define LogDebug(format, ...) ((void)(0))
#else
    #define LogDebug(format, ...) LogMessage(format, ##__VA_ARGS__) 
#endif 


//=====================================================================================================================
// Allocator
//=====================================================================================================================
// basic static size heap memory buffer
// crashes on failure
typedef struct MemoryBuffer 
{
    char* buffer;
    usize size;
} MemoryBuffer;
void Allocate(MemoryBuffer* memory, usize bytes); 
void Free(MemoryBuffer* memory); 


//=====================================================================================================================
// Files
//=====================================================================================================================
// i want to read/write/edit files 
// returns nullptrs/false/0 on failure
typedef struct File
{
    FILE* fp;
    char mode[4];
    char path[MaxPath];
}File;
bool  FileOpen(File* file, const char* filename, const char* mode);
usize FileWrite(MemoryBuffer* data, usize byte_count, File* file);
usize FileRead(MemoryBuffer* destination, usize byte_count, File* file);
usize FilePuts(const char* string, File* file);
void  FileClose(File* file);

// Dont bother with file streaming
// just read it all into a buffer
// buffer is empty on failure
typedef struct FileContents 
{
    char path[MaxPath];
    MemoryBuffer memory;    
} FileContents;
usize GetFileContents(FileContents* file_contents, const char* filepath);
void  FreeFileContents(FileContents* file_contents);

//=====================================================================================================================
// Directories
//=====================================================================================================================
typedef enum 
{
    FileType_File,
    FileType_Directory,
    FileType_Other,
    FileType_Count
} FileType;

typedef struct 
{
    const char* name;
    FileType type;
    char path[MaxPath];
} DirectoryEntry;

typedef struct 
{
    char text_buffer[MaxPath];
    
#ifdef OS_Win32
    HANDLE handle;
    WIN32_FIND_DATAA find_data;
    bool first_file;
#else
    DIR* dir;
    struct dirent* entry;
#endif

} DirectoryIterator;

bool DirectoryOpen(DirectoryIterator* iter, const char* directory);
bool DirectoryNextEntry(DirectoryIterator* directory_iterator, DirectoryEntry* entry);
void DirectoryClose(DirectoryIterator* iter);

//=====================================================================================================================
// C Strings
//=====================================================================================================================
usize StringCopy_NullTerminate(char* to, const char* from, usize last_index); // to[min(strlen(from), last_index] = '\0'
usize StringLength(const char *string);

// Array of c strings
// crashes on failure
typedef struct StringVector
{
    char** data;
    usize size;
    usize capacity;
} StringVector;
void StringVector_Init(StringVector* vec);
void StringVector_PushBack(StringVector* vec, const char* string);
void StringVector_Free(StringVector* vec);
void StringVector_Sort(StringVector* vec);

//=====================================================================================================================
// User Configuration
//=====================================================================================================================
typedef struct UserConfig
{
    StringVector symbols;
    StringVector keywords;
}UserConfig;
UserConfig* GetUserConfig();

//=====================================================================================================================
// Message Table
//=====================================================================================================================
typedef struct MessageBucket
{
    s32 symbol;
    s32 keyword;
    StringVector strings;
} MessageBucket;
    
typedef struct MessageTable
{
    MessageBucket* message_buckets;
    StringVector symbols;
    StringVector keywords;
    StringVector skipped_directories;
    StringVector empty_files;
}MessageTable;

// user config is optional
// non null, but invalid user config will use the defaults as needed
MessageTable* AllocateMessageTable(UserConfig* user_config);
void FreeMessageTable(MessageTable* message_table);

// fill the message table based 
// 
void ProcessUserRequest(MessageTable* message_table, s32 argc, char** argv);

// what user request is calling with your desired input
void ProcessFile(MessageTable* message_table, const char* file);
void ProcessDirectory(MessageTable* message_table, const char* directory);

void PrintSearchPatterns(MessageTable* message_table);
void PrintIgnoredDirectories(MessageTable* message_table);
void PrintEmptyFiles(MessageTable* message_table);
void PrintMessages(MessageTable* message_table);




//=====================================================================================================================
#endif // COMMON_H


