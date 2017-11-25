// Copyright (c) 2016 Martin Moene
//
// https://github.com/martinmoene/expected-lite
//
// This code is licensed under the MIT License (MIT).

#pragma once

#ifndef TEST_EXPECTED_LITE_H_INCLUDED
#define TEST_EXPECTED_LITE_H_INCLUDED

#include "expected.hpp"
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
