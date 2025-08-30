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

// [symbols]
// @
// ~
// #
//
// [keywords]
// todo
// cleanup
// bug
// nocheckin
//
// [ignore directories]
// .vs
// .git
// build
// bin
//
// [include extensions]
// .c
// .cpp
// .h
// .txt
// .bat
// .sh
// .vert
// .frag
//
// [ignore extensions]
// .pdf
// .docx
//

UserConfig* GetUserConfig()
{
    static UserConfig user_config = {};
    
    // @todo:: search for user specified configuration
    // @todo:: parse the found configuration file 
    bool found_user_config = false;
    if(found_user_config)
    {
        // ...
        StringVector_Init(&user_config.symbols);
        StringVector_Init(&user_config.keywords);
        return &user_config;
    }
    return 0;
}
