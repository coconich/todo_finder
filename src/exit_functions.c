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

#define MAX_EXIT_FUNCTION_COUNT 1024
static const char* exit_function_names[MAX_EXIT_FUNCTION_COUNT] = {0};
static ExitFunction exit_functions[MAX_EXIT_FUNCTION_COUNT] = {0};
static s32 exit_function_count = 0;

void RegisterExitFunction(ExitFunction function, const char *name, const char *file)
{
    assert(exit_function_count < MAX_EXIT_FUNCTION_COUNT);
    exit_function_names[exit_function_count] = name;
    exit_functions[exit_function_count] = function;
    exit_function_count++;

    LogDebug("Exit Function registered: %s, %s", name, file);
}

// this function does not return
void Terminate(s32 exit_code, const char *file)
{
    for(s32 i = (exit_function_count - 1); i >= 0; --i)
    {
        if(exit_functions[i])
        {
            exit_functions[i]();
        }
    }

#ifdef OS_Win32
    u32 code = (u32)(exit_code); // just let -1 get cast to u32, it will show up as 4294967295
    HANDLE hProcess = GetCurrentProcess();
    TerminateProcess(hProcess, code);
    CloseHandle(hProcess);
#else
    _exit(exit_code);
#endif

}


