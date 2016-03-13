// Copyright (C) 2016 Martin Moene.
//
// This version targets C++11 and later.
//
// This code is licensed under the MIT License (MIT).
//
// expected lite is based on:
//   A proposal to add a utility class to represent expected monad - Revision 2
//   by Vicente J. Botet Escriba and Pierre Talbot.

#include "expected.hpp"

#include "lest.hpp"

#define CASE( name ) lest_CASE( specification, name )

static lest::tests specification;

using nonstd::expected;
using nonstd::unexpected_type;
using nonstd::bad_expected_access;
using nonstd::make_expected;
using nonstd::make_unexpected;

namespace nonstd {

    template< typename T, typename E >
    std::ostream & operator<<( std::ostream & os, expected<T,E> const & e )
    {
        using lest::to_string;
        return os << ( e ? to_string( *e ) : "[error:" + to_string( e.error() ) + "]" );
    }

    template< typename E >
    std::ostream & operator<<( std::ostream & os, unexpected_type<E> const & u )
    {
        using lest::to_string;
        return os << "[unexp:" << to_string( u.value() ) << "]";
    }
}

enum State
{
    sDefaultConstructed,
    sValueCopyConstructed,
    sValueMoveConstructed,
    sCopyConstructed,
    sMoveConstructed,
    sMoveAssigned,
    sCopyAssigned,
    sValueCopyAssigned,
    sValueMoveAssigned,
    sMovedFrom,
    sValueConstructed
};

struct OracleVal
{
    State s;
    int i;
    OracleVal(int i = 0) : s(sValueConstructed), i(i) {}

    bool operator==( OracleVal const & other ) const { return s==other.s && i==other.i; }
};

struct Oracle
{
    State s;
    OracleVal val;

    Oracle() : s(sDefaultConstructed) {}
    Oracle(const OracleVal& v) : s(sValueCopyConstructed), val(v) {}
    Oracle(OracleVal&& v) : s(sValueMoveConstructed), val(std::move(v)) {v.s = sMovedFrom;}
    Oracle(const Oracle& o) : s(sCopyConstructed), val(o.val) {}
    Oracle(Oracle&& o) : s(sMoveConstructed), val(std::move(o.val)) {o.s = sMovedFrom;}

    Oracle& operator=(const OracleVal& v) { s = sValueCopyConstructed; val = v; return *this; }
    Oracle& operator=(OracleVal&& v) { s = sValueMoveConstructed; val = std::move(v); v.s = sMovedFrom; return *this; }
    Oracle& operator=(const Oracle& o) { s = sCopyConstructed; val = o.val; return *this; }
    Oracle& operator=(Oracle&& o) { s = sMoveConstructed; val = std::move(o.val); o.s = sMovedFrom; return *this; }

    bool operator==( Oracle const & other ) const { return s == other.s && val == other.val;}
};

using namespace nonstd;

std::exception_ptr make_ep()
{
    try
    {
        // this generates an std::out_of_range:
        std::string().at(1); 
    }
    catch(...)
    {
        return std::current_exception();
    }
    return nullptr;
}

CASE( "A C++11 union can contain non-POD types" )
{
    struct nonpod { nonpod(){} };

    union U
    {
        char c;
        nonpod np;
    };
}

CASE( "[storage_t]" )
{
}

CASE( "Unexpected allows to copy-construct from error_type" )
{
    Oracle o;
    unexpected_type<Oracle> u{ o };

    EXPECT( u.value().s == sCopyConstructed );
}

CASE( "Unexpected allows to move-construct from error_type" )
{
    unexpected_type<Oracle> u{ Oracle() };

    EXPECT( u.value().s == sMoveConstructed );
}

CASE( "Unexpected allows to observe the error" )
{
    const int error_value = 7;
    unexpected_type<int> u{ error_value };

    EXPECT( u.value() == error_value );
}

CASE( "Unexpected relational operators" )
{
    SETUP( "" ) {
        unexpected_type<int> u1( 6 );
        unexpected_type<int> u2( 7 );

    // compare engaged expected with engaged expected

    SECTION( "==" ) { EXPECT( u1 == u1 ); }
    SECTION( "!=" ) { EXPECT( u1 != u2 ); }
    SECTION( "< " ) { EXPECT( u1 <  u2 ); }
    SECTION( "> " ) { EXPECT( u2 >  u1 ); }
    SECTION( "<=" ) { EXPECT( u1 <= u1 ); }
    SECTION( "<=" ) { EXPECT( u1 <= u2 ); }
    SECTION( ">=" ) { EXPECT( u1 >= u1 ); }
    SECTION( ">=" ) { EXPECT( u2 >= u1 ); }
    }
}

CASE( "Unexpected relational operators, std::exception_ptr specialization" )
{
    SETUP( "" ) {
        unexpected_type<> u( make_ep() );
        unexpected_type<> u2( make_ep() );

    // compare engaged expected with engaged expected

    SECTION( "==" ) { EXPECT    ( u == u ); }
    SECTION( "!=" ) { EXPECT    ( u != u2); }
    SECTION( "< " ) { EXPECT_NOT( u <  u ); }
    SECTION( "> " ) { EXPECT_NOT( u >  u ); }
    SECTION( "<=" ) { EXPECT    ( u <= u ); }
    SECTION( ">=" ) { EXPECT    ( u >= u ); }
    }
}

CASE( "Expected relational operators" )
{
    SETUP( "" ) {
        expected<int, char> e1( 6 );
        expected<int, char> e2( 7 );

        unexpected_type<char> u( 'u' );
        expected<int, char> d( u );

    // compare engaged expected with engaged expected

    SECTION( "engaged    == engaged"    ) { EXPECT( e1 == e1 ); }
    SECTION( "engaged    != engaged"    ) { EXPECT( e1 != e2 ); }
    SECTION( "engaged    <  engaged"    ) { EXPECT( e1 <  e2 ); }
    SECTION( "engaged    >  engaged"    ) { EXPECT( e2 >  e1 ); }
    SECTION( "engaged    <= engaged"    ) { EXPECT( e1 <= e1 ); }
    SECTION( "engaged    <= engaged"    ) { EXPECT( e1 <= e2 ); }
    SECTION( "engaged    >= engaged"    ) { EXPECT( e1 >= e1 ); }
    SECTION( "engaged    >= engaged"    ) { EXPECT( e2 >= e1 ); }

    // compare engaged expected with value

    SECTION( "engaged    == value"      ) { EXPECT( e1 == 6  ); }
    SECTION( "value      == engaged"    ) { EXPECT(  6 == e1 ); }
    SECTION( "engaged    != value"      ) { EXPECT( e1 != 7  ); }
    SECTION( "value      != engaged"    ) { EXPECT(  6 != e2 ); }
    SECTION( "engaged    <  value"      ) { EXPECT( e1 <  7  ); }
    SECTION( "value      <  engaged"    ) { EXPECT(  6 <  e2 ); }
    SECTION( "engaged    >  value"      ) { EXPECT( e2 >  6  ); }
    SECTION( "value      >  engaged"    ) { EXPECT(  7 >  e1 ); }
    SECTION( "engaged    <= value"      ) { EXPECT( e1 <= 7  ); }
    SECTION( "value      <= engaged"    ) { EXPECT(  6 <= e2 ); }
    SECTION( "engaged    >= value"      ) { EXPECT( e2 >= 6  ); }
    SECTION( "value      >= engaged"    ) { EXPECT(  7 >= e1 ); }

    // compare engaged expected with disengaged expected

    SECTION( "engaged    == disengaged" ) { EXPECT_NOT( e1 == d  ); }
    SECTION( "disengaged == engaged"    ) { EXPECT_NOT( d  == e1 ); }
    SECTION( "engaged    != disengaged" ) { EXPECT    ( e1 != d  ); }
    SECTION( "disengaged != engaged"    ) { EXPECT    ( d  != e2 ); }
    SECTION( "engaged    <  disengaged" ) { EXPECT_NOT( e1 <  d  ); }
    SECTION( "disengaged <  engaged"    ) { EXPECT    ( d  <  e2 ); }
    SECTION( "engaged    >  disengaged" ) { EXPECT    ( e2 >  d  ); }
    SECTION( "disengaged >  engaged"    ) { EXPECT_NOT( d  >  e1 ); }
    SECTION( "engaged    <= disengaged" ) { EXPECT_NOT( e1 <= d  ); }
    SECTION( "disengaged <= engaged"    ) { EXPECT    ( d  <= e2 ); }
    SECTION( "engaged    >= disengaged" ) { EXPECT    ( e2 >= d  ); }
    SECTION( "disengaged >= engaged"    ) { EXPECT_NOT( d  >= e1 ); }

    // compare engaged expected with unexpected

    SECTION( "disengaged == unexpected" ) { EXPECT    ( d  == u  ); }
    SECTION( "unexpected == disengaged" ) { EXPECT    ( u  == d  ); }
    SECTION( "engaged    != unexpected" ) { EXPECT    ( e1 != u  ); }
    SECTION( "unexpected != engaged"    ) { EXPECT    ( u  != e1 ); }
    SECTION( "disengaged <  unexpected" ) { EXPECT_NOT( d  <  u  ); }
    SECTION( "unexpected <  disengaged" ) { EXPECT_NOT( u  <  d  ); }
    SECTION( "disengaged <= unexpected" ) { EXPECT    ( d  <= u  ); }
    SECTION( "unexpected <= disengaged" ) { EXPECT    ( u  <= d  ); }
    SECTION( "disengaged >  unexpected" ) { EXPECT_NOT( d  >  u  ); }
    SECTION( "unexpected >  disengaged" ) { EXPECT_NOT( u  >  d  ); }
    SECTION( "disengaged >= unexpected" ) { EXPECT    ( d  >= u  ); }
    SECTION( "unexpected >= disengaged" ) { EXPECT    ( u  >= d  ); }

    }
}


#if 1

/// disengaged optional state tag

struct nullopt_t{};

const nullopt_t nullopt;

/// optional expressed in expected

template< typename T >
using optional = expected<T, nullopt_t>;

#endif



/*
 * Test driver:
 */

int main( int argc, char * argv[] )
{
    return lest::run( specification, argc, argv );
}

#if 0
cl -nologo -W3   -EHsc -Dlest_FEATURE_AUTO_REGISTER=1 -I../include/nonstd expected.t.cpp && expected.t --pass
cl -nologo -Wall -EHsc -Dlest_FEATURE_AUTO_REGISTER=1 -I../include/nonstd expected.t.cpp && expected.t --pass

g++ -Wall -Wextra -std=c++03 -Wno-unused-parameter -Dlest_FEATURE_AUTO_REGISTER=1 -I../include/nonstd -o expected.t.exe expected.t.cpp && expected.t --pass
g++ -Wall -Wextra -std=c++11 -Wno-unused-parameter -Dlest_FEATURE_AUTO_REGISTER=1 -I../include/nonstd -o expected.t.exe expected.t.cpp && expected.t --pass
g++ -Wall -Wextra -std=c++1y -Wno-unused-parameter -Dlest_FEATURE_AUTO_REGISTER=1 -I../include/nonstd -o expected.t.exe expected.t.cpp && expected.t --pass

#endif // 0
