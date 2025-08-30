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

usize StringCopy_NullTerminate(char* to, const char* from, usize max_chars_to_copy) 
{
    if(max_chars_to_copy <= 0) { return 0; }
    
    usize count = 0;
    while((count < max_chars_to_copy - 1) && from[count]) 
    {
        to[count] = from[count];
        count++;    
    }
    to[count] = '\0';
    return count;
}

s32 StringCompare(const char *left, const char* right)
{
    while (*left != '\0' && *right != '\0') 
    {
        if (*left != *right) { return (u8)(*left) - (u8)(*right); }
        left++;
        right++;
    }
    return (u8)(*left) - (u8)(*right);
}


bool StringEndsWith(const char *string, const char* suffix)
{
    if(!string || !suffix) { return false; }
    
    usize string_length = StringLength(string);
    usize suffix_length = StringLength(suffix);
    
    if(suffix_length == 0) { return false; }
    if(suffix_length > string_length) { return false; }
    
    usize suffix_start_index = string_length - suffix_length;
    return StringCompare(&string[suffix_start_index], suffix) == 0;
}