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





