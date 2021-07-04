# zninja-build
Tool to automate c++ build, compatible with VSCode Editor

Addopted for gcc/g++ compiler. 

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

-s or --section is name of section we want to perform action, in our case is building and linking ( this argument is required, it must be an relative path to section from workspace path. For example if we have workspace at ~/projects  section zninja-build path should be ~/projects/zninja-build  and argument for it should be zninja-build unless we want to compile sigle file (main.cpp for example). In that case it should be zninja-build/main.cpp

--link tells program to link all project object to executable or shared library.



# BASIC VSCODE EDITOR SET UP
tasks.json
```json
{
    "tasks": [
        
        {
            "type": "shell",
            "label": "ZNINJA-BUILD Build release section and set as active",
            "command": "echo -n ${relativeFileDirname} | grep -oP  '^(.*?)(?=/|$)' | while read n;do ln -sfr \"${workspaceFolder}/bin/linux/release/$n\" \"${workspaceFolder}/bin/active_release\";done",
            "args": [],
            "options": {
                "cwd": "${workspaceRoot}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "detail": "Build debug build variant",
           "dependsOn":["ZNINJA-BUILD Build release Section"]
        },
        {
            "type": "shell",
            "label": "ZNINJA-BUILD Build debug section and set as active",
            "command": "echo -n ${relativeFileDirname} | grep -oP  '^(.*?)(?=/|$)' | while read n;do ln -sfr \"${workspaceFolder}/bin/linux/debug/$n\" \"${workspaceFolder}/bin/active_dbg\";done",
            "args": [],
            "options": {
                "cwd": "${workspaceRoot}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "detail": "Build debug build variant",
           "dependsOn":["ZNINJA-BUILD Build debug Section"]
        },
        {
            "type": "cppbuild",
            "label": "ZNINJA-BUILD Build debug Section",
            "command": "./bin/zninja-build",
            "args": [
                "-w",
                "${workspaceRoot}",
                "-s",
                "${relativeFile}",
                "-p",
                "linux",
                "--build-variant",
                "debug",
                "--load",
                "--build",
                "--link" ,
                "-t",
                "4" ,
                "--diagnostics-color"                     
            ],
            "options": {
                "cwd": "${workspaceRoot}"
            },
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": [
                "$gcc"
            ],
            "detail": "Build debug build variant",
        },
        {
            "type": "cppbuild",
            "label": "ZNINJA-BUILD Build release Section",
            "command": "./bin/zninja-build",
            "args": [
                "-w",
                "${workspaceRoot}",
                "-s",
                "${relativeFile}",
                "-p",
                "linux",
                "--build-variant",
                "release",
                "--load",
                "--build",
                "--link" ,
                "-t",
                "4",
                "--diagnostics-color"                   
            ],
            "options": {
                "cwd": "${workspaceRoot}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "detail": "Build release build variant",
        },
        {
            "type": "cppbuild",
            "label": "ZNINJA-BUILD Rebuild debug Section",
            "command": "./bin/zninja-build",
            "args": [
                "-w",
                "${workspaceRoot}",
                "-s",
                "${relativeFile}",
                "-p",
                "linux",
                "--build-variant",
                "debug",
                "--load",
                "--rebuild",
                "--link" ,
                "-t",
                "4",
                "--diagnostics-color"          
            ],
            "options": {
                "cwd": "${workspaceRoot}"
            },
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": [
                "$gcc"
            ],
            "detail": "Rebuild debug build variant",
        },
        {
            "type": "cppbuild",
            "label": "ZNINJA-BUILD Rebuild release Section",
            "command": "./bin/zninja-build",
            "args": [
                "-w",
                "${workspaceRoot}",
                "-s",
                "${relativeFile}",
                "-p",
                "linux",
                "--build-variant",
                "release",
                "--load",
                "--rebuild",
                "--link" ,
                "-t",
                "4",
                "--diagnostics-color"                        
            ],
            "options": {
                "cwd": "${workspaceRoot}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "detail": "Rebuild release build variant",
        },
        {
            "type": "process",
            "label": "ZNINJA-BUILD Section Init",
            "command": "./bin/zninja-build",
            "args": [
                "-w",
                "${workspaceRoot}",
                "-s",
                "${relativeFile}",
                "-p",
                "linux",
                "--build-variant",
                "release",
                "--load",
                "--show"
            ],
            "options": {
                "cwd": "${workspaceRoot}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "detail": "Create build variants Relase/Debug default configuration",
            "dependsOn": [
                "ZNINJA-BUILD Section Init Release","ZNINJA-BUILD Section Init Debug"
            ],
            "dependsOrder": "sequence"  
        },
        {
            "type": "process",
            "label": "ZNINJA-BUILD Section Init Release",
            "command": "./bin/zninja-build",
            "args": [
                "-w",
                "${workspaceRoot}",
                "-s",
                "${relativeFile}",
                "-p",
                "linux",
                "--build-variant",
                "release",
                "--cc",
                "gcc",
                "--cxx",
                "g++",
                "--cxxflags",
                "-std=c++17 -fexceptions -Wall -O2",
                "--cppflags",
                "-I${workspaceRoot}",
                "--save"            ],
            "options": {
                "cwd": "${workspaceRoot}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "detail": "Save default release configuration for section",
            },
        {
            "type": "process",
            "label": "ZNINJA-BUILD Section Init Debug",
            "command": "./bin/zninja-build",
            "args": [
                "-w",
                "${workspaceRoot}",
                "-s",
                "${relativeFile}",
                "-p",
                "linux",
                "--build-variant",
                "debug",
                "--cc",
                "gcc",
                "--cxx",
                "g++",
                "--cxxflags",
                "-std=c++17 -fexceptions -Wall -g",
                "--cppflags",
                "-I${workspaceRoot}",
                "--save"            ],
            "options": {
                "cwd": "${workspaceRoot}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "detail": "Save default debug configuration for section",
            },
    ],
    "version": "2.0.0"
}
```
launch.json
```json
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "ZNINJA-BUILD DBG",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceRoot}/bin/active_dbg",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceRoot}/bin/",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
        
            "preLaunchTask": "ZNINJA-BUILD Build debug section and set as active" ,
            "miDebuggerPath": "/usr/bin/gdb"
        }
    ]
}
```
I have seen vscode editor as good choice to replace my tool for code editing and compiling. So I decided to give this editor automated functionallity which fits my needs. But later it turns out it is not what I was seeking for. 

Code editor which eats that much resources is not good editor for serious project starting.
While my old code editor needs about 200 MB of ram for this project to write.
To build main.cpp file, you will need up to 1 GB of RAM with g++ on 64 bit machine(linux).
I have seen, vscode can use more then 1 GB of RAM. That is too much for code editor.
Beside that, it has few annoying functionality, which should not exists by paying that much high price in resoruces.
So I stoped addopting this project to other platform.


My understanding of workspace is to have one or more projects in workspace, while VSCODE see workspace as a project or single file as a project.
So, goal was to automate project/section compilation of vscode active (directory/workspace) and consider them as modules of a workspace.

