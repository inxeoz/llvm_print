cmake_minimum_required(VERSION 3.16)
project(comprint)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

# Add your executable
add_executable(comprint main.cpp)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Use llvm-config to link required LLVM libraries
execute_process(COMMAND llvm-config --libs interpreter executionengine core target irreader analysis
        OUTPUT_VARIABLE LLVM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(COMMAND llvm-config --system-libs
        OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set the linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LDFLAGS}")

# Link all required libraries
target_link_libraries(comprint ${LLVM_LIBS} ${LLVM_SYSTEM_LIBS})
target_include_directories(comprint PRIVATE ${LLVM_INCLUDE_DIRS})
