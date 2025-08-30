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

void StringVector_Init(StringVector* vec) 
{
    if(!vec) return;
    if(vec->data) 
    { 
        for (usize i = 0; i < vec->size; i++) { free(vec->data[i]); }
        free(vec->data); 
    } 
    vec->data = 0;
    vec->size = 0;
    vec->capacity = 0;
}

void StringVector_PushBack(StringVector* vec, const char* string) 
{
    if(!vec)
    {
        LogDebug("null string vector");
        return;
    }   
    
    if (vec->size >= vec->capacity) 
    {
        const usize starting_capacity = 8;
        usize new_capacity = (vec->capacity < starting_capacity) ? starting_capacity : vec->capacity * 2;
        char** new_data = realloc(vec->data, new_capacity * sizeof(char*));
        if (!new_data) 
        {
            LogDebug("Failed to reallocate memory for string vector");
            return;
        }
        vec->data = new_data;
        vec->capacity = new_capacity;
    }
    
    usize length = StringLength(string);
    vec->data[vec->size] = (char*)(malloc(length + 1));
    
    if (!vec->data[vec->size]) 
    {
        Log("Failed to duplicate string");
        return;
    }
    
    StringCopy_NullTerminate(vec->data[vec->size], string, length + 1);
    vec->size++;
}

void StringVector_Free(StringVector* vec) 
{
    for (usize i = 0; i < vec->size; i++) { free(vec->data[i]); }
    free(vec->data);
    vec->data = 0;
    vec->size = 0; 
    vec->capacity = 0;
}


s32 Compare_CString(const void* a, const void* b) { return strcmp(*(const char**)a, *(const char**)b); }

void StringVector_Sort(StringVector* vec)
{
    if(vec->size > 1)
    {
        GenericQuickSort(vec->data, 0, vec->size - 1, sizeof(vec->data[0]), Compare_CString);
    }
}