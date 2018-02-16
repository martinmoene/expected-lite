// Copyright (c) 2016 Martin Moene
//
// https://github.com/martinmoene/expected-lite
//
// This code is licensed under the MIT License (MIT).

#pragma once

#ifndef TEST_EXPECTED_LITE_H_INCLUDED
#define TEST_EXPECTED_LITE_H_INCLUDED

// Limit C++ Core Guidelines checking to expected-lite:

#include "expected.hpp"

#if defined(_MSC_VER) && _MSC_VER >= 1910
# include <CppCoreCheck/Warnings.h>
# pragma warning(disable: ALL_CPPCORECHECK_WARNINGS)
#endif

#include "lest.hpp"

using namespace nonstd;

#define CASE( name ) lest_CASE( specification(), name )

extern lest::tests & specification();

namespace nonstd { namespace expected_lite {

// use oparator<< instead of to_string() overload;
// see  http://stackoverflow.com/a/10651752/437272

template< typename T, typename E >
inline std::ostream & operator<<( std::ostream & os, expected<T,E> const & v )
{
    using lest::to_string;
    return os << "[expected:" << (v ? to_string(*v) : "[empty]") << "]";
}

}}

namespace lest {

using ::nonstd::expected_lite::operator<<;

} // namespace lest

#endif // TEST_EXPECTED_LITE_H_INCLUDED

// end of file
