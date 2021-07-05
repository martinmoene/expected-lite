// Copyright (c) 2016-2020 Martin Moene
//
// https://github.com/martinmoene/expected-lite
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "nonstd/expected.hpp"
#include <iostream>

template< typename T >
void use( T const & /*x*/) {}

#define expected_PRESENT( x ) \
    std::cout << #x << ": " << x << "\n"

#define expected_ABSENT( x ) \
    std::cout << #x << ": (undefined)\n"

void report()
{
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

#ifdef _CPPRTTI
    expected_PRESENT( _CPPRTTI  );
#else
    expected_ABSENT(  _CPPRTTI  );
#endif
}

int violate_access()
{
    nonstd::expected<int, char> eu( nonstd:: make_unexpected('a') );

    return eu.value();
}

int main()
{
    report();

#if ! nsel_CONFIG_NO_EXCEPTIONS_SEH
    return violate_access();
#else
    __try
    {
        return violate_access();
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        std::cerr << "\n*** Executing SEH __except block ***\n";
    }
#endif
}

// end of file
