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

s32 main(s32 argc, char** argv) 
{
    Log("\n\n=======================================================================================================================\n");
    Log("============================================= Todo Finder  @coconich_dev ==============================================\n\n");

    // @todo:: write the user config parser
    // it can have definitions for any of these things
    // extra stuff and missing stuff is ignored
    //  symbols
    //  keywords
    //  ignore directories
    //  include extensions
    //  ignore extensions
    UserConfig* user_config = GetUserConfig();
    if(!user_config)
    {
        Log("Using default configuration.\n");
        Log("see readme for custom configurations\n\n");
    }
    
    // the message table is just a big memory buffer that is indexed into via 2d coordinates 
    // the current symbol_index and current keyword index are used for offsets into the buckets
    //     this means the size of the table is dependent on user config
    //     there is a default table of symbol keyword pairs if none is provided
    // it also holds string vectors regarding user config, like ignored directories, extensions, etc
    MessageTable* message_table = AllocateMessageTable(user_config);
    if(!message_table) 
    { 
        Log("failed to allocate message table\n"); 
        Exit(-1); 
    }
    
    // build the message table by parsing the files/folders requested
    // this fills up the message buckets with found matches of [symbol][keyword]
    ProcessUserRequest(message_table, argc, argv);
    
    // show the user the results
    PrintSearchPatterns(message_table);
    PrintIgnoredDirectories(message_table);
    PrintIgnoredFiles(message_table);
    PrintEmptyFiles(message_table);
    PrintMessages(message_table);
    

    Log("=======================================================================================================================\n\n");

    // clean up and call global destructors
    FreeMessageTable(message_table);
    Exit(0);
    
}