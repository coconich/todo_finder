# todo_finder
command line tool to find [symbol][keyword] combos in your files 
<img width="890" height="899" alt="image" src="https://github.com/user-attachments/assets/3ca9c214-68a2-4a44-8214-4ec147275427" />

# usage
I leave comments all over my codebases like these
```c
// @todo:: fix this bug
// @cleanup
// @nocheckin
// @alloc
// @perf:: will this be an issue?
// @hack:: what?!?!
// ...
```
running the todo_finder in a directory will search from there into all subfolders for default [symbol][keyword] combos
Currently, I just put a copy of the todo in the codebase src folder, then call into with a key binding in my editor to quickly get a printout while working
When finished, I will just have a copy in my utils folder within PATH, then call it as needed from wherever

# building
The build.bat script is a simple build system that builds for clang and gcc (using wsl).
once running, it will run in a loop and let you continue to run/clean/rebuild as desired 

call it with a compiler and config arg from the following choices 
  - build.bat clang debug
  - build.bat clang shipping
  - build.bat gcc debug
  - build.bat gcc shipping

the clang build script assumes the following is going to work, replace this path if needed for your installation
  - "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

If you want to build with gcc on a non windows platform, just call build_compiler_gcc.sh with the config argument desired. the build.bat is just an interface to choose a config, (it sometimes has a lot more compilers/platform options)
  - ./build_compiler_gcc.sh debug
  - ./build_compiler_gcc.sh shipping

# what's next?
When basic user arguments are finished, you will be able to specify a directory or file and the way to traverse it. 
The intended usage, for me, is to put this in my utils folder within my path, and then specify the directory to traverse per project in my editor.

When user configurations are finished, you will be able to specify symbols, keywords, extensions/folders to ignore.
This will allow you to extend functionality and customize slightly, I plan to use this to ignore folders per project by including a config file within the folders I am planning on traversing. (it will find the config file there first before searching the AppData/Home folders for a config file before using the default)

Extend the build scripts to allow for a config switch to happen so you do not have to close and reopen the script. 
