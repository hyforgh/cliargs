cmake_minimum_required(VERSION 3.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(cliargs)

cliargs_get_version(VERSION)

project(cliargs VERSION ${VERSION} LANGUAGES CXX)
set("PROJECT_DESCRIPTION" "A header-only C++ command line interface argument parser")
set("PROJECT_HOMEPAGE_URL" "https://github.com/hyforgh/cliargs")

include(GNUInstallDirs)

set(CLIARGS_STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CLIARGS_STANDALONE_PROJECT ON)
endif()

option(CLIARGS_BUILD_EXAMPLES "Set to ON to build examples" ${CLIARGS_STANDALONE_PROJECT})
option(CLIARGS_BUILD_TESTS "Set to ON to build tests" ${CLIARGS_STANDALONE_PROJECT})
option(CLIARGS_ENABLE_INSTALL "Set to ON to generate install target" ${CLIARGS_STANDALONE_PROJECT})

add_library(cliargs INTERFACE)
add_library(cliargs::cliargs ALIAS cliargs)
add_subdirectory(include)

if (CLIARGS_ENABLE_INSTALL)
    cliargs_install()
endif()

function(cliarg_add_inner_bin_target target_name)
    add_executable(${target_name} ${target_name}.cpp)
    message(STATUS "Building target: ${target_name}")
    target_compile_features(${target_name} PRIVATE cxx_std_11)
    target_compile_options(${target_name} PRIVATE -Wall)
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction(cliarg_add_inner_bin_target target_name)

if (CLIARGS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (CLIARGS_BUILD_TESTS)
    add_subdirectory(tests)
endif()
