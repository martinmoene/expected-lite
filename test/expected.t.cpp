// This version targets C++11 and later.
//
// Copyright (c) 2016-2018 Martin Moene.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// expected lite is based on:
//   A proposal to add a utility class to represent expected monad
//   by Vicente J. Botet Escriba and Pierre Talbot, http:://wg21.link/p0323

#include "expected-main.t.hpp"

#ifndef nsel_CONFIG_CONFIRMS_COMPILATION_ERRORS
#define nsel_CONFIG_CONFIRMS_COMPILATION_ERRORS  0
#endif

// Suppress:
// - unused parameter, for cases without assertions such as [.std...]
#if defined(__clang__)
# pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined __GNUC__
# pragma GCC   diagnostic ignored "-Wunused-parameter"
#endif

//namespace {

using namespace nonstd;

struct Implicit { int x;          Implicit(int v) : x(v) {} };
struct Explicit { int x; explicit Explicit(int v) : x(v) {} };

struct MoveOnly
{
    int x;
    explicit MoveOnly(int x) :x{x} {}

    MoveOnly( MoveOnly const &  ) = delete;
    MoveOnly( MoveOnly && other ) noexcept : x{ other.x } {}

    MoveOnly& operator=( MoveOnly const &  ) = delete;
    MoveOnly& operator=( MoveOnly && other ) noexcept
    {
        if (&other == this)
            return *this;
        x = other.x;
        return *this;
    }
};

struct NonMovableNonCopyable
{
    NonMovableNonCopyable() = default;

    NonMovableNonCopyable( NonMovableNonCopyable const & ) = delete;
    NonMovableNonCopyable( NonMovableNonCopyable &&      ) = delete;
    NonMovableNonCopyable& operator=( NonMovableNonCopyable const & ) = delete;
    NonMovableNonCopyable& operator=( NonMovableNonCopyable &&      ) = delete;
};

bool operator==( Implicit a, Implicit b ) { return a.x == b.x; }
bool operator==( Explicit a, Explicit b ) { return a.x == b.x; }

template< typename R >
bool operator==( MoveOnly const & a, R const & r ) { return a.x == r; }

std::ostream & operator<<( std::ostream & os, Implicit i ) { return os << "Implicit:" << i.x; }
std::ostream & operator<<( std::ostream & os, Explicit e ) { return os << "Explicit:" << e.x; }
std::ostream & operator<<( std::ostream & os, MoveOnly const & m ) { return os << "MoveOnly:" << m.x; }

struct InitList
{
    std::vector<int> vec;
    char c;

    InitList( std::initializer_list<int> il, char k  ) noexcept
    : vec( il ), c( k ) {}

//    InitList( InitList const & ) noexcept = default;
//    InitList( InitList && ) noexcept = default;
//
//    InitList & operator=( InitList const & ) noexcept = default;
//    InitList & operator=( InitList && ) noexcept = default;
};

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
    OracleVal(int i_ = 0) : s(sValueConstructed), i(i_) {}

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

std::ostream & operator<<( std::ostream & os, OracleVal const & o )
{
    using lest::to_string;
    return os << "[oracle:" << to_string( o.i ) << "]";
}

//} // anonymous namespace

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

using namespace nonstd;

//
// test specification:
//

CASE( "A C++11 union can contain non-POD types" "[.]" )
{
    struct nonpod { nonpod(){} };

    union U
    {
        char c;
        nonpod np;
    };
}

// -----------------------------------------------------------------------
// storage_t<>

CASE( "[storage_t]" "[.implement]" )
{
}

// -----------------------------------------------------------------------
// unexpected_type, unexpected_type<std::exception_ptr>

CASE( "unexpected_type: Disallows default construction" )
{
#if nsel_CONFIG_CONFIRMS_COMPILATION_ERRORS
    unexpected_type<int> u;
#endif
}

CASE( "unexpected_type: Allows to copy-construct from unexpected_type, default" )
{
    unexpected_type<int> a{ 7 };

    unexpected_type<int> b( a );

    EXPECT( a.value() == 7 );
    EXPECT( b.value() == 7 );
}

CASE( "unexpected_type: Allows to move-construct from unexpected_type, default" )
{
    unexpected_type<int> a{ 7 };

    unexpected_type<int> b( std::move( a ) );

    EXPECT( a.value() == 7 );
    EXPECT( b.value() == 7 );
}

CASE( "unexpected_type: Allows to in-place-construct" )
{
    unexpected_type<Explicit> ue( in_place, 5 );
    unexpected_type<Implicit> ui( in_place, 7 );

    EXPECT( ue.value() == Explicit{5} );
    EXPECT( ui.value() == Implicit{7} );
}

CASE( "unexpected_type: Allows to in-place-construct from initializer_list" )
{
    unexpected_type<InitList> u( in_place, { 7, 8, 9 }, 'a' );

    EXPECT( u.value().vec[0]  ==  7 );
    EXPECT( u.value().vec[1]  ==  8 );
    EXPECT( u.value().vec[2]  ==  9 );
    EXPECT( u.value().c       == 'a');
}

CASE( "unexpected_type: Allows to copy-construct from error_type" )
{
    unexpected_type<int> u{ 7 };

    EXPECT( u.value() == 7 );
}

CASE( "unexpected_type: Allows to move-construct from error_type" )
{
    unexpected_type<int> u{ std::move( 7 ) };

    EXPECT( u.value() == 7 );
}

CASE( "unexpected_type: Allows to copy-construct from unexpected_type, explicit converting" )
{
    unexpected_type<int> a{ 7 };

    unexpected_type<Explicit> b{ a };

    EXPECT( b.value() == Explicit{7} );
}

CASE( "unexpected_type: Allows to copy-construct from unexpected_type, non-explicit converting" )
{
    unexpected_type<int> a{ 7 };

    unexpected_type<Implicit> b( a );

    EXPECT( b.value() == Implicit{7} );
}

CASE( "unexpected_type: Allows to move-construct from unexpected_type, explicit converting" )
{
    unexpected_type<int> a{ 7 };

    unexpected_type<Explicit> b{ std::move( a ) };

    EXPECT( b.value() == Explicit{7} );
}

CASE( "unexpected_type: Allows to move-construct from unexpected_type, non-explicit converting" )
{
    unexpected_type<int> a{ 7 };

    unexpected_type<Implicit> b( std::move( a ) );

    EXPECT( b.value() == Implicit{7} );
}

CASE( "unexpected_type: Allows to copy-assign from unexpected_type, default" )
{
    unexpected_type<int> a{ 7   };
    unexpected_type<int> b{ 0 };

    b = a;

    EXPECT( b.value() == 7 );
}

CASE( "unexpected_type: Allows to move-assign from unexpected_type, default" )
{
    unexpected_type<int> a{ 7 };
    unexpected_type<int> b{ 0 };

    b = std::move( a );

    EXPECT( b.value() == 7 );
}

CASE( "unexpected_type: Allows to copy-assign from unexpected_type, converting" )
{
    unexpected_type<int> u{ 7 };
    unexpected_type<Explicit> ue{ 0 };
    unexpected_type<Implicit> ui{ 0 };

    ue = u;
    ui = u;

    EXPECT( ue.value() == Explicit{7} );
    EXPECT( ui.value() == Implicit{7} );
}

CASE( "unexpected_type: Allows to move-assign from unexpected, converting" )
{
    unexpected_type<int> u{ 7 };
    unexpected_type<int> v{ 7 };
    unexpected_type<Explicit> ue{ 0 };
    unexpected_type<Implicit> ui{ 0 };

    ue = std::move( u );
    ui = std::move( v );

    EXPECT( ue.value() == Explicit{7} );
    EXPECT( ui.value() == Implicit{7} );
}

CASE( "unexpected_type: Allows to observe its value via a l-value reference" )
{
    unexpected_type<int>  u{ 7 };
    unexpected_type<int> uc{ 7 };

    EXPECT(  u.value() == 7 );
    EXPECT( uc.value() == 7 );
}

CASE( "unexpected_type: Allows to observe its value via a r-value reference" )
{
    unexpected_type<int>  u{ 7 };
    unexpected_type<int> uc{ 7 };

    EXPECT( std::move( u).value() == 7 );
    EXPECT( std::move(uc).value() == 7 );
}

CASE( "unexpected_type: Allows to modify its value via a l-value reference" )
{
    unexpected_type<int> u{ 5 };

    u.value() = 7;

    EXPECT( u.value() == 7 );
}

//CASE( "unexpected_type: Allows to modify its value via a r-value reference" )
//{
//    const auto v = 9;
//    unexpected_type<int> u{ 7 };
//
//    std::move( u.value() ) = v;
//
//    EXPECT( u.value() == v );
//}

CASE( "unexpected_type: Allows to be swapped" )
{
    unexpected_type<int> a{ 5 };
    unexpected_type<int> b{ 7 };

    a.swap( b );

    EXPECT( a.value() == 7 );
    EXPECT( b.value() == 5 );
}

//CASE( "unexpected_type: Allows reset via = {}" )
//{
//    unexpected_type<int> u( 3 );
//
//    u = {};
//}

// -----------------------------------------------------------------------
// unexpected_type<std::exception_ptr>

// TODO: unexpected_type: Should expected be specialized for particular E types such as exception_ptr and how?
//       See p0323r7 2.1. Ergonomics, http://wg21.link/p0323

namespace {

std::exception_ptr make_ep()
{
    try
    {
        // this generates an std::out_of_range:
        (void) std::string().at(1);
    }
    catch(...)
    {
        return std::current_exception();
    }
    return nullptr;
}
} // anonymous namespace

CASE( "unexpected_type<std::exception_ptr>: Disallows default construction" )
{
#if nsel_CONFIG_CONFIRMS_COMPILATION_ERRORS
    unexpected_type<std::exception_ptr> u;
#endif
}

CASE( "unexpected_type<std::exception_ptr>: Allows to copy-construct from error_type" )
{
    auto ep = make_ep();

    unexpected_type<std::exception_ptr> u{ ep };

    EXPECT( u.value() == ep );
}

CASE( "unexpected_type<std::exception_ptr>: Allows to move-construct from error_type" )
{
    auto ep_move = make_ep();
    const auto ep_copy = ep_move;

    unexpected_type<std::exception_ptr> u{ std::move( ep_move ) };

    EXPECT( u.value() == ep_copy );
}

CASE( "unexpected_type<std::exception_ptr>: Allows to copy-construct from an exception" )
{
    std::string text = "hello, world";

    unexpected_type<std::exception_ptr> u{ std::make_exception_ptr( std::logic_error( text.c_str() ) ) };

    try
    {
         std::rethrow_exception( u.value() );
    }
    catch( std::exception const & e )
    {
        EXPECT( e.what() == text );
    }
}

CASE( "unexpected_type<std::exception_ptr>: Allows to observe its value" )
{
    const auto ep = make_ep();
    unexpected_type<std::exception_ptr> u{ ep };

    EXPECT( u.value() == ep );
}

CASE( "unexpected_type<std::exception_ptr>: Allows to modify its value" )
{
    const auto ep1 = make_ep();
    const auto ep2 = make_ep();
    unexpected_type<std::exception_ptr> u{ ep1 };

    u.value() = ep2;

    EXPECT( u.value() == ep2 );
}

//CASE( "unexpected_type: Allows reset via = {}, std::exception_ptr specialization" )
//{
//    unexpected_type<int> u( 3 );
//
//    u = {};
//}

// -----------------------------------------------------------------------
// unexpected_type relational operators

CASE( "unexpected_type: Provides relational operators" )
{
    SETUP( "" ) {
        unexpected_type<int> u1( 6 );
        unexpected_type<int> u2( 7 );

    // compare engaged expected with engaged expected

    SECTION( "==" ) { EXPECT( u1 == u1 ); }
    SECTION( "!=" ) { EXPECT( u1 != u2 ); }
#if nsel_P0323R <= 2
    SECTION( "< " ) { EXPECT( u1 <  u2 ); }
    SECTION( "> " ) { EXPECT( u2 >  u1 ); }
    SECTION( "<=" ) { EXPECT( u1 <= u1 ); }
    SECTION( "<=" ) { EXPECT( u1 <= u2 ); }
    SECTION( ">=" ) { EXPECT( u1 >= u1 ); }
    SECTION( ">=" ) { EXPECT( u2 >= u1 ); }
#endif
    }
}

CASE( "unexpected_type: Provides relational operators, std::exception_ptr specialization" )
{
    SETUP( "" ) {
        unexpected_type<std::exception_ptr>  u( make_ep() );
        unexpected_type<std::exception_ptr> u2( make_ep() );

    // compare engaged expected with engaged expected

    SECTION( "==" ) { EXPECT    ( u == u ); }
    SECTION( "!=" ) { EXPECT    ( u != u2); }
#if nsel_P0323R <= 2
    SECTION( "< " ) { EXPECT_NOT( u <  u ); }
    SECTION( "> " ) { EXPECT_NOT( u >  u ); }
    SECTION( "< " ) { EXPECT_NOT( u <  u2); }
    SECTION( "> " ) { EXPECT_NOT( u >  u2); }
    SECTION( "<=" ) { EXPECT    ( u <= u ); }
    SECTION( ">=" ) { EXPECT    ( u >= u ); }
#endif
    }
}

// unexpected: traits

CASE( "is_unexpected<X>: Is true for unexpected_type" "[.deprecated]" )
{
#if nsel_P0323R <= 3
    EXPECT( is_unexpected<unexpected_type<std::exception_ptr>>::value );
#else
    EXPECT( !!"is_unexpected<> is not available (nsel_P0323R > 3)" );
#endif
}

CASE( "is_unexpected<X>: Is false for non-unexpected_type (int)" "[.deprecated]" )
{
#if nsel_P0323R <= 3
    EXPECT_NOT( is_unexpected<int>::value );
#else
    EXPECT( !!"is_unexpected<> is not available (nsel_P0323R > 3)" );
#endif
}

// unexpected: factory

CASE( "make_unexpected(): Allows to create an unexpected_type<E> from an E" )
{
    const auto error = 7;
    auto u = make_unexpected( error );

    EXPECT( u.value() == error );
}

CASE( "make_unexpected_from_current_exception(): Allows to create an unexpected_type<std::exception_ptr> from the current exception" "[.deprecated]" )
{
#if nsel_P0323R <= 2
    std::string text = "hello, world";

    try
    {
         throw std::logic_error( text.c_str() );
    }
    catch( std::exception const & )
    {
        auto u = make_unexpected_from_current_exception() ;

        try
        {
            std::rethrow_exception( u.value() );
        }
        catch( std::exception const & e )
        {
            EXPECT( e.what() == text );
        }
    }
#else
    EXPECT( !!"make_unexpected_from_current_exception() is not available (nsel_P0323R > 2)" );
#endif
}

CASE( "unexpected: C++17 and later provide unexpected_type as unexpected" )
{
#if nsel_CPP17_OR_GREATER || nsel_COMPILER_MSVC_VERSION > 141
    nonstd::unexpected<int> u{7};
#else
    EXPECT( !!"unexpected is not available (no C++17)." );
#endif
}

// -----------------------------------------------------------------------
// x.x.6 bad_expected_access<>

CASE( "bad_expected_access: Disallows default construction" )
{
#if nsel_CONFIG_CONFIRMS_COMPILATION_ERRORS
    bad_expected_access<int> bad;
#endif
}

CASE( "bad_expected_access: Allows construction from error_type" )
{
    bad_expected_access<int> bea( 123 );
}

CASE( "bad_expected_access: Allows to observe its error" )
{
    const int error = 7;
    bad_expected_access<int> bea( error );
    bad_expected_access<int> const beac( error );

    EXPECT(             bea.error()   == error );
    EXPECT(            beac.error()   == error );
    EXPECT( std::move(  bea.error() ) == error );
    EXPECT( std::move( beac.error() ) == error );
}

CASE( "bad_expected_access: Allows to change its error" )
{
    const int old_error = 0;
    const int new_error = 7;
    bad_expected_access<int> bea( old_error );

    bea.error() = new_error;

    EXPECT( bea.error() == new_error  );
}

CASE( "bad_expected_access: Provides non-empty what()" )
{
    bad_expected_access<int> bea( 123 );

    EXPECT( ! std::string( bea.what() ).empty() );
}

// -----------------------------------------------------------------------
// expected<>

// x.x.4.1 expected<> constructors (value)

CASE( "expected: Allows to default construct" )
{
    expected<int, char> e;

    EXPECT( e.has_value() );
}

CASE( "expected: Allows to default construct from noncopyable, noncopyable value type" )
{
    expected<NonMovableNonCopyable, NonMovableNonCopyable> e;

    EXPECT( e.has_value() );
}

CASE( "expected: Allows to default construct from noncopyable, noncopyable error type" )
{
    expected<NonMovableNonCopyable, NonMovableNonCopyable> e{ unexpect_t{} };

    EXPECT( !e.has_value() );
}

CASE( "expected: Allows to copy-construct from expected: value" )
{
    expected<int, char> a = 7;

    expected<int, char> b{ a };

    EXPECT( b              );
    EXPECT( b.value() == 7 );
}

CASE( "expected: Allows to copy-construct from expected: error" )
{
    expected<char, int> a{ unexpect, 7 };

    expected<char, int> b{ a };

    EXPECT( !b              );
    EXPECT(  b.error() == 7 );
}

CASE( "expected: Allows to move-construct from expected: value" )
{
    expected<int, char> a = 7;

    expected<int, char> b{ std::move( a ) };

    EXPECT( b              );
    EXPECT( b.value() == 7 );
    EXPECT( a              );   // postcondition: unchanged!
}

CASE( "expected: Allows to move-construct from expected: error" )
{
    expected<char, int> a{ unexpect, 7 };

    expected<char, int> b{ std::move( a ) };

    EXPECT( !b              );
    EXPECT(  b.error() == 7 );
}

CASE( "expected: Allows to copy-construct from expected; value, explicit converting" )
{
    expected<int, char> a = 7;

    expected<Explicit, char> b{ a };

    EXPECT( b                        );
    EXPECT( b.value() == Explicit{7} );
}

CASE( "expected: Allows to copy-construct from expected; error, explicit converting" )
{
    expected<char, int> a{ unexpect, 7 };

    expected<char, Explicit> b{ a };

    EXPECT( !b                        );
    EXPECT(  b.error() == Explicit{7} );
}

CASE( "expected: Allows to copy-construct from expected; value, non-explicit converting" )
{
    expected<int, char> a = 7;

    expected<Implicit, char> b( a );

    EXPECT( b                        );
    EXPECT( b.value() == Implicit{7} );
}

CASE( "expected: Allows to copy-construct from expected; error, non-explicit converting" )
{
    expected<char, int> a{ unexpect, 7 };

    expected<char, Implicit> b( a );

    EXPECT( !b                        );
    EXPECT(  b.error() == Implicit{7} );
}

CASE( "expected: Allows to move-construct from expected; value, explicit converting" )
{
    expected<int, char> a = 7;

    expected<Explicit, char> b{ std::move( a ) };

    EXPECT( b                        );
    EXPECT( b.value() == Explicit{7} );
}

CASE( "expected: Allows to move-construct from expected; error, explicit converting" )
{
    expected<char, int> a{ unexpect, 7 };

    expected<char, Explicit> b{ std::move( a ) };

    EXPECT( !b                        );
    EXPECT(  b.error() == Explicit{7} );
}

CASE( "expected: Allows to move-construct from expected; value, non-explicit converting" )
{
    expected<int, char> a = 7;

    expected<Implicit, char> b( std::move( a ) );

    EXPECT( b                        );
    EXPECT( b.value() == Implicit{7} );
}

CASE( "expected: Allows to move-construct from expected; error, non-explicit converting" )
{
    expected<char, int> a{ unexpect, 7 };

    expected<char, Implicit> b( std::move( a ) );

    EXPECT( !b                        );
    EXPECT(  b.error() == Implicit{7} );
}

CASE( "expected: Allows to forward-construct from value, explicit converting" )
{
    auto v = 7;

    expected<Explicit, char> b{ std::move( v ) };

    EXPECT( b                        );
    EXPECT( b.value() == Explicit{7} );
}

CASE( "expected: Allows to forward-construct from value, non-explicit converting" )
{
    auto v = 7;

    expected<Implicit, char> b{ std::move( v ) };

    EXPECT( b                        );
    EXPECT( b.value() == Implicit{7} );
}

CASE( "expected: Allows to in-place-construct value" )
{
    auto v = 7;

    expected<Implicit, char> b{ in_place, v };

    EXPECT( b                        );
    EXPECT( b.value() == Implicit{7} );
}

CASE( "expected: Allows to in-place-construct value from initializer_list" )
{
    expected<InitList, char> b{ in_place, { 7, 8, 9 }, 'a' };

    EXPECT( b                );
    EXPECT( b->vec[0]  ==  7 );
    EXPECT( b->vec[1]  ==  8 );
    EXPECT( b->vec[2]  ==  9 );
    EXPECT( b->c       == 'a');
}

// x.x.4.1 expected<> constructors (error)

CASE( "expected: Allows to copy-construct from unexpected, explicit converting" )
{
    unexpected_type<int> u{ 7 };

    expected<int, Explicit> e{ u };

    EXPECT( e.error() == Explicit{7} );
}

CASE( "expected: Allows to copy-construct from unexpected, non-explicit converting" )
{
    unexpected_type<int> u{ 7 };

    expected<int, Implicit> e{ u };

    EXPECT( e.error() == Implicit{7} );
}

CASE( "expected: Allows to move-construct from unexpected, explicit converting" )
{
    unexpected_type<int> u{ 7 };

    expected<int, Explicit> e{ std::move( u ) };

    EXPECT( e.error() == Explicit{7} );
}

CASE( "expected: Allows to move-construct from unexpected, non-explicit converting" )
{
    unexpected_type<int> u{ 7 };

    expected<int, Implicit> e{ std::move( u ) };

    EXPECT( e.error() == Implicit{7} );
}

CASE( "expected: Allows to in-place-construct error" )
{
    expected<int, Explicit> e{ unexpect, 7 };
    expected<int, Implicit> i{ unexpect, 7 };

    EXPECT( e.error() == Explicit{7} );
    EXPECT( i.error() == Implicit{7} );
}

CASE( "expected: Allows to in-place-construct error from initializer_list" )
{
    expected<char, InitList> e{ unexpect, { 7, 8, 9 }, 'a' };

    EXPECT( !e                       );
    EXPECT(  e.error().vec[0]  ==  7 );
    EXPECT(  e.error().vec[1]  ==  8 );
    EXPECT(  e.error().vec[2]  ==  9 );
    EXPECT(  e.error().c       == 'a');
}

// x.x.4.3 expected<> assignment

CASE( "expected: Allows to copy-assign from expected, value" )
{
    expected<int, char> a{ 7 };
    expected<int, char> b;

    b = a;

    EXPECT( b              );
    EXPECT( b.value() == 7 );
}

CASE( "expected: Allows to copy-assign from expected, error" )
{
    expected<char, int> a{ unexpect, 7 };
    expected<char, int> b;

    b = a;

    EXPECT( !b              );
    EXPECT(  b.error() == 7 );
}

CASE( "expected: Allows to move-assign from expected, value" )
{
    expected<int, char> a{ 7 };
    expected<int, char> b;

    b = std::move( a );

    EXPECT( b              );
    EXPECT( b.value() == 7 );
}

CASE( "expected: Allows to move-assign from expected, error" )
{
    expected<char, int> a{ unexpect, 7 };
    expected<char, int> b;

    b = std::move( a );

    EXPECT( !b              );
    EXPECT(  b.error() == 7 );
}

CASE( "expected: Allows to forward-assign from value" )
{
    expected<int, char> a;
    expected<int, char> b;

    a = '7';
    b =  7;

    EXPECT( a                );
    EXPECT( b                );
    EXPECT( a.value() == '7' );
    EXPECT( b.value() ==  7  );
}

CASE( "expected: Allows to copy-assign from unexpected" )
{
    expected<char, int>  e;
    unexpected_type<int> u{ 7 } ;

    e = u;

    EXPECT( e.error() == 7 );
}

CASE( "expected: Allows to move-assign from unexpected" )
{
    expected<char, int>  e;
    unexpected_type<int> u{ 7 } ;

    e = std::move( u );

    EXPECT( e.error() == 7 );
}

CASE( "expected: Allows to move-assign from move-only unexpected" )
{
    expected<char, MoveOnly> e;
    unexpected_type<MoveOnly> u{7};

    e = std::move( u );

    EXPECT( e.error() == 7 );
}

CASE( "expected: Allows to emplace value" )
{
    expected<int, char> a;
    expected<int, char> b;

    auto va = a.emplace( '7' );
    auto vb = b.emplace(  7  );

    EXPECT( va == '7' );
    EXPECT( vb ==  7  );
    EXPECT( a.value() == '7' );
    EXPECT( b.value() ==  7  );
}

CASE( "expected: Allows to emplace value from initializer_list" )
{
    expected<InitList, char> e{ {}, 'x'};

    auto ve = e.emplace( { 7, 8, 9 }, 'a' );

    EXPECT( e                );
    EXPECT( ve.vec[0]  ==  7 );
    EXPECT( ve.vec[1]  ==  8 );
    EXPECT( ve.vec[2]  ==  9 );
    EXPECT( ve.c       == 'a');
    EXPECT( e.value().vec[0]  ==  7 );
    EXPECT( e.value().vec[1]  ==  8 );
    EXPECT( e.value().vec[2]  ==  9 );
    EXPECT( e.value().c       == 'a');
}

// x.x.4.4 expected<> swap

CASE( "expected: Allows to be swapped" )
{
    using std::swap;

    SETUP("") {

    expected<int, char> e1{ 1 };
    expected<int, char> e2{ 2 };
    expected<int, char> u1{ unexpect, '1' };
    expected<int, char> u2{ unexpect, '2' };

    SECTION("value-value, member swap")
    {
        e1.swap( e2 );

        EXPECT( e1.value() == 2 );
        EXPECT( e2.value() == 1 );
    }
    SECTION("error-error, member swap")
    {
        u1.swap( u2 );

        EXPECT( u1.error() == '2' );
        EXPECT( u2.error() == '1' );
    }
    SECTION("value-error, member swap")
    {
        e1.swap( u1 );

        EXPECT( e1.error() == '1' );
        EXPECT( u1.value() ==  1  );
    }
    SECTION("error-value, member swap")
    {
        u1.swap( e1 );

        EXPECT( u1.value() ==  1  );
        EXPECT( e1.error() == '1' );
    }
    }
}

// x.x.4.5 expected<> observers

CASE( "expected: Allows to observe its value via a pointer" )
{
    const auto v = 7;
    expected<Implicit, char> e{ v };

    EXPECT( e->x == v );
}

CASE( "expected: Allows to observe its value via a pointer to constant" )
{
    const auto v = 7;
    const expected<Implicit, char> e{ v };

    EXPECT( e->x == v );
}

CASE( "expected: Allows to modify its value via a pointer" )
{
    const auto v1 = 7;
    const auto v2 = 42;
    expected<Implicit, char> e{ v1 };

    e->x = v2;

    EXPECT( e->x == v2 );
}

CASE( "expected: Allows to observe its value via a l-value reference" )
{
    const auto v = 7;
    expected<Implicit, char>        e{ v };
    expected<Implicit, char> const ec{ v };

    EXPECT(  *e == Implicit{v} );
    EXPECT( *ec == Implicit{v} );
}

CASE( "expected: Allows to observe its value via a r-value reference" )
{
    const auto v = 7;
    expected<Implicit, char>        e{ v };
    expected<Implicit, char> const ec{ v };

    EXPECT( *std::move( e) == Implicit{v} );
    EXPECT( *std::move(ec) == Implicit{v} );
}

CASE( "expected: Allows to modify its value via a l-value reference" )
{
    const auto v1 = 7;
    const auto v2 = 42;
    expected<Implicit, char> e{ v1 };

    *e = Implicit{v2};

    EXPECT( *e == Implicit{v2} );
}

CASE( "expected: Allows to modify its value via a r-value reference" )
{
    const auto v1 = 7;
    const auto v2 = 42;
    expected<Implicit, char> e{ v1 };

#if !nsel_COMPILER_GNUC_VERSION || nsel_COMPILER_GNUC_VERSION >= 490
    *std::move(e) = Implicit{v2};
#else
    *e = Implicit{v2};  // non-r-value
#endif

    EXPECT( *e == Implicit{v2} );
}

CASE( "expected: Allows to observe if it contains a value (or error)" )
{
    expected<int, int> e;
    expected<int, int> u{ unexpect, 3 };

    EXPECT(  e );
    EXPECT( !u );
}

CASE( "expected: Allows to observe its value" )
{
    const auto v = 7;
    expected<Implicit, char>        e{ v };
    expected<Implicit, char> const ec{ v };

    EXPECT(  e.value() == Implicit{v} );
    EXPECT( ec.value() == Implicit{v} );
}

CASE( "expected: Allows to modify its value" )
{
    const auto v1 = 7;
    const auto v2 = 42;
    expected<Implicit, char> e{ v1 };

    e.value() = Implicit{v2};

    EXPECT( e.value() == Implicit{v2} );
}

CASE( "expected: Allows to move its value" )
{
    const auto v = 7;
    expected<Implicit, char>        m{ v };
    expected<Implicit, char> const mc{ v };

    expected<Implicit, char>  e{ std::move(m ).value() };
    expected<Implicit, char> ec{ std::move(mc).value() };

    EXPECT(  e.value() == Implicit{v} );
    EXPECT( ec.value() == Implicit{v} );
}

CASE( "expected: Allows to observe its error" )
{
    const auto v = 7;
    expected<char, Implicit>        e{ unexpect, v };
    expected<char, Implicit> const ec{ unexpect, v };

    EXPECT(  e.error() == Implicit{v} );
    EXPECT( ec.error() == Implicit{v} );
}

CASE( "expected: Allows to modify its error" )
{
    const auto v1 = 7;
    const auto v2 = 42;
    expected<char, Implicit> e{ unexpect, v1 };

    e.error() = Implicit{v2};

    EXPECT( e.error() == Implicit{v2} );
}

CASE( "expected: Allows to move its error" )
{
    const auto v = 7;
    expected<char, Implicit>        m{ unexpect, v };
    expected<char, Implicit> const mc{ unexpect, v };

    expected<char, Implicit>  e{ unexpect, std::move(m ).error() };
    expected<char, Implicit> ec{ unexpect, std::move(mc).error() };

    EXPECT(  e.error() == Implicit{v} );
    EXPECT( ec.error() == Implicit{v} );
}

CASE( "expected: Allows to observe its error as unexpected" )
{
    const auto v = 7;
    expected<char, Implicit> e{ unexpect, v };

    EXPECT( e.get_unexpected().value() == v );
}

CASE( "expected: Allows to query if it contains an exception of a specific base type" )
{
    expected<int, std::out_of_range> e{ unexpect, std::out_of_range( "oor" ) };

    EXPECT(  e.has_exception< std::logic_error   >() );
    EXPECT( !e.has_exception< std::runtime_error >() );
}

CASE( "expected: Allows to observe its value if available, or obtain a specified value otherwise" )
{
    const auto ve = 3;
    const auto vu = 7;
    expected<int, int> e{ ve };
    expected<int, int> u{ unexpect, 0 };

    EXPECT( e.value_or( vu ) == ve );
    EXPECT( u.value_or( vu ) == vu );
}

CASE( "expected: Allows to move its value if available, or obtain a specified value otherwise" )
{
    const auto ve = 3;
    const auto vu = 7;
    expected<int, int> mv{ ve };
    expected<int, int> mu{ unexpect, 0 };

    EXPECT( std::move( mv ).value_or( vu ) == ve );
    EXPECT( std::move( mu ).value_or( vu ) == vu );
}

CASE( "expected: Throws bad_expected_access on value access when disengaged" )
{
    expected<char, int >  e{ unexpect, 7 };
    expected<char, int > ec{ unexpect, 7 };

    EXPECT_THROWS(     e.value() );
    EXPECT_THROWS_AS(  e.value(), bad_expected_access<int> );
    EXPECT_THROWS(    ec.value() );
    EXPECT_THROWS_AS( ec.value(), bad_expected_access<int> );

    EXPECT_THROWS(    std::move( e).value() );
    EXPECT_THROWS_AS( std::move( e).value(), bad_expected_access<int> );
    EXPECT_THROWS(    std::move(ec).value() );
    EXPECT_THROWS_AS( std::move(ec).value(), bad_expected_access<int> );
}

CASE( "expected: Allows to map value with and_then" )
{
    const auto mul2 = []( int n ) -> expected<int, int> { return n * 2; };
    const auto to_unexpect42 = []( int ) -> expected<int, int> { return make_unexpected( 42 ); };

    {
        expected<int, int> e{ 11 };
        const expected<int, int> ce{ 21 };
        expected<int, int> ue{ unexpect, 42 };
        EXPECT( e.and_then( mul2 ).value() == 22 );
        EXPECT( ce.and_then( mul2 ).value() == 42 );
        EXPECT( !ue.and_then( mul2 ).has_value());
        EXPECT( ue.and_then( mul2 ).error() == 42 );
        EXPECT( !e.and_then( to_unexpect42 ).has_value());
        EXPECT( e.and_then( to_unexpect42 ).error() == 42 );
        EXPECT( ce.and_then( to_unexpect42 ).error() == 42 );
    }

    const auto moveonly_x_mul2 = [](MoveOnly val) -> expected<int, int> { return val.x * 2; };
    EXPECT( (expected<MoveOnly, int>{ MoveOnly{ 33 } }).and_then( moveonly_x_mul2 ).value() == 66 );
    EXPECT( (expected<MoveOnly, int>{ MoveOnly{ 15 } }).and_then( [](MoveOnly&&) -> expected<MoveOnly, int> { return make_unexpected( 42 ); } ).error() == 42 );

    const auto map_to_void = [](int) -> expected<void, int> { return {}; };
    const auto map_to_void_unexpect42 = [](int) -> expected<void, int> { return make_unexpected( 42 ); };
    static_assert( std::is_same< expected<void, int>, decltype( expected<int, int>( 3 ).and_then( map_to_void ) ) >::value,
        "and_then mapping to void results in expected<void>");
    EXPECT( (expected<int, int>(3)).and_then( map_to_void ).has_value() );
    EXPECT( !(expected<int, int>(3)).and_then( map_to_void_unexpect42 ).has_value() );
    EXPECT( (expected<int, int>(3)).and_then( map_to_void_unexpect42 ).error() == 42 );
}

CASE( "expected: Handling unexpected with or_else" )
{
    const auto to_unexpect43 = []( int ) -> expected<int, int> { return make_unexpected( 43 ); };

    {
        expected<int, int> e{ 11 };
        const expected<int, int> ce{ 21 };
        expected<int, int> ue{ unexpect, 42 };
        EXPECT( e.or_else( to_unexpect43 ).has_value());
        EXPECT( e.or_else( to_unexpect43 ).value() == 11 );
        EXPECT( ce.or_else( to_unexpect43 ).value() == 21 );
        EXPECT( !ue.or_else( to_unexpect43 ).has_value());
        EXPECT( ue.or_else( to_unexpect43 ).error() == 43 );
    }

    const auto fallback_throw = []( int ) -> expected<int, int> { throw std::runtime_error( "or_else" ); };
    EXPECT_THROWS_AS( (expected<int, int>{ unexpect, 42 }).or_else( fallback_throw ), std::runtime_error );

    const auto moveonly_fallback_to_66 = [](int) -> expected<MoveOnly, int> { return MoveOnly{ 66 }; };
    EXPECT( (expected<MoveOnly, int>{ MoveOnly{ 33 } }).or_else( moveonly_fallback_to_66 ).value() == 33 );
    EXPECT( (expected<MoveOnly, int>{ unexpect, 15 }).or_else(moveonly_fallback_to_66).value() == 66 );
}

CASE( "expected: transform values" )
{
    const auto mul2 = []( int n ) -> int { return n * 2; };

    {
        expected<int, int> e{ 11 };
        const expected<int, int> ce{ 21 };
        expected<int, int> ue{ unexpect, 42 };
        EXPECT( e.transform( mul2 ).value() == 22 );
        EXPECT( ce.transform( mul2 ).value() == 42 );
        EXPECT( !ue.transform( mul2 ).has_value());
        EXPECT( ue.transform( mul2 ).error() == 42 );
    }

    const auto moveonly_map_to_x = &MoveOnly::x;
    const auto moveonly_x_mul2 = [](MoveOnly val) -> int { return val.x * 2; };
    EXPECT( (expected<MoveOnly, int>{ MoveOnly{ 33 } }).transform( moveonly_map_to_x ).value() == 33 );
    EXPECT( (expected<MoveOnly, int>{ MoveOnly{ 33 } }).transform( moveonly_map_to_x ).transform( mul2 ).value() == 66 );
    EXPECT( (expected<MoveOnly, int>{ MoveOnly{ 33 } }).transform( moveonly_x_mul2 ).has_value() );
    EXPECT( (expected<MoveOnly, int>{ MoveOnly{ 33 } }).transform( moveonly_x_mul2 ).value() == 66 );
    EXPECT( !(expected<MoveOnly, int>{ unexpect, 15 }).transform( [](MoveOnly&&) -> int { return 42; } ).has_value() );
    EXPECT( (expected<MoveOnly, int>{ unexpect, 15 }).transform( [](MoveOnly&&) -> int { return 42; } ).error() == 15 );

    const auto map_to_void = [](int) -> void { };
    static_assert( std::is_same< expected<void, int>, decltype( expected<int, int>( 3 ).transform( map_to_void ) ) >::value,
        "transform to void results in expected<void>" );
    EXPECT( (expected<int, int>(3)).transform( map_to_void ).has_value() );
    static_assert( std::is_same< decltype( (expected<int, int>(3)).transform( map_to_void ).value() ), void >::value,
        "transform to void results in void value" );
}

CASE( "expected: Mapping errors with transform_error" )
{
    const auto to_43 = []( int ) -> int { return 43; };

    {
        expected<int, int> e{ 11 };
        const expected<int, int> ce{ 21 };
        expected<int, int> ue{ unexpect, 42 };
        EXPECT( e.transform_error( to_43 ).has_value());
        EXPECT( e.transform_error( to_43 ).value() == 11 );
        EXPECT( ce.transform_error( to_43 ).value() == 21 );
        EXPECT( !ue.transform_error( to_43 ).has_value());
        EXPECT( ue.transform_error( to_43 ).error() == 43 );
    }
}

// -----------------------------------------------------------------------
// expected<void> specialization

// x.x.4.1 expected<void> constructors

CASE( "expected<void>: Allows to default-construct" )
{
    expected<void, char> e;

    EXPECT( e.has_value() );
}

CASE( "expected<void>: Allows to copy-construct from expected<void>: value" )
{
    expected<void, int> a;

    expected<void, int> b{ a };

    EXPECT( b );
}

CASE( "expected<void>: Allows to copy-construct from expected<void>: error" )
{
    expected<void, int > a{ unexpect, 7 };

    expected<void, int> b{ a };

    EXPECT( !b              );
    EXPECT(  b.error() == 7 );
}

CASE( "expected<void>: Allows to move-construct from expected<void>: value" )
{
    expected<void, int> a;

    expected<void, int> b{ std::move( a ) };

    EXPECT( b );
    EXPECT( a );    // postcondition: unchanged!
}

CASE( "expected<void>: Allows to move-construct from expected<void>: error" )
{
    expected<void, int > a{ unexpect, 7 };

    expected<void, int> b{ std::move( a ) };

    EXPECT( !b              );
    EXPECT(  b.error() == 7 );
}

CASE( "expected<void>: Allows to in-place-construct" )
{
    expected<void, int > e{ in_place };

    EXPECT( e );
}

CASE( "expected<void>: Allows to copy-construct from unexpected, explicit converting" )
{
    unexpected_type<int> u{ 7 };

    expected<void, Explicit> e{ u };

    EXPECT( e.error() == Explicit{7} );
}

CASE( "expected<void>: Allows to copy-construct from unexpected, non-explicit converting" )
{
    unexpected_type<int> u{ 7 };

    expected<void, Implicit> e{ u };

    EXPECT( e.error() == Implicit{7} );
}

CASE( "expected<void>: Allows to move-construct from unexpected, explicit converting" )
{
    unexpected_type<int> u{ 7 };

    expected<void, Explicit> e{ std::move( u ) };

    EXPECT( e.error() == Explicit{7} );
}

CASE( "expected<void>: Allows to move-construct from unexpected, non-explicit converting" )
{
    unexpected_type<int> u{ 7 };

    expected<void, Implicit> e{ std::move( u ) };

    EXPECT( e.error() == Implicit{7} );
}

CASE( "expected<void>: Allows to in-place-construct unexpected_type" )
{
    expected<void, Explicit> e{ unexpect, 7 };
    expected<void, Implicit> i{ unexpect, 7 };

    EXPECT( e.error() == Explicit{7} );
    EXPECT( i.error() == Implicit{7} );
}

CASE( "expected<void>: Allows to in-place-construct error from initializer_list" )
{
    expected<void, InitList> e{ unexpect, { 7, 8, 9 }, 'a' };

    EXPECT( !e                       );
    EXPECT(  e.error().vec[0]  ==  7 );
    EXPECT(  e.error().vec[1]  ==  8 );
    EXPECT(  e.error().vec[2]  ==  9 );
    EXPECT(  e.error().c       == 'a');
}

// x.x.4.3 expected<void> assignment

CASE( "expected<void>: Allows to copy-assign from expected, value" )
{
    expected<void, int> a;
    expected<void, int> b;

    b = a;

    EXPECT( b );
}

CASE( "expected<void>: Allows to copy-assign from expected, error" )
{
    expected<void, int> a{ unexpect, 7 };
    expected<void, int> b;

    b = a;

    EXPECT( !b              );
    EXPECT(  b.error() == 7 );
}

CASE( "expected<void>: Allows to move-assign from expected, value" )
{
    expected<void, int> a;
    expected<void, int> b;

    b = std::move( a );

    EXPECT( b );
}

CASE( "expected<void>: Allows to move-assign from expected, error" )
{
    expected<void, int> a{ unexpect, 7 };
    expected<void, int> b;

    b = std::move( a );

    EXPECT( !b              );
    EXPECT(  b.error() == 7 );
}

CASE( "expected<void>: Allows to emplace value" )
{
    expected<void, int> a{ unexpect, 7 };

    a.emplace();

    EXPECT( a );
}

// x.x.4.4 expected<void> swap

CASE( "expected<void>: Allows to be swapped" )
{
    using std::swap;

    SETUP("") {

    expected<void, char> e1;
    expected<void, char> e2;
    expected<void, char> u1{ unexpect, '1' };
    expected<void, char> u2{ unexpect, '2' };

    SECTION("value-value, member swap")
    {
        e1.swap( e2 );

        EXPECT( e1 );
        EXPECT( e2 );
    }
    SECTION("error-error, member swap")
    {
        u1.swap( u2 );

        EXPECT( u1.error() == '2' );
        EXPECT( u2.error() == '1' );
    }
    SECTION("value-error, member swap")
    {
        e1.swap( u1 );

        EXPECT( e1.error() == '1' );
        EXPECT( u1                );
    }
    SECTION("error-value, member swap")
    {
        u1.swap( e1 );

        EXPECT( u1                );
        EXPECT( e1.error() == '1' );
    }
    }
}

// x.x.4.5 expected<void> observers

CASE( "expected<void>: Allows to observe if it contains a value (or error)" )
{
    expected<void, int> e;
    expected<void, int> u{ unexpect, 3 };

    EXPECT(  e );
    EXPECT( !u );
}

CASE( "expected<void>: Allows to observe its value" )
{
    expected<void, int>        e;
    expected<void, int> const ec;

    EXPECT( (  e.value(), true ) );
    EXPECT( ( ec.value(), true ) );
}

CASE( "expected<void>: Allows to observe its error" )
{
    const auto v = 7;
    expected<void, Implicit>        e{ unexpect, v };
    expected<void, Implicit> const ec{ unexpect, v };

    EXPECT(  e.error() == Implicit{v} );
    EXPECT( ec.error() == Implicit{v} );
}

CASE( "expected<void>: Allows to modify its error" )
{
    const auto v1 = 7;
    const auto v2 = 42;
    expected<void, Implicit> e{ unexpect, v1 };

    e.error() = Implicit{v2};

    EXPECT( e.error() == Implicit{v2} );
}

CASE( "expected<void>: Allows to move its error" )
{
    const auto v = 7;
    expected<void, Implicit>        m{ unexpect, v };
    expected<void, Implicit> const mc{ unexpect, v };

    expected<void, Implicit>  e{ unexpect, std::move(m ).error() };
    expected<void, Implicit> ec{ unexpect, std::move(mc).error() };

    EXPECT(  e.error() == Implicit{v} );
    EXPECT( ec.error() == Implicit{v} );
}

CASE( "expected<void>: Allows to observe its error as unexpected" )
{
    const auto value = 7;
    expected<void, int> e{ unexpect, value };

    EXPECT( e.get_unexpected().value() == value );
}

CASE( "expected<void>: Allows to query if it contains an exception of a specific base type" )
{
    expected<void, std::out_of_range> e{ unexpect, std::out_of_range( "oor" ) };

    EXPECT(  e.has_exception< std::logic_error   >() );
    EXPECT( !e.has_exception< std::runtime_error >() );
}

CASE( "expected<void>: Throws bad_expected_access on value access when disengaged" )
{
    expected<void, int >  e{ unexpect, 7 };
    expected<void, int > ec{ unexpect, 7 };

    EXPECT_THROWS(     e.value() );
    EXPECT_THROWS_AS(  e.value(), bad_expected_access<int> );
    EXPECT_THROWS(    ec.value() );
    EXPECT_THROWS_AS( ec.value(), bad_expected_access<int> );

    EXPECT_THROWS(    std::move( e).value() );
    EXPECT_THROWS_AS( std::move( e).value(), bad_expected_access<int> );
    EXPECT_THROWS(    std::move(ec).value() );
    EXPECT_THROWS_AS( std::move(ec).value(), bad_expected_access<int> );
}

CASE( "expected<void>: calling argless functions with and_then" )
{
    const auto ret22 = []() -> expected<int, int> { return 22; };
    const auto unexpect32 = []() -> expected<int, int> { return make_unexpected( 32 ); };

    {
        expected<void, int> e;
        const expected<void, int> ce;
        expected<void, int> ue{ unexpect, 42 };
        EXPECT( e.has_value() );
        EXPECT( ce.has_value() );
        EXPECT( e.and_then( ret22 ).value() == 22 );
        EXPECT( ce.and_then( ret22 ).value() == 22 );
        EXPECT( !ue.and_then( ret22 ).has_value());
        EXPECT( ue.and_then( ret22 ).error() == 42 );
        EXPECT( !e.and_then( unexpect32 ).has_value());
        EXPECT( e.and_then( unexpect32 ).error() == 32 );
        EXPECT( ce.and_then( unexpect32 ).error() == 32 );
    }

    {
        bool called = false;
        expected<void, int> e;
        e.and_then( [&called]() -> expected<void, int> {
            called = true;
            return {};
        } );
        EXPECT( called );
    }

    {
        bool called = false;
        expected<void, int>{}.and_then( [&called]() -> expected<void, int> {
            called = true;
            return {};
        } );
        EXPECT( called );
    }

    {
        bool called = false;
        expected<void, int>{ unexpect, 42 }.and_then( [&called]() -> expected<void, int> {
            called = true;
            return {};
        } );
        EXPECT( !called );
    }

    const bool map_to_unexpect_success = !expected<void, int>{}.and_then( []() -> expected<void, int> { return make_unexpected( 42 ); } ).has_value();
    EXPECT( map_to_unexpect_success );
}

CASE( "expected<void>: or_else unexpected handling works" )
{
    const auto make_valid = [](int) -> expected<void, int> { return {}; };
    const auto unexpect32 = [](int) -> expected<void, int> { return make_unexpected( 32 ); };

    {
        expected<void, int> e;
        const expected<void, int> ce;
        expected<void, int> ue{ unexpect, 42 };
        EXPECT( e.has_value() );
        EXPECT( ce.has_value() );
        EXPECT( e.or_else( unexpect32 ).has_value());
        static_assert( std::is_same< decltype( e.or_else( unexpect32 ).value() ), void >::value,
            "or_else mapping to void results in void value" );
        EXPECT( ce.or_else( unexpect32 ).has_value());
        EXPECT( !ue.or_else( unexpect32 ).has_value());
        EXPECT( ue.or_else( make_valid ).has_value() );
        static_assert( std::is_same< decltype( ue.or_else( make_valid ).value() ), void >::value,
            "or_else mapping to void results in void value" );
        EXPECT( ue.or_else( unexpect32 ).error() == 32 );
    }
}

CASE( "expected<void>: transform_error maps unexpected values" )
{
    const auto mul2 = []( int v ) -> int { return v * 2; };
    enum class my_error { einval };
    const auto map_to_my_error = [](int) { return my_error::einval; };

    {
        expected<void, int> e;
        const expected<void, int> ce;
        expected<void, int> ue{ unexpect, 42 };
        EXPECT( e.transform_error( mul2 ).has_value());
        EXPECT( ce.transform_error( mul2 ).has_value());
        EXPECT( !ue.transform_error( mul2 ).has_value());
        EXPECT( ue.transform_error( mul2 ).error() == 84 );
        EXPECT( ue.transform_error( map_to_my_error ).error() == my_error::einval );
    }
}

// [expected<> unwrap()]

// [expected<> factories]

// x.x.4.7 expected<>: relational operators

CASE( "operators: Provides expected relational operators" )
{
    SETUP( "" ) {
        auto v1 = 6;
        auto v2 = 7;
        expected<int, char> e1( v1 );
        expected<int, char> e2( v2 );

        unexpected_type<char> u( 'u' );
        expected<int, char> d( u );

    // compare engaged expected with engaged expected

    SECTION( "engaged    == engaged"    ) { EXPECT( e1 == e1 ); }
    SECTION( "engaged    != engaged"    ) { EXPECT( e1 != e2 ); }
#if nsel_P0323R <= 2
    SECTION( "engaged    <  engaged"    ) { EXPECT( e1 <  e2 ); }
    SECTION( "engaged    >  engaged"    ) { EXPECT( e2 >  e1 ); }
    SECTION( "engaged    <= engaged"    ) { EXPECT( e1 <= e1 ); }
    SECTION( "engaged    <= engaged"    ) { EXPECT( e1 <= e2 ); }
    SECTION( "engaged    >= engaged"    ) { EXPECT( e1 >= e1 ); }
    SECTION( "engaged    >= engaged"    ) { EXPECT( e2 >= e1 ); }
#endif

    // compare engaged expected with value

    SECTION( "engaged    == value"      ) { EXPECT( e1 == v1 ); }
    SECTION( "value      == engaged"    ) { EXPECT( v1 == e1 ); }
    SECTION( "engaged    != value"      ) { EXPECT( e1 != v2 ); }
    SECTION( "value      != engaged"    ) { EXPECT( v1 != e2 ); }
#if nsel_P0323R <= 2
    SECTION( "engaged    <  value"      ) { EXPECT( e1 <  v2 ); }
    SECTION( "value      <  engaged"    ) { EXPECT( v1 <  e2 ); }
    SECTION( "engaged    >  value"      ) { EXPECT( e2 >  v1 ); }
    SECTION( "value      >  engaged"    ) { EXPECT( v2 >  e1 ); }
    SECTION( "engaged    <= value"      ) { EXPECT( e1 <= v2 ); }
    SECTION( "value      <= engaged"    ) { EXPECT( v1 <= e2 ); }
    SECTION( "engaged    >= value"      ) { EXPECT( e2 >= v1 ); }
    SECTION( "value      >= engaged"    ) { EXPECT( v2 >= e1 ); }
#endif

    // compare engaged expected with disengaged expected

    SECTION( "engaged    == disengaged" ) { EXPECT_NOT( e1 == d  ); }
    SECTION( "disengaged == engaged"    ) { EXPECT_NOT( d  == e1 ); }
    SECTION( "engaged    != disengaged" ) { EXPECT    ( e1 != d  ); }
    SECTION( "disengaged != engaged"    ) { EXPECT    ( d  != e2 ); }
#if nsel_P0323R <= 2
    SECTION( "engaged    <  disengaged" ) { EXPECT_NOT( e1 <  d  ); }
    SECTION( "disengaged <  engaged"    ) { EXPECT    ( d  <  e2 ); }
    SECTION( "engaged    >  disengaged" ) { EXPECT    ( e2 >  d  ); }
    SECTION( "disengaged >  engaged"    ) { EXPECT_NOT( d  >  e1 ); }
    SECTION( "engaged    <= disengaged" ) { EXPECT_NOT( e1 <= d  ); }
    SECTION( "disengaged <= engaged"    ) { EXPECT    ( d  <= e2 ); }
    SECTION( "engaged    >= disengaged" ) { EXPECT    ( e2 >= d  ); }
    SECTION( "disengaged >= engaged"    ) { EXPECT_NOT( d  >= e1 ); }
#endif

    // compare engaged expected with unexpected

    SECTION( "disengaged == unexpected" ) { EXPECT    ( d  == u  ); }
    SECTION( "unexpected == disengaged" ) { EXPECT    ( u  == d  ); }
    SECTION( "engaged    != unexpected" ) { EXPECT    ( e1 != u  ); }
    SECTION( "unexpected != engaged"    ) { EXPECT    ( u  != e1 ); }
#if nsel_P0323R <= 2
    SECTION( "disengaged <  unexpected" ) { EXPECT_NOT( d  <  u  ); }
    SECTION( "unexpected <  disengaged" ) { EXPECT_NOT( u  <  d  ); }
    SECTION( "disengaged <= unexpected" ) { EXPECT    ( d  <= u  ); }
    SECTION( "unexpected <= disengaged" ) { EXPECT    ( u  <= d  ); }
    SECTION( "disengaged >  unexpected" ) { EXPECT_NOT( d  >  u  ); }
    SECTION( "unexpected >  disengaged" ) { EXPECT_NOT( u  >  d  ); }
    SECTION( "disengaged >= unexpected" ) { EXPECT    ( d  >= u  ); }
    SECTION( "unexpected >= disengaged" ) { EXPECT    ( u  >= d  ); }
#endif

    }
}

CASE( "operators: Provides expected relational operators (void)" )
{
    SETUP( "" ) {
        expected<void, char> ev1;
        expected<void, char> ev2;
        expected<void, char> evu{ unexpect };

    // compare engaged expected with engaged expected

    SECTION( "  engaged == engaged"     ) { EXPECT(     ev1 == ev2 ); }
    SECTION( "!(engaged != engaged)"    ) { EXPECT_NOT( ev1 != ev2 ); }

    // compare engaged expected with disengaged expected

    SECTION( "  engaged != disengaged"  ) { EXPECT(     ev1 != evu ); }
    SECTION( "!(engaged == disengaged)" ) { EXPECT_NOT( ev1 == evu ); }
    }
}

// -----------------------------------------------------------------------
// expected: specialized algorithms

// -----------------------------------------------------------------------
// Other

CASE( "swap: Allows expected to be swapped" )
{
    using std::swap;

    SETUP("") {

    expected<int, char> e1{ 1 };
    expected<int, char> e2{ 2 };
    expected<int, char> u1{ unexpect, '1' };
    expected<int, char> u2{ unexpect, '2' };

    SECTION("value-value, std::swap")
    {
        swap( e1, e2 );

        EXPECT( e1.value() == 2 );
        EXPECT( e2.value() == 1 );
    }
    SECTION("error-error, std::swap")
    {
        swap( u1, u2 );

        EXPECT( u1.error() == '2' );
        EXPECT( u2.error() == '1' );
    }
    SECTION("value-error, std::swap")
    {
        swap( e1, u1 );

        EXPECT( e1.error() == '1' );
        EXPECT( u1.value() ==  1  );
    }
    SECTION("error-value, std::swap")
    {
        swap( u1, e1 );

        EXPECT( u1.value() ==  1  );
        EXPECT( e1.error() == '1' );
    }
    }
}

CASE( "std::hash: Allows to compute hash value for expected" )
{
    expected<int, char> a{ 7 };
    expected<int, char> b{ 7 };

    EXPECT( (std::hash< expected<int, char> >{}( a )) == (std::hash< expected<int, char> >{}( b )) );
}

#if nsel_P0323R <= 3

#include <memory>

void vfoo() {}

expected<int> foo()
{
    return make_expected( 7 );
}

expected<std::unique_ptr<int>> bar()
{
    return make_expected( std::unique_ptr<int>( new int(7) ) );
}

#endif // nsel_P0323R

CASE( "make_expected(): create expected from given value" "[.deprecated]" )
{
#if nsel_P0323R <= 3
    auto e = make_expected( 7 );

    EXPECT(  e      );
    EXPECT( *e == 7 );
#else
    EXPECT( !!"make_expected() is not available (nsel_P0323R > 3)" );
#endif
}

CASE( "make_expected(): create expected<void>" "[.deprecated]" )
{
#if nsel_P0323R <= 3
    auto e = make_expected();

    EXPECT( e );
#else
    EXPECT( !!"make_expected() is not available (nsel_P0323R > 3)" );
#endif
}

CASE( "make_expected_from_current_exception(): create expected from current exception" "[.deprecated]" )
{
#if nsel_P0323R <= 3
    EXPECT( !!"Implement" );
#else
    EXPECT( !!"make_expected() is not available (nsel_P0323R > 3)" );
#endif
}

CASE( "make_expected_from_exception(): create expected from given exception" "[.deprecated]" )
{
#if nsel_P0323R <= 3
    EXPECT( !!"Implement" );
#else
    EXPECT( !!"make_expected() is not available (nsel_P0323R > 3)" );
#endif
}

CASE( "make_expected_from_call(): non-void return type" "[.deprecated]" )
{
#if nsel_P0323R <= 3
    expected<int> ei = foo();
    expected<std::unique_ptr<int>> eup = bar();

    auto e2   = make_expected_from_call( foo );
    auto eup2 = make_expected_from_call( bar );

    EXPECT( e2   );
    EXPECT( eup2 );
#else
    EXPECT( !!"make_expected_from_call() is not available (nsel_P0323R > 3)" );
#endif
}

CASE( "make_expected_from_call(): void return type" "[.deprecated]" )
{
#if nsel_P0323R <= 3
    auto ev = make_expected_from_call( vfoo );

    EXPECT( ev );
#else
    EXPECT( !!"make_expected_from_call() is not available (nsel_P0323R > 3)" );
#endif
}

CASE( "tweak header: reads tweak header if supported " "[tweak]" )
{
#if expected_HAVE_TWEAK_HEADER
    EXPECT( EXPECTED_TWEAK_VALUE == 42 );
#else
    EXPECT( !!"Tweak header is not available (expected_HAVE_TWEAK_HEADER: 0)." );
#endif
}

// -----------------------------------------------------------------------
// expected: issues

// issue #15, https://github.com/martinmoene/expected-dark/issues/15

CASE( "issue-15d" )
{
    nonstd::expected< int, std::error_code > e = 12;
    (void)e.value();
}

// issue #15, https://github.com/martinmoene/expected-lite/issues/15

CASE( "issue-15" )
{
    (void) nonstd::expected< int, int >( 12).value();
}

// issue #29, https://github.com/martinmoene/expected-lite/issues/29
// issue #32, https://github.com/martinmoene/expected-lite/issues/32

namespace issue_32 {

enum class Error
{
    Bad
};

class MyNonMoveableObject
{
public:
    MyNonMoveableObject() = default;
    MyNonMoveableObject( MyNonMoveableObject const &  ) = default;
    MyNonMoveableObject( MyNonMoveableObject       && ) = delete;
    MyNonMoveableObject& operator=( MyNonMoveableObject const  &) = default;
    MyNonMoveableObject& operator=( MyNonMoveableObject       &&) = delete;

    ~MyNonMoveableObject() = default;
};

nonstd::expected< MyNonMoveableObject, Error > create_copyable()
{
    return MyNonMoveableObject{};
}

class MyNonCopyableObject
{
public:
    MyNonCopyableObject() = default;
    MyNonCopyableObject( MyNonCopyableObject const &  ) = delete;
    MyNonCopyableObject( MyNonCopyableObject       && ) = default;
    MyNonCopyableObject& operator=( MyNonCopyableObject const &  ) = delete;
    MyNonCopyableObject& operator=( MyNonCopyableObject       && ) = default;

    ~MyNonCopyableObject() = default;
};

nonstd::expected< MyNonCopyableObject, Error > create_moveable()
{
    return MyNonCopyableObject{};
}

} // namespace issue_32

CASE( "pr-41" )
{
    expected<int, int> a{7};
    const expected<int, int> ca{7};
    expected<int, int> b{unexpect, 7};
    const expected<int, int> cb{unexpect, 7};
    expected<void, int> c{unexpect, 7};
    const expected<void, int> cc{unexpect, 7};
    EXPECT( *std::move(a) == 7 );
    EXPECT( *std::move(ca) == 7 );
    EXPECT( std::move(b).error() == 7 );
    EXPECT( std::move(cb).error() == 7 );
    EXPECT( std::move(c).error() == 7 );
    EXPECT( std::move(cc).error() == 7 );
}

// issue #50, https://github.com/martinmoene/expected-lite/issues/50

namespace issue_50 {

struct MyConstMemberNonMoveableObject
{
    const int x;
    MyConstMemberNonMoveableObject( int x_ ) : x( x_ ) {}
    MyConstMemberNonMoveableObject( MyConstMemberNonMoveableObject const & ) = default;
};

nonstd::unexpected_type<MyConstMemberNonMoveableObject> create_nonmoveable()
{
    return nonstd::make_unexpected<MyConstMemberNonMoveableObject>( MyConstMemberNonMoveableObject(3) );
}

} // namespace issue_50

namespace issue_51 {

int compare_equal_with_expected_void()
{
    auto ev1 = nonstd::expected<void, int>{};
    auto ev2 = nonstd::expected<void, int>{};
    return ev1 == ev2;
}

int compare_not_equal_with_expected_void()
{
    auto ev1 = nonstd::expected<void, int>{};
    auto ev2 = nonstd::expected<void, int>{};
    return ev1 != ev2;
}

} // namespace issue_51

namespace issue_59 {

struct NonMovableNonCopyable
{
    NonMovableNonCopyable() = default;

    NonMovableNonCopyable( NonMovableNonCopyable const &  ) = delete;
    NonMovableNonCopyable( NonMovableNonCopyable       && ) = delete;
    NonMovableNonCopyable& operator=( NonMovableNonCopyable const &  ) = delete;
    NonMovableNonCopyable& operator=( NonMovableNonCopyable       && ) = delete;
};
} // namespace issue_59

CASE( "issue-58" )
{
    static_assert( !std::is_copy_constructible<issue_59::NonMovableNonCopyable>::value, "is not copy constructible" );
    static_assert( !std::is_move_constructible<issue_59::NonMovableNonCopyable>::value, "is not move constructible" );

    nonstd::expected<issue_59::NonMovableNonCopyable, issue_59::NonMovableNonCopyable> expected;
    nonstd::expected<issue_59::NonMovableNonCopyable, issue_59::NonMovableNonCopyable> unexpected( nonstd::unexpect_t{} );

    EXPECT(  expected.has_value()   );
    EXPECT( !unexpected.has_value() );
}

CASE( "invoke" )
{
    struct A {
      int x;
      constexpr int get() const { return x; }
      constexpr int get2(char) const { return x; }
    };
    static_assert( nonstd::expected_lite::detail::invoke( &A::x, A{21} ) == 21, "" );
    EXPECT( nonstd::expected_lite::detail::invoke( &MoveOnly::x, MoveOnly(42) ) == 42 );
    constexpr A lval{ 7 };
    static_assert( nonstd::expected_lite::detail::invoke( &A::x, lval ) == 7, "" );
    A mut_lval{ 12 };
    std::reference_wrapper<A> ref{ mut_lval };
    const std::reference_wrapper<const A> cref{ lval };
    EXPECT( nonstd::expected_lite::detail::invoke( &A::x, ref ) == 12 );
    EXPECT( nonstd::expected_lite::detail::invoke( &A::x, cref ) == 7 );
    static_assert( nonstd::expected_lite::detail::invoke(&A::x, &lval) == 7, "" );
    static_assert( nonstd::expected_lite::detail::invoke(&A::get, &lval) == 7, "" );
    static_assert( nonstd::expected_lite::detail::invoke(&A::get, A{77}) == 77, "" );
    EXPECT( nonstd::expected_lite::detail::invoke(&A::get, ref) == 12 );
    EXPECT( nonstd::expected_lite::detail::invoke(&A::get, cref) == 7 );
    static_assert( nonstd::expected_lite::detail::invoke(&A::get2, &lval, 'a') == 7, "" );
    static_assert( nonstd::expected_lite::detail::invoke(&A::get2, A{77}, 'a') == 77, "" );
    EXPECT( nonstd::expected_lite::detail::invoke(&A::get2, ref, 'a') == 12 );
    EXPECT( nonstd::expected_lite::detail::invoke(&A::get2, cref, 'a') == 7 );
}

// -----------------------------------------------------------------------
//  using as optional

#if 1

/// disengaged optional state tag

struct nullopt_t{};

const nullopt_t nullopt{};

/// optional expressed in expected

template< typename T >
using optional = expected<T, nullopt_t>;

#endif
