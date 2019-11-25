// Copyright (c) 2016-2019 Martin Moene
//
// https://github.com/martinmoene/expected-lite
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "nonstd/expected.hpp"

template< typename T >
void use( T const & /*x*/) {}

int main()
{
    nonstd::expected<int, char> e;
    nonstd::unexpected_type<int> ut(7);

    use(e);
    use(ut);

#if nsel_CPP17_OR_GREATER && nsel_COMPILER_MSVC_VERSION > 141
    nonstd::unexpected<int> u(3); u;
    use(u);
#endif

    nonstd::expected<int, char> eu( nonstd:: make_unexpected('a') );

    return eu.value();
}

// end of file
