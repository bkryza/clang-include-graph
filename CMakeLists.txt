cmake_minimum_required(VERSION 3.10)
project(clang_include_graph)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

option(LLVM_CONFIG_PATH "Path to custom llvm-config executable")

option(GIT_VERSION "clang-include-graph version" "0.1.0")

option(BOOST_STATIC "Enable linking static Boost libraries" OFF)

if (LLVM_CONFIG_PATH)
    message(STATUS "Using llvm-config from ${LLVM_CONFIG_PATH}")
    set(LIBCLANG_LLVM_CONFIG_EXECUTABLE ${LLVM_CONFIG_PATH})
    set(LLVM_CONFIG_BINARY ${LLVM_CONFIG_PATH})
endif (LLVM_CONFIG_PATH)

message(STATUS "Checking for libclang...")
set(LLVM_PREFERRED_VERSION 16.0.0)
# Add
#   -DLLVM_CONFIG_PATH=/path/to/llvm-config
# to use custom LLVM version
find_package(LibClang REQUIRED)
#
## Configure executable version
#file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/src/version)
#configure_file(src/version.h.in ${PROJECT_BINARY_DIR}/src/version/version.h)

include_directories()

set(Boost_USE_STATIC_LIBS ${BOOST_STATIC})
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS graph program_options filesystem log unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

set(THIRDPARTY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/thirdparty/)
include_directories(${THIRDPARTY_HEADERS_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-deprecated-copy-with-user-provided-copy -std=c++11 -DLIBCLANG_VERSION_STRING=\\\"${LIBCLANG_VERSION_STRING}\\\" -DGIT_VERSION=\\\"${GIT_VERSION}\\\" -DBOOST_LOG_DYN_LINK ${LIBCLANG_CXXFLAGS}")

file(GLOB_RECURSE SOURCES src/*.cc src/*.h)
set(MAIN_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
list(REMOVE_ITEM SOURCES ${MAIN_SOURCE_FILE})

add_library(lib-clang-include-graph OBJECT ${SOURCES})

add_executable(clang-include-graph $<TARGET_OBJECTS:lib-clang-include-graph> src/main.cc)

target_link_libraries(clang-include-graph ${LIBCLANG_LIBRARIES} Boost::filesystem Boost::graph Boost::program_options Boost::log)

find_program(
        CLANG_TIDY
        NAMES "run-clang-tidy-18"
        DOC "Path to run-clang-tidy script")

if(CLANG_TIDY)
    include(ProcessorCount)
    ProcessorCount(CLANG_TIDY_PARALLEL_JOBS)
    message(STATUS
            "run-clang-tidy script found: ${CLANG_TIDY} - adding target clang-tidy")
    set(CLANG_TIDY_SOURCE_FILTER "src/*.cc")
    file(GLOB_RECURSE CLANG_TIDY_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/${CLANG_TIDY_SOURCE_FILTER}")
    add_custom_target(clang-tidy COMMAND ${CLANG_TIDY}
            -export-fixes clang-tidy-suggested-fixes.yaml
            -j ${CLANG_TIDY_PARALLEL_JOBS}
            ${CLANG_TIDY_SOURCES})
else(CLANG_TIDY)
    message(STATUS "run-clang-tidy script not found - target clang-tidy not available")
endif(CLANG_TIDY)

include(GNUInstallDirs)

install(TARGETS clang-include-graph DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES LICENSE.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

# Enable testing via CTest
enable_testing()
add_subdirectory(tests)
