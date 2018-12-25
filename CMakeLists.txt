# Copyright 2016-2018 by Martin Moene
#
# https://github.com/martinmoene/expected-lite
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required( VERSION 3.5 )

# expected-lite project and version, updated by script/update-version.py:

project(
    expected_lite
    VERSION 0.2.0
    DESCRIPTION "Expected objects in C++11 and later in a single-file header-only library"
#   HOMEPAGE_URL "https://github.com/martinmoene/expected-lite"
    LANGUAGES CXX )

set( ${PROJECT_NAME}_version "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" )

# Toplevel or subproject:

if ( CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME )
    set( expected_IS_TOPLEVEL_PROJECT TRUE )
else()
    set( expected_IS_TOPLEVEL_PROJECT FALSE )
endif()

# If toplevel project, enable building and performing of tests, disable building of examples:

option( EXPECTED_LITE_OPT_BUILD_TESTS    "Build and perform expected-lite tests" ${expected_IS_TOPLEVEL_PROJECT} )
option( EXPECTED_LITE_OPT_BUILD_EXAMPLES "Build expected-lite examples" OFF )
set(    EXPEXTED_P0323R  "99" STRING     "Specify proposal revision compatibility (99: latest)" )

option( EXPECTED_LITE_OPT_SELECT_STD     "Select std::expected"    OFF )
option( EXPECTED_LITE_OPT_SELECT_NONSTD  "Select nonstd::expected" OFF )

include( GNUInstallDirs )

set( package_name "expected-lite" )

set( include_source_dir "${PROJECT_SOURCE_DIR}/include" )

# Interface library:

add_library(
    ${package_name} INTERFACE )

target_include_directories(
    ${package_name} INTERFACE "$<BUILD_INTERFACE:${include_source_dir}>" )

# Installation:

install(
    DIRECTORY   ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )

# If requested, build and perform tests, build examples:

enable_testing()

if ( EXPECTED_LITE_OPT_BUILD_TESTS )
    add_subdirectory( test )
endif()

if ( EXPECTED_LITE_OPT_BUILD_EXAMPLES )
    add_subdirectory( example )
endif()

# end of file
