cmake_minimum_required(VERSION 3.10)
project(clang-include-graph)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

option(LLVM_CONFIG_PATH "Path to custom llvm-config executable")

option(BOOST_STATIC "Enable linking static Boost libraries" OFF)

option(WITH_JSON "Add JSON output support" ON)

#
# Setup version string
#
include(GitVersion)
setup_git_version()
message(STATUS "clang-include-graph version: "
        "${GIT_VERSION_MAJOR}.${GIT_VERSION_MINOR}.${GIT_VERSION_PATCH}")
if("${GIT_VERSION}" STREQUAL "")
    set(GIT_VERSION "${GIT_VERSION_MAJOR}.${GIT_VERSION_MINOR}.${GIT_VERSION_PATCH}")
endif()
message(STATUS "Effective clang-include-version is: ${GIT_VERSION}")

if (LLVM_CONFIG_PATH)
    message(STATUS "Using llvm-config from ${LLVM_CONFIG_PATH}")
    set(LIBCLANG_LLVM_CONFIG_EXECUTABLE ${LLVM_CONFIG_PATH})
    set(LLVM_CONFIG_BINARY ${LLVM_CONFIG_PATH})
endif (LLVM_CONFIG_PATH)

message(STATUS "Checking for libclang...")
set(LLVM_PREFERRED_VERSION 16.0.0)

find_package(LibClang REQUIRED)
message(STATUS "Linking libClang from ${LIBCLANG_LIBRARIES}")

set(Boost_USE_STATIC_LIBS ${BOOST_STATIC})
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_DEBUG_RUNTIME OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_LOG_NO_LIB ON)

set(BOOST_COMPONENTS graph program_options filesystem log_setup log unit_test_framework)
if(WITH_JSON)
    list(APPEND BOOST_COMPONENTS json)
    add_compile_definitions(WITH_JSON_OUTPUT)
endif(WITH_JSON)
find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})
include_directories(${Boost_INCLUDE_DIRS})

set(THIRDPARTY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/thirdparty/)
include_directories(${THIRDPARTY_HEADERS_DIR})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DCINDEX_NO_EXPORTS -DLIBCLANG_VERSION_STRING=\\\"${LIBCLANG_VERSION_STRING}\\\" -DGIT_VERSION=\\\"${GIT_VERSION}\\\" ${LIBCLANG_CXXFLAGS}")
else(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-deprecated-copy-with-user-provided-copy -DLIBCLANG_VERSION_STRING=\\\"${LIBCLANG_VERSION_STRING}\\\" -DGIT_VERSION=\\\"${GIT_VERSION}\\\" -DBOOST_LOG_DYN_LINK ${LIBCLANG_CXXFLAGS}")
endif(MSVC)

file(GLOB_RECURSE SOURCES src/*.cc src/*.h)
if(NOT WITH_JSON)
    list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/include_graph_json_printer.cc)
    list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/include_graph_json_printer.h)
endif(NOT WITH_JSON)
set(MAIN_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc)
list(REMOVE_ITEM SOURCES ${MAIN_SOURCE_FILE})

add_library(lib-clang-include-graph OBJECT ${SOURCES})

add_executable(clang-include-graph $<TARGET_OBJECTS:lib-clang-include-graph> src/main.cc)

target_link_libraries(clang-include-graph ${LIBCLANG_LIBRARIES}
        Boost::filesystem Boost::graph Boost::program_options Boost::log)
if(WITH_JSON)
    target_link_libraries(clang-include-graph Boost::json)
endif(WITH_JSON)

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
install(FILES AUTHORS.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

#
# Setup installer
#
set(CPACK_PACKAGE_NAME "clang-include-graph")
set(CPACK_PACKAGE_VENDOR "Bartek Kryza <bkryza@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "clang-include-graph - Simple tool for analyzing C++ project include graph")
set(CPACK_PACKAGE_VERSION "${GIT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${GIT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${GIT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${GIT_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "clang-include-graph")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md)

if(MSVC)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_DISPLAY_NAME "clang-include-graph")
    set(CPACK_NSIS_HELP_LINK "https://github.com/bkryza/clang-include-graph")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/bkryza/clang-include-graph")
    set(CPACK_NSIS_CONTACT "Bartek Kryza <bkryza@gmail.com>")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_SOURCE_GENERATOR "ZIP")
endif(MSVC)

include(CPack)

# Enable testing via CTest
enable_testing()
add_subdirectory(tests)
