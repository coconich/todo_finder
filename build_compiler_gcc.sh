#!/bin/bash

# GCC Build Script #
# compiles c and c++ files
#
#

# User Variables
EXE_NAME="todo_finder"
SOURCE_DIR="src"
CPP_STANDARD="c++17"
DEBUG_FLAGS="-g -O0 -DDEBUG -D_DEBUG"
RELEASE_W_DEBUG_FLAGS="-g -O2 -DNDEBUG"
RELEASE_FLAGS="-O2 -DNDEBUG"
SHIPPING_FLAGS="-O2 -w -DNDEBUG"

# check args
if [ -z "$1" ]; then
    echo "Error: No configuration specified"
    echo "Usage: $0 [debug|release_with_debug|release|shipping]"
    exit 1
fi

# Configuration
CONFIG=$1
OUTPUT_ROOT="gcc"
COMPILER="gcc"
CPP_COMPILER="g++"
LINKER="g++"
OUTPUT_ACTUAL="$OUTPUT_ROOT/$CONFIG/$EXE_NAME"

DEBUG_LINK_FLAGS="-o $OUTPUT_ACTUAL"
RELEASE_W_DEBUG_LINK_FLAGS="-o $OUTPUT_ACTUAL"
RELEASE_LINK_FLAGS="-o $OUTPUT_ACTUAL"
SHIPPING_LINK_FLAGS="-o $OUTPUT_ACTUAL"

find_source_files() {
    local file_list=()
    while IFS= read -r -d '' file; do
        file_list+=("$file")
    done < <(find "$SOURCE_DIR" -type f \( \
        -name "*.c" -o \
        -name "*.cpp" -o \
        -name "*.cxx" -o \
        -name "*.cc" -o \
        -name "*.c++" \
        \) -print0 2>/dev/null)
    
    if [ ${#file_list[@]} -eq 0 ]; then
        echo "Error: No source files found in $SOURCE_DIR"
        return 1
    fi
    
    echo "${file_list[@]}"
    return 0
}

compile_stage() {
    echo "Compiling source files..."
    echo .
    echo .
    echo .
    
    local file_list
    file_list=$(find_source_files)
    if [ $? -ne 0 ]; then
        return 1
    fi
    
    mkdir -p "$OUTPUT_ROOT/$CONFIG"
    
    case $CONFIG in
        debug)
        BASE_FLAGS="$DEBUG_FLAGS"
        LINK_FLAGS="$DEBUG_LINK_FLAGS"
        ;;
        release_with_debug)
        BASE_FLAGS="$RELEASE_W_DEBUG_FLAGS"
        LINK_FLAGS="$RELEASE_W_DEBUG_LINK_FLAGS"
        ;;
        release)
        BASE_FLAGS="$RELEASE_FLAGS"
        LINK_FLAGS="$RELEASE_LINK_FLAGS"
        ;;
        shipping)
        BASE_FLAGS="$SHIPPING_FLAGS"
        LINK_FLAGS="$SHIPPING_LINK_FLAGS"
        ;;
        *)
        echo "Unknown Configuration: $CONFIG"
        echo "Valid configurations: debug, release_with_debug, release, shipping"
        return 1
        ;;
    esac
    
    # Compile each file 
    OBJECT_FILES=()
    for file in $file_list; do
        src_file=$(basename "$file")
        echo "Compiling: $file"
        
        case "$src_file" in
            *.cpp|*.cxx|*.cc|*.c++)
                obj_file="$OUTPUT_ROOT/$CONFIG/${src_file%.*}.o"
                compiler="$CPP_COMPILER"
                std_flag="-std=$CPP_STANDARD"
                ;;
            *.c)
                obj_file="$OUTPUT_ROOT/$CONFIG/${src_file%.*}.o"
                compiler="$COMPILER"
                std_flag=""
                ;;
            *)
                echo "Unknown file type: $file"
                continue
                ;;
        esac
        
        # try to compile this file
        if ! $compiler $BASE_FLAGS $std_flag -c "$file" -o "$obj_file"; then
            echo "Compilation failed for $file"
            return 1
        fi
        
        OBJECT_FILES+=("$obj_file")
    done
    
    echo .
    echo .
    echo .
    return 0
}

link_stage() {
    echo "Linking executable..."
    if ! $LINKER "${OBJECT_FILES[@]}" $LINK_FLAGS; then
        echo "Linking failed!"
        return 1
    fi
    
    echo .
    echo .
    echo .
    echo "Build succeeded: $OUTPUT_ACTUAL"
    return 0
}


clean_single_config() {
    local config_to_clean="$1"
    local clean_path="$OUTPUT_ROOT/$config_to_clean"
    
    echo "Cleaning $config_to_clean build..."
    if [ -d "$clean_path" ]; then
        if rm -rf "$clean_path"; then
            echo "Deleted: $clean_path"
        else
            echo "Error: Could not delete $clean_path (files may be in use)"
            return 1
        fi
    else
        echo "No $config_to_clean build to clean"
    fi
}

clean_all_configs() {
    echo "Cleaning ALL configs..."
    clean_single_config "debug"
    clean_single_config "release_with_debug"
    clean_single_config "release"
    clean_single_config "shipping"
}

menu_prompt() {
    while true; do
        if [ -f "$OUTPUT_ACTUAL" ]; then
            echo "Run Latest Build? (Y/N) Rebuild? (R) Clean? (C)"
            read -n 1 -r choice
            echo
            
            case $choice in
                [Yy])
                    echo "Running $OUTPUT_ACTUAL $RUN_ARGS"
                    ./"$OUTPUT_ACTUAL" $RUN_ARGS
                    echo
                    ;;
                [Nn])
                    echo "Exiting..."
                    break
                    ;;
                [Rr])
                    echo "Rebuilding..."
                    compile_stage && link_stage
                    echo
                    ;;
                [Cc])
                    clean_menu
                    echo
                    ;;
                *)
                    echo "Invalid choice, please try again"
                    echo
                    ;;
            esac
        else
            echo "Build not found. Would you like to rebuild? (Y/N)"
            read -n 1 -r choice
            echo
            case $choice in
                [Yy])
                    compile_stage && link_stage
                    ;;
                [Nn])
                    echo "Exiting..."
                    break
                    ;;
                *)
                    echo "Invalid choice"
                    ;;
            esac
        fi
    done
}


clean_menu() {
    while true; do
        echo
        echo "Clean Options:"
        echo "C. Clean current config ($CONFIG)"
        echo "A. Clean ALL configs"
        echo "1. Clean Debug config"
        echo "2. Clean Release_w_Debug config"  
        echo "3. Clean Release config"
        echo "4. Clean Shipping config"
        echo "0. Cancel"
        echo
        
        read -n 1 -r clean_choice
        echo
        
        case $clean_choice in
            [Cc])
                clean_single_config "$CONFIG"
                break
                ;;
            [Aa])
                clean_all_configs
                break
                ;;
            1)
                clean_single_config "debug"
                break
                ;;
            2)
                clean_single_config "release_with_debug"
                break
                ;;
            3)
                clean_single_config "release"
                break
                ;;
            4)
                clean_single_config "shipping"
                break
                ;;
            0)
                echo "Clean cancelled."
                break
                ;;
            *)
                echo "Invalid option"
                ;;
        esac
    done
}

if compile_stage && link_stage; then
    menu_prompt
else
    echo "Build failed!"
    exit 1
fi

exit 0