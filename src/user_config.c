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

const char* symbols_identifier = "[symbols]";
const char* keywords_identifier = "[keywords]";
const char* ignore_directories_identifier = "[ignore directories]";
const char* ignore_extensions_identifier = "[ignore extensions]";

typedef enum ConfigSection 
{
    ConfigSection_None,
    ConfigSection_Symbols,
    ConfigSection_Keywords,
    ConfigSection_IgnoreDirectories,
    ConfigSection_IgnoreExtensions
}ConfigSection;

static void ParseConfigFile(UserConfig* user_config, FileContents* config_file);
static bool FindUserConfigFile(UserConfig* user_config);

UserConfig* GetUserConfig()
{
    UserConfig* user_config = (UserConfig*)( malloc(sizeof(UserConfig)) );
    if(!user_config) 
    {
        LogDebug("GetUserConfig, failed to allocate user config\n");
        return 0;
    }
    memset(user_config, 0, sizeof(UserConfig));
    
    bool found_user_config = FindUserConfigFile(user_config);
    if(found_user_config)
    {
        FileContents config_file = {0};
        usize size = GetFileContents(&config_file, user_config->path);
        if (size == 0) 
        {
            LogDebug("GetUserConfig, config file is empty\n");
            return 0;
        }
        
        ParseConfigFile(user_config, &config_file);
        FreeFileContents(&config_file);
        return user_config;
    }
    return 0;
}

static bool FindUserConfigFile(UserConfig* user_config)
{
    if(!user_config) { return false; }

    DirectoryIterator directory_iterator = {0};
    DirectoryEntry current_entry = {0};
    DirectoryInfo directory_info = {0};
    
    if(!GetCurrentDirectoryInfo(&directory_info))
    {
        LogDebug("FindUserConfigFile, failed to get current directory info\n");
        return false;
    }
    
    if(!DirectoryOpen(&directory_iterator, directory_info.path))
    {
        LogDebug("FindUserConfigFile, failed to open file %s\n", directory_info.path);
        return false;
    }

    while(DirectoryNextEntry(&directory_iterator, &current_entry))
    {
        // look for the config file
        if(StringEndsWith(current_entry.name, ".todo_config"))
        {
            StringCopy_NullTerminate(user_config->path, current_entry.path, ArrayCount(user_config->path));
            DirectoryClose(&directory_iterator);
            return true;
        } 
    }
    DirectoryClose(&directory_iterator);
    return false;
} 

static void ParseConfigFile(UserConfig* user_config, FileContents* config_file)
{
    char* file_memory = config_file->memory.buffer;
    usize file_size = config_file->memory.size;
    char* current_pos = file_memory;
    char* end_pos = file_memory + file_size;
    
    ConfigSection current_section = ConfigSection_None;   

    while (current_pos < end_pos)
    {
        // Find the end of the current line
        char* line_end = current_pos;
        while (line_end < end_pos && *line_end != '\n' && *line_end != '\r') 
        {
            line_end++;
        }
        
        // if there isnt a line just go to the next one
        usize line_length = line_end - current_pos;
        if (line_length == 0) { current_pos = line_end + 1; continue; }
        
        // skip comments 
        if (current_pos[0] == '#')
        {
            current_pos = line_end + 1;
            continue;
        }
        
        // Start of a new section
        // make sure its valid then change the section enum
        if (current_pos[0] == '[') 
        {
            // temporarily replace line endings with null for easy comparisons
            char temp = *line_end;
            *line_end = '\0';
            
            if (StringCompare(current_pos, symbols_identifier) == 0) { current_section = ConfigSection_Symbols; } 
            else if (StringCompare(current_pos, keywords_identifier) == 0) { current_section = ConfigSection_Keywords; } 
            else if (StringCompare(current_pos, ignore_directories_identifier) == 0) { current_section = ConfigSection_IgnoreDirectories; } 
            else if (StringCompare(current_pos, ignore_extensions_identifier) == 0) { current_section = ConfigSection_IgnoreExtensions; } 
            else { current_section = ConfigSection_None; }
            
            *line_end = temp;
        } 
        else if (current_section != ConfigSection_None) 
        {
            // This is a token in the current section
            // Trim whitespace from the line
            char* token_start = current_pos;
            char* token_end = line_end;
            
            // Trim whitespace
            while (token_start < token_end && isspace(*token_start)) { token_start++; }
            while (token_end > token_start && isspace(*(token_end - 1))) { token_end--; }
            
            // we trimmed whitespace on both sides and should have the entire line's contents
            if (token_start < token_end) 
            {
                char token[256] = {0};
                usize token_length = token_end - token_start;
                if(token_length > ArrayCount(token))
                {
                    LogDebug("ParseConfigFile, Token too long\n");
                }
                else
                {
                    memcpy(token, token_start, token_length);
                    token[token_length] = '\0';
                    switch (current_section) 
                    {
                        case ConfigSection_None: LogDebug("ParseConfigFile, stray token not within any section\n"); break;
                        case ConfigSection_Symbols: StringVector_PushBack(&user_config->symbols, token); break;
                        case ConfigSection_Keywords: StringVector_PushBack(&user_config->keywords, token); break;
                        case ConfigSection_IgnoreDirectories: StringVector_PushBack(&user_config->ignore_directories, token); break;
                        case ConfigSection_IgnoreExtensions: StringVector_PushBack(&user_config->ignore_extensions, token); break;
                    }
                }
            }
        }
        
        // Move to the next line
        current_pos = line_end;
        while (current_pos < end_pos && (*current_pos == '\n' || *current_pos == '\r')) 
        {
            current_pos++;
        }
    }
}
