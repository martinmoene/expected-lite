# Copyright (C) 2016 Martin Moene.
#
# This code is licensed under the MIT License (MIT).

project( test )

include_directories  ( "${PROJECT_SOURCE_DIR}/../include/nonstd" )

set( SOURCES expected.t.cpp )

add_definitions( -Dnsel_CONFIG_CONFIRMS_COMPILATION_ERRORS=0 )
add_definitions( -Dlest_FEATURE_AUTO_REGISTER=1 )

set( SINGLE_TEST_PROGRAM TRUE )

# GNU: have -std=c++14 and lower?
if( ${CMAKE_CXX_COMPILER_ID} MATCHES GNU )
    if( CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9.2 )
        message( FATAL_ERROR "GCC 4.9.2 or higher required for option -std=c++14" )
    endif()
endif()

if( ${CMAKE_CXX_COMPILER_ID} MATCHES MSVC )
    if( MSVC_VERSION VERSION_LESS 1900 )
        message( FATAL_ERROR "MSVC 14 (1900) or newer required" )
    endif()

    add_executable ( expected-lite.t ${SOURCES} )

    target_compile_options( expected-lite.t PUBLIC -W3 -EHsc -wd4814 )

elseif( ${CMAKE_CXX_COMPILER_ID} MATCHES GNU OR
        ${CMAKE_CXX_COMPILER_ID} MATCHES Clang )
    add_executable( expected-lite-cpp11.t ${SOURCES} )
    add_executable( expected-lite-cpp14.t ${SOURCES} )

    add_compile_options( -Wall -Wno-missing-braces )

    target_compile_options( expected-lite-cpp11.t PUBLIC -std=c++11 )
    target_compile_options( expected-lite-cpp14.t PUBLIC -std=c++14 )

    set( SINGLE_TEST_PROGRAM FALSE )

elseif( ${CMAKE_CXX_COMPILER_ID} MATCHES Intel )
# as is
else()
# as is
endif()

# configure unit tests via CTest:

enable_testing()

 if( ${SINGLE_TEST_PROGRAM} )
add_test( NAME test          COMMAND expected-lite.t )
add_test( NAME list_version  COMMAND expected-lite.t --version )
add_test( NAME list_tags     COMMAND expected-lite.t --list-tags )
add_test( NAME list_tests    COMMAND expected-lite.t --list-tests )
 else()
add_test( NAME test-cpp11    COMMAND expected-lite-cpp11.t )
add_test( NAME test-cpp14    COMMAND expected-lite-cpp14.t )
add_test( NAME list_version  COMMAND expected-lite-cpp14.t --version )
add_test( NAME list_tags     COMMAND expected-lite-cpp14.t --list-tags )
add_test( NAME list_tests    COMMAND expected-lite-cpp14.t --list-tests )
 endif()

# end of file
