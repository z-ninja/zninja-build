# zninja-build
Tool to automate c++ build, compatible with VSCode Editor

Addopted for g++ compiler. 

Build

g++ -std=c++17 -DOS_LINUX -fexceptions -Wall -O2 -m64 -c main.cpp -o main.o

g++ -std=c++17 -DOS_LINUX -fexceptions -Wall -O2 -m64 -c endian/bree_endian.cpp -o bree_endian.o

g++ -std=c++17 -DOS_LINUX -fexceptions -Wall -O2 -m64 -c endian/bree_endian.cpp -o bree_endian.o

Link

g++ -o zninja-build sha256.o bree_endian.o main.o -m64 -lpthread -lboost_filesystem -lboost_program_options -s




