#!/bin/bash

# Clean up previous builds
rm -rf build
mkdir "build"
cp code.ved ./build
cd build || exit

# Generate CMake build files
cmake ..

# Build the project
make

# Generate LLVM IR
./comprint > output.ll

# Check that output.ll was generated
ls
cat output.ll

# Compile the LLVM IR to an object file
llc -filetype=obj -march=x86-64 output.ll -o output.o

# Check if the object file was generated
ls

# Link the object file to create the static executable
gcc output.o -o program -static

# Ensure the program is executable
chmod +x program

# Run the program with the filename as an argument
./program code.ved

# Cleanup intermediate files
rm output.ll output.o

echo "Build and execution completed."







##!/bin/bash
#
## Clean up previous builds
#rm -rf build
#mkdir "build"
#cp code.ved ./build
#cd build || exit
#
## Generate CMake build files
#cmake ..
#
## Build the project
#make
#
## Generate LLVM IR
#./comprint > output.ll
#
## Check that output.ll was generated
#ls
#cat output.ll
#
## Compile the LLVM IR to an object file
#llc -filetype=obj -march=x86-64 output.ll -o output.o
#
## Check if the object file was generated
#ls
#
## Link the object file to create the static executable
#gcc output.o -o program -static
#
## Ensure the program is executable
#chmod +x program
#
## Run the program with the filename as an argument
#./program code.ved
#
## Cleanup intermediate files
#rm output.ll output.o
#
#echo "Build and execution completed."
