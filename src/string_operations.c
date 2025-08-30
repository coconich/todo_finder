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

usize StringLength(const char *string)
{
    usize length = 0;
    while(*string++) { length++; }
    return length;
}

usize StringCopy_NullTerminate(char* to, const char* from, usize last_index) 
{
    if(last_index <= 0) { return 0; }
    
    usize count = 0;
    while((count < last_index - 1) && from[count]) 
    {
        to[count] = from[count];
        count++;    
    }
    to[count] = '\0';
    return count;
}
