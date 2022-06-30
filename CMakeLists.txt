cmake_minimum_required(VERSION 3.22)
project(clang_include_graph)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

option(LLVM_CONFIG_PATH "Path to custom llvm-config executable")

if(LLVM_CONFIG_PATH)
    message(STATUS "Using llvm-config from ${LLVM_CONFIG_PATH}")
    set(LIBCLANG_LLVM_CONFIG_EXECUTABLE ${LLVM_CONFIG_PATH})
    set(LLVM_CONFIG_BINARY ${LLVM_CONFIG_PATH})
endif(LLVM_CONFIG_PATH)


message(STATUS "Checking for libclang...")
set(LLVM_PREFERRED_VERSION 12.0.0)
# Add
#   -DLLVM_CONFIG_PATH=/path/to/llvm-config
# to use custom LLVM version
find_package(LibClang REQUIRED)

include_directories()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS graph program_options)

include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -std=c++17 -DLIBCLANG_VERSION_STRING=\\\"${LIBCLANG_VERSION_STRING}\\\" ${LIBCLANG_CXXFLAGS}")

add_executable(clang-include-graph main.cc)

target_link_libraries(clang-include-graph ${LIBCLANG_LIBRARIES} ${Boost_LIBRARIES})