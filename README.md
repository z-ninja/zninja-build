# zninja-build
Tool to automate c++ build, compatible with VSCode Editor

Addopted for g++ compiler. 

# Build
```cpp
g++ -std=c++17 -DOS_LINUX -fexceptions -Wall -O2 -m64 -c main.cpp -o main.o
g++ -std=c++17 -DOS_LINUX -fexceptions -Wall -O2 -m64 -c endian/bree_endian.cpp -o bree_endian.o
g++ -std=c++17 -DOS_LINUX -fexceptions -Wall -O2 -m64 -c endian/bree_endian.cpp -o bree_endian.o
```
# Link
```cpp
g++ -o zninja-build sha256.o bree_endian.o main.o -m64 -lpthread -lboost_filesystem -lboost_program_options -s
```
# Arguments List
```cpp
Generic Arguments:

  --version                       Outputs Program Version
  -h [ --help ]                   Help screen

Required Arguments:

  -w [ --workspace-root ] arg     Absolute Workspace Path
  -s [ --section ] arg            Section Name or Section Relative Path From 
                                  Workspace Root (Example section_one/)                          
  -p [ --platform ] arg           Platform (All,Android,iOS,Linux,OSX,Windows,W
                                  SL)

Configuration:

  --save                          Save configuration for this section to file
  --load                          Load configuration for this section from file
  --show                          Show loaded configuration for this section
  --cc arg                        Set C compiler path
  --cxx arg                       Set C++ compiler path
  --cflags arg                    Set flags for C compiler
  --cxxflags arg                  Set flags for C++ compiler
  --cppflags arg                  Set flags for both, C & C++ compiler
  --ldflags arg                   Set flags for linker
  --ldlibs arg                    Set libs for linker

Compile:

  --build                         Compile all modified files for section
  --rebuild                       Rebuild whole section
  --clean                         Clean active section object files
  --link                          Link all objects of section to executable
  --build-static-lib              Link static library
  --build-variant arg (=default)  Build configuration variant (debug, release)
  --out arg                       Name of Executable to generate to (default is
                                  section name)                        
  -t [ --max-threads ] arg        Maximum threads to be used for compiling
  --build-file                    Compile file (compile file path is --section 
                                  argument)                           
  --diagnostics-color             Turn on diagnostics color of compiler
  ```

# Manual Use Example from command line
Go to workspace directory where zninja-build project is located.
```cpp
./bin/zninja-build -w `pwd` --cc gcc --cxx g++ --build-variant release -p linux --show --cxxflags "-std=c++17 -DOS_LINUX -fexceptions -Wall -O2 -m64" --ldlibs "-m64 -lpthread -lboost_filesystem -lboost_program_options -s"  --build  --diagnostics-color -t 4 -s zninja-build --link
```
-w is workspace directory for example ~/projects
--cc is C compiler path, gcc or /usr/bin/gcc

--cxx is C++ compiler path, g++ or /usr/bin/g++

--build-variant is branch of build type. In above example build-variant is realease, each build-variant can have its own configuration which can be loaded via --load parameter but previouslly must be saved with --save parameter, so next time we call build variant we can use --load parameter instead passing all compiler arguments trough command line. 

For example: 
```cpp
// saving configuration for build variant release
./bin/zninja-build -w `pwd` --cc gcc --cxx g++ --build-variant release -p linux --show --cxxflags "-std=c++17 -DOS_LINUX -fexceptions -Wall -O2 -m64" --ldlibs "-m64 -lpthread -lboost_filesystem -lboost_program_options -s" -s zninja-build --save

// loading configuration for build variant release
/bin/zninja-build -w `pwd` --cc gcc --cxx g++ --build-variant release -p linux -s zninja-build --load --build --link
```

-p is operating system branch for objects,realease, configuration to be located at, in our case "linux"

--show this argument will output loaded or passed compiler configuration

--cxxflags  are cxxflags for c++ compiler to be used for compilation.

--ldlibs are linker flags to be used for linking

--build tells program to compile each source file if conditions are meet, meaning program will test if source file is modified and each header found in source file and their dependency. So by modifying headers can require for program to recompile one or more source file. There is also --rebuild parameter which force program to recompile all files and also --build-file will compile single source file.

Detecting modification is done by generating sha256 hash of each soruce/header file and comparing it with previous known hash of same file. 

--diagnostics-color will force compiler to output colored diagnostics text ( gcc/g++)

-t tells program how much source file can be compiled at once in parallel.

--link tells program to link all project object to executable or shared library.





