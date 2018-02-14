# Copyright 2016-2018 by Martin Moene
#
# https://github.com/martinmoene/expected-lite
#
# This code is licensed under the MIT License (MIT).

cmake_minimum_required( VERSION 3.0 )

project( test )

set( MAIN_BASE expected-lite )
set( HDRNAME   expected.hpp    )
set( SOURCES   ${MAIN_BASE}.t.cpp expected.t.cpp )

set( HDRDIR  ${PROJECT_SOURCE_DIR}/../include/nonstd )
set( HDRPATH ${HDRDIR}/${HDRNAME} )

set( OPTIONS "" )
set( DEFCMN  "-Dlest_FEATURE_AUTO_REGISTER=1" )

set( HAS_STD_FLAGS  FALSE )
set( HAS_CPP98_FLAG FALSE )
set( HAS_CPP03_FLAG FALSE )
set( HAS_CPP11_FLAG FALSE )
set( HAS_CPP14_FLAG FALSE )
set( HAS_CPP17_FLAG FALSE )
set( HAS_CPP20_FLAG FALSE )

if( MSVC )
    set( HAS_STD_FLAGS TRUE )

    set( STD_OPT     -std: )
    set( OPTIONS     -W3 -EHsc )
    set( DEFINITIONS -D_SCL_SECURE_NO_WARNINGS -DNOMINMAX ${DEFCMN} )

    if( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.00 )
        set( HAS_CPP14_FLAG TRUE )
        set( HAS_CPPLATEST_FLAG TRUE )
    endif()
    if( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.11 )
        set( HAS_CPP17_FLAG TRUE )
    endif()

elseif( CMAKE_CXX_COMPILER_ID MATCHES GNU OR
        CMAKE_CXX_COMPILER_ID MATCHES Clang )

    set( HAS_STD_FLAGS  TRUE )
    set( HAS_CPP98_FLAG TRUE )
    set( HAS_CPP03_FLAG TRUE )

    set( STD_OPT     -std= )
    set( OPTIONS     -Wall -Wno-missing-braces -fno-elide-constructors )
    set( DEFINITIONS ${DEFCMN} )

    # GNU: available -std flags depends on version
    if( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
        if( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8.0 )
            set( HAS_CPP11_FLAG TRUE )
        endif()
        if( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9.2 )
            set( HAS_CPP14_FLAG TRUE )
        endif()
        if( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.1.0 )
            set( HAS_CPP17_FLAG TRUE )
        endif()
    endif()

    # Clang: available -std flags depends on version
    if( CMAKE_CXX_COMPILER_ID MATCHES Clang )
        if( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3.0 )
            set( HAS_CPP11_FLAG TRUE )
        endif()
        if( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4.0 )
            set( HAS_CPP14_FLAG TRUE )
        endif()
        if( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0.0 )
            set( HAS_CPP17_FLAG TRUE )
        endif()
    endif()

elseif( CMAKE_CXX_COMPILER_ID MATCHES Intel )
# as is
else()
# as is
endif()

# enable MS C++ Core Guidelines checker if MSVC:

function( enable_msvs_guideline_checker target )
    if( MSVC )
        set_target_properties( ${target} PROPERTIES
        VS_GLOBAL_EnableCppCoreCheck true
        VS_GLOBAL_CodeAnalysisRuleSet CppCoreCheckRules.ruleset
        VS_GLOBAL_RunCodeAnalysis true )
    endif()
endfunction()

# make target, compile for given standard if specified:

function( make_target target std )
    if( std )
        set( OPTIONAL_STD ${STD_OPT}${std} )
    endif()
    add_executable            ( ${target} ${SOURCES} ${HDRPATH} )
    target_compile_options    ( ${target} PRIVATE ${OPTIONS} ${OPTIONAL_STD} )
    target_compile_definitions( ${target} PRIVATE ${DEFINITIONS} )
    target_include_directories( ${target} PRIVATE ${HDRDIR} )
endfunction()

# add generic executable, unless -std flags can be specified:

if( NOT HAS_STD_FLAGS )
    make_target( ${MAIN_BASE}.t "" )
else()
#    # unconditionally add C++98 variant as MSVC has no option for it:
#    if( HAS_CPP98_FLAG )
#        make_target( ${MAIN_BASE}-cpp98.t c++98 )
#    else()
#        make_target( ${MAIN_BASE}-cpp98.t "" )
#    endif()
#
#    if( HAS_CPP03_FLAG )
#        make_target( ${MAIN_BASE}-cpp03.t c++03 )
#    endif()

    # unconditionally add C++11 variant as MSVC has no option for it:
    if( HAS_CPP11_FLAG )
        make_target( ${MAIN_BASE}-cpp11.t c++11 )
    else()
        make_target( ${MAIN_BASE}-cpp11.t "" )
    endif()

    if( HAS_CPP14_FLAG )
        make_target( ${MAIN_BASE}-cpp14.t c++14 )
    endif()

    if( HAS_CPP17_FLAG )
        make_target( ${MAIN_BASE}-cpp17.t c++17 )
        enable_msvs_guideline_checker( ${MAIN_BASE}-cpp17.t )
    endif()

    if( HAS_CPPLATEST_FLAG )
        make_target( ${MAIN_BASE}-cpplatest.t c++latest )
    endif()
endif()

# configure unit tests via CTest:

enable_testing()

if( HAS_STD_FLAGS )
#    # unconditionally add C++98 variant for MSVC:
#    add_test(     NAME test-cpp98     COMMAND ${MAIN_BASE}-cpp98.t )
#
#    if( HAS_CPP03_FLAG )
#        add_test( NAME test-cpp03     COMMAND ${MAIN_BASE}-cpp03.t )
#    endif()

    # unconditionally add C++11 variant for MSVC:
    add_test( NAME test-cpp11     COMMAND ${MAIN_BASE}-cpp11.t )

    if( HAS_CPP14_FLAG )
        add_test( NAME test-cpp14     COMMAND ${MAIN_BASE}-cpp14.t )
    endif()
    if( HAS_CPP17_FLAG )
        add_test( NAME test-cpp17     COMMAND ${MAIN_BASE}-cpp17.t )
    endif()
    if( HAS_CPPLATEST_FLAG )
        add_test( NAME test-cpplatest COMMAND ${MAIN_BASE}-cpplatest.t )
    endif()
else()
    add_test(     NAME test           COMMAND ${MAIN_BASE}.t --pass )
    add_test(     NAME list_version   COMMAND ${MAIN_BASE}.t --version )
    add_test(     NAME list_tags      COMMAND ${MAIN_BASE}.t --list-tags )
    add_test(     NAME list_tests     COMMAND ${MAIN_BASE}.t --list-tests )
endif()

# end of file
