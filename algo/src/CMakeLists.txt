cmake_minimum_required(VERSION 3.16)
project(procon32 LANGUAGES CXX CUDA)

find_package(OpenMP)
add_executable(solver solver.cu)

add_executable(tmp tmp.cpp)

#set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS}")
set(CMAKE_CUDA_FLAGS_RELEASE "-O3 -DNDEBUG -Xcompiler=\"-O3 -DNDEBUG\"")
set(CMAKE_CUDA_FLAGS_RELWITHDEBINFO "-g -O3 -DDEBUG -Xcompiler=\"-g -O3 -DDEBUG\"")
set(CMAKE_CUDA_FLAGS_DEBUG "-g -O0 -DDEBUG -Xcompiler=\"-Wall -g -O0 -DDEBUG\"")

set(CMAKE_CXX_FLAGS "-std=c++17 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

set_target_properties( solver
    PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    # RUNTIME_OUTPUT_DIRECTORY "../../bin/"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler=${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")

# set(CMAKE_VERBOSE_MAKEFILE ON)