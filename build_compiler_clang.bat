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

:: Clang Build Script ::
:: compiles c and c++ files
::
::
:: Environment setup 
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

:: User Variables ::
set EXE_NAME=todo_finder.exe
set RUN_ARGS=. clang release
set SOURCE_DIR=src
set CPP_STANDARD=c++17
set DEBUG_FLAGS=-g -O0 -DDEBUG -D_DEBUG -Xclang --dependent-lib=libcmtd
set RELEASE_W_DEBUG_FLAGS=-g -O2 -DNDEBUG 
set RELEASE_FLAGS=-O2 -DNDEBUG 
set SHIPPING_FLAGS=-O2 -w -DNDEBUG 

set COMMON_LIBS=
set DEBUG_LIBS=%COMMON_LIBS% 
set RELEASE_LIBS=%COMMON_LIBS% 

:: check args
if "%1" == "" (
    echo Error: No configuration specified
    echo Usage: %~nx0 [debug|release_with_debug|release|shipping]
    exit /b 1
)

:: Configuration
set CONFIG=%1
set OUTPUT_ROOT=clang
set COMPILER=clang
set LINKER=clang
set OUTPUT_PATH=%OUTPUT_ROOT%\%CONFIG%\%EXE_NAME%

set DEBUG_FLAGS=%DEBUG_FLAGS%
set RELEASE_W_DEBUG_FLAGS=%RELEASE_W_DEBUG_FLAGS%
set RELEASE_FLAGS=%RELEASE_FLAGS%
set SHIPPING_FLAGS=%SHIPPING_FLAGS%

set DEBUG_LINK_FLAGS=-g -Xlinker /DEBUG -o "%OUTPUT_PATH%" -Xlinker /NODEFAULTLIB:libcmt.lib %DEBUG_LIBS%
set RELEASE_W_DEBUG_LINK_FLAGS=-g -Xlinker /DEBUG -o "%OUTPUT_PATH%" %RELEASE_LIBS%
set RELEASE_LINK_FLAGS=-o "%OUTPUT_PATH%" %RELEASE_LIBS%
set SHIPPING_LINK_FLAGS=-o "%OUTPUT_PATH%" %RELEASE_LIBS%

:: Compile everything
:compile_stage
echo Compiling source files...
echo .
echo .
echo .

:: Get all source files ::
set FILE_LIST=
for /r "%SOURCE_DIR%" %%f in (*.c *.cpp *.cxx *.cc *.c++) do (
    set FILE_LIST=!FILE_LIST! "%%f"
)

:: Create output directory
if not exist "%OUTPUT_ROOT%\%CONFIG%\" mkdir "%OUTPUT_ROOT%\%CONFIG%\"
if not exist "temp\" mkdir "temp\"

:: Set active flags
if "%CONFIG%" == "debug" (
    set "BASE_FLAGS=%DEBUG_FLAGS%"
    set "LINK_FLAGS=%DEBUG_LINK_FLAGS%"
) else if "%CONFIG%" == "release_with_debug" (
    set "BASE_FLAGS=%RELEASE_W_DEBUG_FLAGS%"
    set "LINK_FLAGS=%RELEASE_W_DEBUG_LINK_FLAGS%"
) else if "%CONFIG%" == "release" (
    set "BASE_FLAGS=%RELEASE_FLAGS%"
    set "LINK_FLAGS=%RELEASE_LINK_FLAGS%"
) else if "%CONFIG%" == "shipping" (
    set "BASE_FLAGS=%SHIPPING_FLAGS%"
    set "LINK_FLAGS=%SHIPPING_LINK_FLAGS%"
) else (
    echo Unknown Configuration: %CONFIG%
    echo Valid configurations: debug, release_with_debug, release, shipping
    exit /b 1
)

for %%f in (%FILE_LIST%) do (
    echo %%~f 
    set "src_file=%%~nxf"
    :: Chop off file extensions to correctly name .o files
    if "!src_file:~-4!" == ".cpp" (
        set "obj_file=%OUTPUT_ROOT%\%CONFIG%\!src_file:~0,-4!.o"
    ) else if "!src_file:~-4!" == ".cxx" (
        set "obj_file=%OUTPUT_ROOT%\%CONFIG%\!src_file:~0,-4!.o"
    ) else if "!src_file:~-3!" == ".cc" (
        set "obj_file=%OUTPUT_ROOT%\%CONFIG%\!src_file:~0,-3!.o"
    ) else if "!src_file:~-4!" == ".c++" (
        set "obj_file=%OUTPUT_ROOT%\%CONFIG%\!src_file:~0,-4!.o"
    ) else if "!src_file:~-2!" == ".c" (
        set "obj_file=%OUTPUT_ROOT%\%CONFIG%\!src_file:~0,-2!.o"
    ) 
    
    :: Check if this is a C or C++ file
    if "!src_file:~-2!" == ".c" (
        %COMPILER% -x c %BASE_FLAGS% -c "%%~f" -o "!obj_file!"
    ) else (
        %COMPILER% -x c++ %BASE_FLAGS% "-std=%CPP_STANDARD%" -c "%%~f" -o "!obj_file!"
    )
        
    if errorlevel 1 (
        echo Compilation failed for %%~f
        goto menu_prompt
    )
)

echo .
echo .
echo .

:: Link everything
:link_stage
echo Linking executable...
%LINKER% %LINK_FLAGS% "%OUTPUT_ROOT%\%CONFIG%\*.o"

if errorlevel 1 (
    echo Linking failed!
    echo .
    echo .
    echo .
    echo Running %LINKER% -v to see invocation
    %LINKER% -v
    goto menu_prompt
)

echo .
echo .
echo .
echo Build succeeded: %OUTPUT_PATH%



:menu_prompt

:: was there something built to run?
if defined OUTPUT_PATH (
:run_loop
    choice /c ynrc /n /m "Run Latest Build? (Y/N) Rebuild? (R) Clean? (C)"
    if errorlevel 4 (
        goto clean_stage
    )
    if errorlevel 3 (
        goto compile_stage
    )
    if errorlevel 2 (
        goto shutdown
    ) else if errorlevel 1 (
        echo Running !OUTPUT_PATH! %RUN_ARGS%
        "!OUTPUT_PATH!" %RUN_ARGS%
        goto run_loop
    )
) else (
    echo Latest build not found...
    echo.
    set /p ="[Press any key to close...]" <nul & pause >nul
    goto shutdown
)


:clean_stage
echo.
echo Clean Options:
echo C. Clean current config (%CONFIG%)
echo A. Clean ALL configs
echo 1. Clean Debug config
echo 2. Clean Release_w_Debug config  
echo 3. Clean Release config
echo 4. Clean Shipping config
echo 0. Cancel
echo.

choice /c CA12340 /n /m "Select clean option:"
set CLEAN_OPTION=%errorlevel%

if %CLEAN_OPTION% equ 7 (
    echo Clean cancelled.
    echo.
    goto run_loop
)

if %CLEAN_OPTION% equ 1 (
    call :clean_single_config "%CONFIG%"
) else if %CLEAN_OPTION% equ 2 (
    call :clean_all_configs
) else if %CLEAN_OPTION% equ 3 (
    call :clean_single_config "Debug"
) else if %CLEAN_OPTION% equ 4 (
    call :clean_single_config "Release_w_Debug"
) else if %CLEAN_OPTION% equ 5 (
    call :clean_single_config "Release"
) else if %CLEAN_OPTION% equ 6 (
    call :clean_single_config "Shipping"
)

goto menu_prompt


:clean_single_config
setlocal
set CONFIG_TO_CLEAN=%~1
set CLEAN_PATH=!OUTPUT_ROOT!\!CONFIG_TO_CLEAN!

echo Cleaning !CONFIG_TO_CLEAN! build...
if exist "!CLEAN_PATH!" (
    rem Remove the entire folder and all contents
    rd /s /q "!CLEAN_PATH!" 2>nul
    if exist "!CLEAN_PATH!" (
        echo Error: Could not delete !CLEAN_PATH! (files may be in use)
    ) else (
        echo Deleted: !CLEAN_PATH!
    )
)
endlocal
goto :eof

:clean_all_configs
echo Cleaning ALL configs...
call :clean_single_config "Debug"
call :clean_single_config "Release_w_Debug"
call :clean_single_config "Release"
call :clean_single_config "Shipping"
goto :eof

:shutdown
exit
