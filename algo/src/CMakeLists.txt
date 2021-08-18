cmake_minimum_required(VERSION 3.16)
#project(procon32 LANGUAGES CXX CUDA)
project(procon32 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

find_package(OpenMP)

#set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS}")
#set(CMAKE_CUDA_FLAGS_RELEASE "-O3 -DNDEBUG -Xcompiler=\"-O3 -DNDEBUG\"")
#set(CMAKE_CUDA_FLAGS_RELWITHDEBINFO "-g -O3 -DDEBUG -Xcompiler=\"-g -O3 -DDEBUG\"")
#set(CMAKE_CUDA_FLAGS_DEBUG "-g -O0 -DDEBUG -Xcompiler=\"-Wall -g -O0 -DDEBUG\"")

set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -fopenmp")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3 -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler=${OpenMP_CXX_FLAGS}")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")

# set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(utility)
add_subdirectory(common_lib)
add_subdirectory(image_restorer)
# add_subdirectory(build_procedure)
# add_executable(tmp tmp.cpp)

add_executable(main main.cpp)
target_link_libraries(main utility)
