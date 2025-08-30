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

typedef struct MessageType
{
    s32 symbol;
    s32 keyword;
}MessageType;
MessageType CheckLineForMessages(MessageTable* message_table, const char* line);

MessageTable* AllocateMessageTable(UserConfig* user_config)
{
    static MessageTable message_table = {};
    
    // default table
    if(!user_config)
    {
        StringVector_Init(&message_table.symbols);
        StringVector_Init(&message_table.keywords);

        for (usize i = 0; i < ArrayCount(default_symbols); i++)
        {
            StringVector_PushBack(&message_table.symbols, default_symbols[i]);
        }
        for (usize i = 0; i < ArrayCount(default_keywords); i++)
        {
            StringVector_PushBack(&message_table.keywords, default_keywords[i]);
        }
    }
    else
    {
        message_table.symbols = user_config->symbols;
        message_table.keywords = user_config->keywords;
    }

    if(message_table.symbols.size == 0 || message_table.keywords.size == 0)
    {
        LogDebug("empty symbol or keyword table");
        return 0;
    }

    // allocate each cell in the table
    usize combination_count = message_table.symbols.size * message_table.keywords.size;
    usize bytes_needed = combination_count * sizeof(MessageBucket);
    message_table.message_buckets = (MessageBucket*)(malloc(bytes_needed));
    
    if (!message_table.message_buckets)
    {
        LogDebug("Failed to allocate message table text buffers");
        return 0;
    }
    
    memset(message_table.message_buckets, 0, combination_count * sizeof(MessageBucket));
    
    // init the cells
    for (usize s = 0; s < message_table.symbols.size; s++)
    {
        for (usize k = 0; k < message_table.keywords.size; k++) 
        {
            usize type_index = s * message_table.keywords.size + k;
            message_table.message_buckets[type_index].keyword = (s32)k;
            message_table.message_buckets[type_index].symbol = (s32)s;
            StringVector_Init(&message_table.message_buckets[type_index].strings);
        }
    }
    return &message_table;  
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




MessageType CheckLineForMessages(MessageTable* message_table, const char* line)
{
    MessageType pair;
    pair.symbol = -1;
    pair.keyword = -1;
    if(!message_table) { return pair; }
 
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
                    pair.symbol = (s32)s;
                    pair.keyword = (s32)k;
                    return pair;
                }
            }
        }
    }
    return pair;
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
            
            MessageType pair = CheckLineForMessages(message_table, line);
            s32 symbol = pair.symbol;
            s32 keyword = pair.keyword;
            if (symbol != -1 && keyword != -1) 
            {
                char current_symbol = *message_table->symbols.data[symbol];
                char* at_pos = strchr(line, current_symbol); // todo:: replace, return ptr to first occurance of char
                if (at_pos) 
                {
                    // Prepare output
                    char buffer[1024];
                    usize length = StringLength(filename);
                    snprintf(buffer, sizeof(buffer), "%-48.*s %4d: %s\n", (s32)(length), filename, line_number, at_pos);
                    usize type_index = symbol * message_table->keywords.size + keyword;
                    StringVector_PushBack(&message_table->message_buckets[type_index].strings, buffer);
                }
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
    DirectoryIterator directory_iterator;
    DirectoryEntry current_entry;
    
    if (!DirectoryOpen(&directory_iterator, directory)) 
    {
        Log("Failed to open directory: %s\n", directory);
        Exit(-1);
    }
    
    while (DirectoryNextEntry(&directory_iterator, &current_entry)) 
    {
        const char* filename = current_entry.name;

        if (strcmp(filename, ".") == 0 || 
            strcmp(filename, "..") == 0 ||
            strcmp(filename, log_file_name) == 0)
        {
            continue;
        }
        else if(strcmp(filename, ".svn") == 0 || 
                strcmp(filename, ".vs") == 0 || 
                strcmp(filename, "bin") == 0 || 
                strcmp(filename, "bin-obj") == 0 || 
                strcmp(filename, "libs") == 0 || 
                strcmp(filename, "deps") == 0)
        { 
            StringVector_PushBack(&message_table->skipped_directories, filename);
            continue; 
        }

        if (current_entry.type == FileType_Directory) 
        {
            ProcessDirectory(message_table, current_entry.path);
        } 
        else if (current_entry.type == FileType_File) 
        {
            ProcessFile(message_table, current_entry.path);
        }
    }
    
    DirectoryClose(&directory_iterator);
}


void PrintSearchPatterns(MessageTable* message_table)
{
    // Tell the user what is being searched for
    Log("Finding all instances of [symbol][keyword]:\n\n");
    for(usize i = 0; i < message_table->keywords.size; ++i)
    {
        Log("    @%s\n", message_table->keywords.data[i]);
    }
    Log("\n");
}

void PrintIgnoredDirectories(MessageTable* message_table)
{
    // tell the users which directories were auto skipped
    // @todo:: let them know they can edit a config to change this
    Log("Ignored Directories:\n\n");
    for(usize i = 0; i < message_table->skipped_directories.size; ++i)
    {
        Log("   %s\n", message_table->skipped_directories.data[i]);
    }
    Log("\n");
}

void PrintEmptyFiles(MessageTable* message_table)
{
    Log("Empty Files:\n\n");
    for(usize i = 0; i < message_table->empty_files.size; ++i)
    {
        Log("   %s\n", message_table->empty_files.data[i]);
    }
    Log("\n");
}

void PrintMessages(MessageTable* message_table)
{
    // pretty print out the messages in table 
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