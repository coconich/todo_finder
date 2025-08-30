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

static File log_file;   
void CloseLogFile() { FileClose(&log_file); }

void LogMessage(const char* format, ...)        
{
    // try to open log file if its closed
    if(!log_file.fp)
    {
        FileOpen(&log_file, log_file_name, "w+");
        AtExit(CloseLogFile);
    }
    
    // log to file if open and stdout                                  
    va_list arg_ptr;  
    if (log_file.fp)
    {
        va_start(arg_ptr, format); 
        vfprintf(log_file.fp, format, arg_ptr);  
        va_end(arg_ptr); 
    }                                            
    va_start(arg_ptr, format);
    vprintf(format, arg_ptr);
    va_end(arg_ptr);
}
