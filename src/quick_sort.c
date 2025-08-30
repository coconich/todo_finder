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

static void GenericSwap(void* a, void* b, usize object_size)
{
    char temp_buffer[256];
    if (object_size <= sizeof(temp_buffer)) 
    {
        memcpy(temp_buffer, a, object_size);
        memcpy(a, b, object_size);
        memcpy(b, temp_buffer, object_size);
    } 
    else 
    {
        void* temp = malloc(object_size);
        memcpy(temp, a, object_size);
        memcpy(a, b, object_size);
        memcpy(b, temp, object_size);
        free(temp);
    }
}

static usize GenericPartition(void* ptr, usize low, usize high, usize object_size, s32 (*Compare)(const void*, const void*)) 
{
    char* array = (char*)ptr;
    void* pivot = array + high * object_size;
    usize i = low;
    
    for (usize j = low; j < high; j++) 
    {
        if (Compare(array + j * object_size, pivot) <= 0) 
        {
            GenericSwap(array + i * object_size, array + j * object_size, object_size);
            i++;
        }
    }
    GenericSwap(array + i * object_size, array + high * object_size, object_size);
    return i;
}

void GenericQuickSort(void* ptr, usize low, usize high, usize object_size, s32 (*Compare)(const void*, const void*)) 
{
    if (low < high) 
    {
        usize partition = GenericPartition(ptr, low, high, object_size, Compare);
        if (partition > low) { GenericQuickSort(ptr, low, partition - 1, object_size, Compare); }
        if (partition < high) { GenericQuickSort(ptr, partition + 1, high, object_size, Compare); }
    }
}