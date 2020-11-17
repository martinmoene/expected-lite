// Copyright (c) 2016-2018 Martin Moene
//
// https://github.com/martinmoene/expected-lite
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "expected-main.t.hpp"

#define expected_PRESENT( x ) \
    std::cout << #x << ": " << x << "\n"

#define expected_ABSENT( x ) \
    std::cout << #x << ": (undefined)\n"

// Suppress:
// - unused parameter, for cases without assertions such as [.std...]
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined __GNUC__
# pragma GCC   diagnostic ignored "-Wunused-parameter"
#endif

lest::tests & specification()
{
    static lest::tests tests;
    return tests;
}

CASE( "expected-lite version" "[.expected][.version]" )
{
    expected_PRESENT( expected_lite_MAJOR );
    expected_PRESENT( expected_lite_MINOR );
    expected_PRESENT( expected_lite_PATCH );
    expected_PRESENT( expected_lite_VERSION );
}

CASE( "any configuration" "[.expected][.config]" )
{
    expected_PRESENT( nsel_HAVE_STD_EXPECTED );
    expected_PRESENT( nsel_USES_STD_EXPECTED );
    expected_PRESENT( nsel_EXPECTED_DEFAULT );
    expected_PRESENT( nsel_EXPECTED_NONSTD );
    expected_PRESENT( nsel_EXPECTED_STD );
    expected_PRESENT( nsel_CONFIG_SELECT_EXPECTED );
    expected_PRESENT( nsel_CONFIG_NO_EXCEPTIONS );
    expected_PRESENT( nsel_CPLUSPLUS );
}

CASE( "__cplusplus" "[.stdc++]" )
{
    expected_PRESENT( __cplusplus );

#ifdef _MSVC_LANG
    expected_PRESENT( _MSVC_LANG );
#else
    expected_ABSENT(  _MSVC_LANG );
#endif
}

CASE( "Compiler version" "[.compiler]" )
{
#if nsel_USES_STD_EXPECTED
    std::cout << "(Compiler version not available: using std::expected)\n";
#else
    expected_PRESENT( nsel_COMPILER_CLANG_VERSION );
    expected_PRESENT( nsel_COMPILER_GNUC_VERSION );
    expected_PRESENT( nsel_COMPILER_MSVC_VERSION );
#endif
}

CASE( "presence of C++ language features" "[.stdlanguage]" )
{
#if nsel_USES_STD_EXPECTED
    std::cout << "(Presence of C++ language features not available: using std::expected)\n";
#else
    std::cout << "[.stdlanguage]: none\n";
#endif
}

CASE( "presence of C++ library features" "[.stdlibrary]" )
{
#if nsel_USES_STD_EXPECTED
    std::cout << "(Presence of C++ library features not available: using std::expected)\n";
#else

#ifdef __cpp_exceptions
    expected_PRESENT( __cpp_exceptions );
#else
    expected_ABSENT(  __cpp_exceptions );
#endif

#ifdef __EXCEPTIONS
    expected_PRESENT( __EXCEPTIONS );
#else
    expected_ABSENT(  __EXCEPTIONS );
#endif

#ifdef _HAS_EXCEPTIONS
    expected_PRESENT( _HAS_EXCEPTIONS );
#else
    expected_ABSENT(  _HAS_EXCEPTIONS );
#endif

#ifdef _CPPUNWIND
    expected_PRESENT( _CPPUNWIND );
#else
    expected_ABSENT(  _CPPUNWIND );
#endif

#endif
}

int main( int argc, char * argv[] )
{
    return lest::run( specification(), argc, argv );
}

#if 0
g++            -I../include -o expected-lite.t.exe expected-lite.t.cpp && expected-lite.t.exe --pass
g++ -std=c++98 -I../include -o expected-lite.t.exe expected-lite.t.cpp && expected-lite.t.exe --pass
g++ -std=c++03 -I../include -o expected-lite.t.exe expected-lite.t.cpp && expected-lite.t.exe --pass
g++ -std=c++0x -I../include -o expected-lite.t.exe expected-lite.t.cpp && expected-lite.t.exe --pass
g++ -std=c++11 -I../include -o expected-lite.t.exe expected-lite.t.cpp && expected-lite.t.exe --pass
g++ -std=c++14 -I../include -o expected-lite.t.exe expected-lite.t.cpp && expected-lite.t.exe --pass
g++ -std=c++17 -I../include -o expected-lite.t.exe expected-lite.t.cpp && expected-lite.t.exe --pass

cl -EHsc -I../include expected-lite.t.cpp && expected-lite.t.exe --pass
#endif

// end of file
