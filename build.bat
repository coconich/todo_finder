:: MIT License

:: Copyright (c) 2025 Cory Simonich

:: Permission is hereby granted, free of charge, to any person obtaining a copy
:: of this software and associated documentation files (the "Software"), to deal
:: in the Software without restriction, including without limitation the rights
:: to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
:: copies of the Software, and to permit persons to whom the Software is
:: furnished to do so, subject to the following conditions:

:: The above copyright notice and this permission notice shall be included in all
:: copies or substantial portions of the Software.

:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
:: FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
:: AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
:: LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
:: OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
:: SOFTWARE.

 @echo off
 setlocal enabledelayedexpansion

if "%1"=="" (
    goto ERROR
)

if /I "%1"=="clean" (
    call :clean_all
    exit /b 0
)

if /I "%1"=="gcc" (  
    if /I "%2"=="debug" (
        call :gcc_debug 
        exit /b 0
    )
    if /I "%2"=="shipping" (
        call :gcc_shipping 
        exit /b 0
    )
    goto ERROR
)

if /I "%1"=="clang" (  
    if /I "%2"=="debug" (
        call :clang_debug 
        exit /b 0
    )
    if /I "%2"=="shipping" (
        call :clang_shipping 
        exit /b 0
    )
    goto ERROR  
)

exit /b 0

:gcc_debug
start "" cmd /c "wsl bash -c " ./build_compiler_gcc.sh debug" "
exit /b 0

:gcc_shipping
start "" cmd /c "wsl bash -c " ./build_compiler_gcc.sh shipping" "
exit /b 0

:clang_debug
start "" cmd /c "build_compiler_clang.bat debug"
exit /b 0

:clang_shipping
start "" cmd /c "build_compiler_clang.bat shipping"
exit /b 0

:ERROR
echo Usage: build ^<gcc^|clang^> ^<debug^|shipping^>
exit /b 1

:clean_all
setlocal
set "directories=clang\debug clang\shipping gcc\debug gcc\shipping"

for %%D in (%directories%) do (
    if exist "%%D\*" (
        set "hasFiles=0"
        for /f %%i in ('dir /b /a-d "%%D\*" 2^>nul ^| find /v /c ""') do set "hasFiles=%%i"
        
        if !hasFiles! gtr 0 (
            echo Cleaning directory: %%D
            
            for %%F in ("%%D\*") do (
                del /f "%%F" >nul
                echo     Deleted: %%~nxF
            )
        ) else (
            echo Nothing to clean: %%D
        )
    ) else (
        echo Directory not found: %%D
    )
)
endlocal

exit /b 0

        