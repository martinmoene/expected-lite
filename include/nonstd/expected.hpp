// Copyright (C) 2016 Martin Moene.
//
// This version targets C++11 and later.
//
// This code is licensed under the MIT License (MIT).
//
// expected lite is based on:
//   A proposal to add a utility class to represent expected monad - Revision 2
//   by Vicente J. Botet Escriba and Pierre Talbot.

#ifndef NONSTD_EXPECTED_LITE_HPP
#define NONSTD_EXPECTED_LITE_HPP

#include <cassert>
#include <exception>
#include <initializer_list>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#define  expected_lite_VERSION "0.0.0"

// Compiler detection:

#define nsel_CPP11_OR_GREATER  ( __cplusplus >= 201103L )
#define nsel_CPP14_OR_GREATER  ( __cplusplus >= 201402L )

#if nsel_CPP14_OR_GREATER
# define nsel_constexpr14 constexpr
#else
# define nsel_constexpr14 /*constexpr*/
#endif

namespace nonstd {

template< typename T, typename E >
class expected;

namespace expected_detail {

/// constructed union to hold value.

template< typename T, typename E >
union storage_t
{
    friend class expected<T,E>;

private:
    typedef T value_type;
    typedef E error_type;

    // no-op construction
    storage_t() {}
    ~storage_t() {}

    void construct_value( value_type const & v )
    {
        new( &m_value ) value_type( v );
    }

    void construct_value( value_type && v )
    {
        new( &m_value ) value_type( std::move( v ) );
    }

    void destruct_value()
    {
        m_value.~value_type();
    }

    void construct_error( error_type const & e )
    {
        new( &m_error ) error_type( e );
    }

    void construct_error( error_type && e )
    {
        new( &m_error ) error_type( std::move( e ) );
    }

    void destruct_error()
    {
        m_error.~error_type();
    }

    constexpr value_type const & value() const &
    {
        return m_value;
    }

    value_type & value() &
    {
        return m_value;
    }

    constexpr value_type && value() const &&
    {
        return std::move( m_value );
    }

    void emplace( value_type && v )
    {
        m_value( std::forward( v ) );
    }

    value_type * value_ptr() const
    {
        return &m_value;
    }

    error_type const & error() const
    {
        return m_error;
    }

    error_type & error()
    {
        return m_error;
    }

//    error_type * error_ptr() const
//    {
//        return &m_error;
//    }

private:
    value_type m_value;
    error_type m_error;
};

} // namespace expected_detail

/// in-place tag

struct in_place_t{};

/// class unexpected_type

template< typename E = std::exception_ptr >
class unexpected_type
{
public:
    typedef E error_type;

    unexpected_type() = delete;

    constexpr explicit unexpected_type( error_type const & error )
    : m_error( error )
    {}

    constexpr explicit unexpected_type( error_type && error )
    : m_error( std::move( error ) )
    {}

    constexpr error_type const & value() const
    {
        return m_error;
    }

private:
    error_type m_error;
};

/// class unexpected_type, std::exception_ptr specialization

template<>
class unexpected_type< std::exception_ptr >
{
public:
    typedef std::exception_ptr error_type;

    unexpected_type() = delete;

    ~unexpected_type(){}

    explicit unexpected_type( std::exception_ptr const & error )
    : m_error( error )
    {}

    explicit unexpected_type(std::exception_ptr && error )
    : m_error( std::move( error ) )
    {}

    template< typename E >
    explicit unexpected_type( E error )
    : m_error( std::make_exception_ptr( error ) )
    {}

    std::exception_ptr const & value() const
    {
        return m_error;
    }

private:
    error_type m_error;
};

// unexpected: relational operators

template< typename E >
constexpr bool operator==( unexpected_type<E> const & x, unexpected_type<E> const & y )
{
    return x.value() == y.value();
}

template< typename E >
constexpr bool operator!=( unexpected_type<E> const & x, unexpected_type<E> const & y )
{
    return ! ( x == y );
}

template< typename E >
constexpr bool operator<( unexpected_type<E> const & x, unexpected_type<E> const & y )
{
    return x.value() < y.value();
}

template< typename E >
constexpr bool operator>( unexpected_type<E> const & x, unexpected_type<E> const & y )
{
    return ( y < x );
}

template< typename E >
constexpr bool operator<=( unexpected_type<E> const & x, unexpected_type<E> const & y )
{
    return ! ( y < x  );
}

template< typename E >
constexpr bool operator>=( unexpected_type<E> const & x, unexpected_type<E> const & y )
{
    return ! ( x < y );
}

constexpr bool operator<( unexpected_type<std::exception_ptr> const & x, unexpected_type<std::exception_ptr> const & y )
{
    return false;
}

constexpr bool operator>( unexpected_type<std::exception_ptr> const & x, unexpected_type<std::exception_ptr> const & y )
{
    return false;
}

constexpr bool operator<=( unexpected_type<std::exception_ptr> const & x, unexpected_type<std::exception_ptr> const & y )
{
    return ( x == y );
}

constexpr bool operator>=( unexpected_type<std::exception_ptr> const & x, unexpected_type<std::exception_ptr> const & y )
{
    return ( x == y );
}

// unexpected: traits

template <typename E>
struct is_unexpected : std::false_type {};

template <typename E>
struct is_unexpected<unexpected_type<E>> : std::true_type {};

// unexpected: factory

template< typename E>
nsel_constexpr14 auto
make_unexpected( E && v) -> unexpected_type< typename std::decay<E>::type >
{
    return unexpected_type< typename std::decay<E>::type >( v );
}

/*nsel_constexpr14*/ auto
make_unexpected_from_current_exception() -> unexpected_type< std::exception_ptr >
{
    return unexpected_type< std::exception_ptr >( std::current_exception() );
}

/// expected access error

class bad_expected_access : public std::logic_error
{
public:
  explicit bad_expected_access( const char * what_arg )
  : logic_error( what_arg ) {}
};

/// class expected

template< typename T, typename E = std::exception_ptr >
class expected
{
public:
    typedef T value_type;
    typedef E error_type;

    // constructors

    // enable_if: is_default_constructible<T>
    nsel_constexpr14 expected()
    noexcept( std::is_nothrow_default_constructible<T>::value )
    : is_valid( true )
    {
        contained.construct_value( value_type() );
    }

    nsel_constexpr14 expected( value_type const & rhs )
    : is_valid( true )
    {
        contained.construct_value( rhs );
    }

    // enable_if: is_move_constructible<T>::value && is_move_constructible<E>::value
    nsel_constexpr14 expected( value_type && rhs )
    noexcept(
        std::is_nothrow_move_constructible<T>::value
        && std::is_nothrow_move_constructible<E>::value )
    : is_valid( true )
    {
        contained.construct_value( std::move( rhs ) );
    }

    template <class... Args>
    nsel_constexpr14 explicit expected( in_place_t, Args&&... args )
    : is_valid( true )
    {
        contained.construct_value( std::forward<Args>( args )... );
    }

    template< typename U, class... Args>
    nsel_constexpr14 explicit expected( in_place_t, std::initializer_list<U> il, Args&&... args )
    : is_valid( true )
    {
        contained.construct_value( il, std::forward<Args>( args )... );
    }

    nsel_constexpr14 expected( expected const & rhs )
    : is_valid( rhs.is_valid )
    {
        if ( is_valid ) contained.construct_value( rhs.contained.value() );
        else            contained.construct_error( rhs.contained.error() );
    }

    nsel_constexpr14 expected( expected && rhs )
    : is_valid( rhs.is_valid )
    {
        if ( is_valid ) contained.construct_value( std::move( rhs.contained.value() ) );
        else            contained.construct_error( std::move( rhs.contained.error() ) );
    }

    nsel_constexpr14 expected( unexpected_type<E> const & error )
    : is_valid( false )
    {
        contained.construct_error( error.value() );
    }

    template< typename U >
    nsel_constexpr14 expected( unexpected_type<U> const & error )
    : is_valid( false )
    {
        contained.construct_error( error.value() );
    }

    // destructor

    ~expected()
    {
        if ( is_valid ) contained.destruct_value();
        else            contained.destruct_error();
    }

    // assignment

    typename std::enable_if<
        std::is_copy_constructible<T>::value
        && std::is_copy_assignable<T>::value
        && std::is_copy_constructible<E>::value
        && std::is_copy_assignable<E>::value,
    expected & >::type operator=( expected const & rhs )
    {
        if      ( bool(*this) && bool(rhs) ) { contained.value() = *rhs; }
        else if ( bool(*this) &&     !rhs  ) { contained.destruct_value();
                                               contained.construct_error( error.value() ); }
        else if (      !*this &&     !rhs  ) { contained.error() = *rhs; }
        else if (      !*this && bool(rhs) ) { contained.destruct_error();
                                               contained.construct_value( rhs.value() ); }

        is_valid = rhs.is_valid;

        return *this;
    }

    typename std::enable_if<
        std::is_move_constructible<T>::value
        && std::is_move_assignable<T>::value
        && std::is_move_constructible<E>::value
        && std::is_move_assignable<E>::value,
    expected & >::type operator=( expected && rhs )
    noexcept(
        std::is_nothrow_move_assignable<T>::value
        && std::is_nothrow_move_constructible<T>::value
        && std::is_nothrow_move_assignable<E>::value
        && std::is_nothrow_move_constructible<E>::value )
    {
        if      ( bool(*this) && bool(rhs) ) { contained.value() = std::move( *rhs ); }
        else if ( bool(*this) &&     !rhs  ) { contained.destruct_value();
                                               contained.construct_error( std::move( error.value() ) ); }
        else if (      !*this &&     !rhs  ) { contained.error() = std::move( *rhs ); }
        else if (      !*this && bool(rhs) ) { contained.destruct_error();
                                               contained.construct_value( std::move( rhs.value() ) ); }

        is_valid = rhs.is_valid;

        return *this;
    }

    template< typename U >
    typename std::enable_if<
        std::is_constructible<T,U>::value
        && std::is_assignable<T&, U>::value,
    expected & >::type operator=( U && v )
    {
        if ( bool(*this) )
        {
            contained.value() = std::forward( v );
        }
        else
        {
            contained.destruct_error();
            contained.construct_value( std::forward( v ) );
            is_valid = true;
        }
    }

    typename std::enable_if<
        std::is_copy_constructible<E>::value
        && std::is_assignable<E&, E>::value,
    expected & >::type operator=( unexpected_type<E> const & u )
    {
        if ( ! *this )
        {
            contained.error() = u.value();
        }
        else
        {
            contained.destruct_value();
            contained.construct_error( u.value() );
            is_valid = false;
        }
    }

    typename std::enable_if<
        std::is_copy_constructible<E>::value
        && std::is_assignable<E&, E>::value,
    expected & >::type operator=( unexpected_type<E> && u )
    {
        if ( ! *this )
        {
            contained.error() = std::forward( u.value() );
        }
        else
        {
            contained.destruct_value();
            contained.construct_error( std::forward( u.value() ) );
            is_valid = false;
        }
    }

    template< typename... Args >
    typename std::enable_if<
        std::is_constructible<T, Args&&...>::value,
    void>::type emplace( Args &&... args )
    {
        if ( bool(*this) )
        {
            contained.value() = value_type( std::forward<Args>( args )... );
        }
        else
        {
            contained.destruct_error();
            contained.construct_value( std::forward<Args>( args )... );
            is_valid = true;
        }
    }

    template< typename U, typename... Args >
    typename std::enable_if<
        std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value,
    void>::type emplace( std::initializer_list<U> il, Args &&... args )
    {
        if ( bool(*this) )
        {
            contained.value() = value_type( il, std::forward<Args>( args )... );
        }
        else
        {
            contained.destruct_error();
            contained.construct_value( il, std::forward<Args>( args )... );
            is_valid = true;
        }
    }

    // swap
    // The function shall not participate in overload resolution unless:
    //  LValues of type T shall be swappable, is_move_constructible<T>::value,
    //  LValues of type E shall be swappable and is_move_constructible<T>::value.
    void swap( expected & rhs )
    noexcept(
        std::is_nothrow_move_constructible<T>::value && noexcept(swap(std::declval<T&>(), std::declval<T&>())) &&
        std::is_nothrow_move_constructible<E>::value && noexcept(swap(std::declval<E&>(), std::declval<E&>())) )
    {
        using std::swap;

        if      ( bool(*this) && bool(rhs) ) { swap( contained.value(), rhs.contained.value() ); }
        else if (     !*this  &&     !rhs  ) { swap( contained.error(), rhs.contained.error() ); }
        else if ( bool(*this) &&     !rhs  ) { error_type t( std::move( rhs.error() ) );
                                               rhs   = std::move( *( *this ) );
                                               *this = std::move( t );
                                               swap( is_valid, rhs.is_valid ); }
        else if (     !*this  && bool(rhs) ) { rhs.swap( *this ); }
    }

    // observers

    constexpr value_type const * operator ->() const
    {
        assert( is_valid );
        return contained.value_ptr();
    }

    value_type * operator ->()
    {
        assert( is_valid );
        return contained.value_ptr();
    }

    constexpr value_type const & operator *() const &
    {
        assert( is_valid );
        return contained.value();
    }

    value_type & operator *() &
    {
        assert( is_valid );
        return contained.value();
    }

    constexpr value_type && operator *() const &&
    {
        assert( is_valid );
        return std::move( contained.value() );
    }

    constexpr explicit operator bool() const noexcept
    {
        return is_valid;
    }

    constexpr value_type const & value() const &
    {
        return is_valid ? contained.value() : ( std::rethrow_exception( contained.error() ), contained.value() );
    }

    value_type & value() &
    {
        return is_valid ? contained.value() : ( std::rethrow_exception( contained.error() ), contained.value() );
    }

    constexpr value_type && value() const &&
    {
        return is_valid ? std::move( contained.value() ) : ( std::rethrow_exception( contained.error() ), contained.value() );
    }

    constexpr error_type const & error() const &
    {
        return is_valid ? (throw nonstd::bad_expected_access("expected: no contained error"), contained.error() ) : contained.error();
    }

    error_type & error() &
    {
        return is_valid ? (throw nonstd::bad_expected_access("expected: no contained error"), contained.error() ) : contained.error();
    }

    constexpr error_type && error() const &&
    {
        return is_valid ? (throw nonstd::bad_expected_access("expected: no contained error"), contained.error() ) : std::move( contained.error() );
    }

    constexpr unexpected_type<E> get_unexpected() const
    {
        return unexpected_type<error_type>( contained.error() );
    }

    template< typename Ex >
    bool has_exception() const
    {
        return ! (*this) && std::is_base_of< Ex, decltype( get_unexpected().value() ) >::value;
    }

    // is_copy_constructible<T>::value && is_convertible<U&&, T>::value.
    template< typename U >
    constexpr value_type value_or( U && v ) const &
    {
        return is_valid ? **this : std::move( v );
    }

    // is_move_constructible<T>::value && is_convertible<U&&, T>::value
    template< typename U >
    value_type value_or( U && v ) const &&
    {
        return is_valid ? std::move( **this ) :std::move( v );
    }

    // unwrap()

//  template constexpr ’see below’ unwrap() const&;
//  template ’see below’ unwrap() &&;

    // factories

//  template <typename Ex, typename F>
//  expected<T,E> catch_exception(F&& f);

//  template <typename F>
//  expected<decltype(func(declval<T>())),E> map(F&& func) ;

//  template <typename F>
//  ’see below’ bind(F&& func);

//  template <typename F>
//  expected<T,E> catch_error(F&& f);

//  template <typename F>
//  ’see below’ then(F&& func);

private:
    bool is_valid;
    expected_detail::storage_t<T,E> contained;
};

// expected: relational operators

template <typename T, typename E>
constexpr bool operator==( expected<T,E> const & x, expected<T,E> const & y )
{
    return bool(x) != bool(y) ? false : bool(x) == false ? true : *x == *y;
}

template <typename T, typename E>
constexpr bool operator!=( expected<T,E> const & x, expected<T,E> const & y )
{
    return !(x == y);
}

template <typename T, typename E>
constexpr bool operator<( expected<T,E> const & x, expected<T,E> const & y )
{
    return (!y) ? false : (!x) ? true : *x < *y;
}

template <typename T, typename E>
constexpr bool operator>( expected<T,E> const & x, expected<T,E> const & y )
{
    return (y < x);
}

template <typename T, typename E>
constexpr bool operator<=( expected<T,E> const & x, expected<T,E> const & y )
{
    return !(y < x);
}

template <typename T, typename E>
constexpr bool operator>=( expected<T,E> const & x, expected<T,E> const & y )
{
    return !(x < y);
}

// expected: comparison with unexpected_type

template <typename T, typename E>
constexpr bool operator==( expected<T,E> const & x, unexpected_type<E> const & u )
{
    return (!x) ? x.get_unexpected() == u : false;
}

template <typename T, typename E>
constexpr bool operator==( unexpected_type<E> const & u, expected<T,E> const & x )
{
    return ( x == u );
}

template <typename T, typename E>
constexpr bool operator!=( expected<T,E> const & x, unexpected_type<E> const & u )
{
    return ! ( x == u );
}

template <typename T, typename E>
constexpr bool operator!=( unexpected_type<E> const & u, expected<T,E> const & x )
{
    return ! ( x == u );
}

template <typename T, typename E>
constexpr bool operator<( expected<T,E> const & x, unexpected_type<E> const & u )
{
    return (!x) ? ( x.get_unexpected() < u ) : false;
}

template <typename T, typename E>
constexpr bool operator<( unexpected_type<E> const & u, expected<T,E> const & x )
{
  return (!x) ? ( u < x.get_unexpected() ) : true ;
}

template <typename T, typename E>
constexpr bool operator>( expected<T,E> const & x, unexpected_type<E> const & u )
{
    return ( u < x );
}

template <typename T, typename E>
constexpr bool operator>( unexpected_type<E> const & u, expected<T,E> const & x )
{
    return ( x < u );
}

template <typename T, typename E>
constexpr bool operator<=( expected<T,E> const & x, unexpected_type<E> const & u )
{
    return ! ( u < x );
}

template <typename T, typename E>
constexpr bool operator<=( unexpected_type<E> const & u, expected<T,E> const & x)
{
    return ! ( x < u );
}

template <typename T, typename E>
constexpr bool operator>=( expected<T,E> const & x, unexpected_type<E> const & u  )
{
    return ! ( u > x );
}

template <typename T, typename E>
constexpr bool operator>=( unexpected_type<E> const & u, expected<T,E> const & x )
{
    return ! ( x > u );
}

// expected: comparison with T

template <typename T, typename E>
constexpr bool operator==( expected<T,E> const & x, T const & v )
{
    return bool(x) ? *x == v : false;
}

template <typename T, typename E>
constexpr bool operator==(T const & v, expected<T,E> const & x )
{
    return bool(x) ? v == *x : false;
}

template <typename T, typename E>
constexpr bool operator!=( expected<T,E> const & x, T const & v )
{
    return bool(x) ? *x != v : true;
}

template <typename T, typename E>
constexpr bool operator!=( T const & v, expected<T,E> const & x )
{
    return bool(x) ? v != *x : true;
}

template <typename T, typename E>
constexpr bool operator<( expected<T,E> const & x, T const & v )
{
    return bool(x) ? *x < v : true;
}

template <typename T, typename E>
constexpr bool operator<( T const & v, expected<T,E> const & x )
{
    return bool(x) ? v < *x : false;
}

template <typename T, typename E>
constexpr bool operator>( T const & v, expected<T,E> const & x )
{
    return bool(x) ? *x < v : false;
}

template <typename T, typename E>
constexpr bool operator>( expected<T,E> const & x, T const & v )
{
    return bool(x) ? v < *x : false;
}

template <typename T, typename E>
constexpr bool operator<=( T const & v, expected<T,E> const & x )
{
    return bool(x) ? ! ( *x < v ) : false;
}

template <typename T, typename E>
constexpr bool operator<=( expected<T,E> const & x, T const & v )
{
    return bool(x) ? ! ( v < *x ) : true;
}

template <typename T, typename E>
constexpr bool operator>=( expected<T,E> const & x, T const & v )
{
    return bool(x) ? ! ( *x < v ) : false;
}

template <typename T, typename E>
constexpr bool operator>=( T const & v, expected<T,E> const & x )
{
    return bool(x) ? ! ( v < *x ) : true;
}

// expected: specialized algorithms

template< typename T, typename E >
void swap( expected<T,E> & x, expected<T,E> & y ) noexcept( noexcept( x.swap(y) ) )
{
    x.swap( y );
}

//template< typename T >
//auto make_expected( T const & v ) -> expected<T>
//{
//    return expected<T>( v );
//}

template< typename T>
constexpr auto make_expected( T && v ) -> expected< typename std::decay<T>::type >
{
    return expected< typename std::decay<T>::type >( std::forward( v ) );
}

//auto make_expected() -> expected<void>
//{
//    return expected<void>( in_place );
//}

template< typename T>
constexpr auto make_expected_from_current_exception() -> expected<T>
{
    return expected<T>( make_unexpected_from_current_exception() );
}

template< typename T>
constexpr auto make_expected_from_exception( std::exception_ptr v ) -> expected<T>
{
    return expected<T>( unexpected_type<>( std::forward<std::exception_ptr>( v ) ) );
}

template< typename T, typename E >
constexpr auto make_expected_from_error( E e ) -> expected<T, typename std::decay<E>::type>
{
    return expected<T, typename std::decay<E>::type>( make_unexpected( e ) );
}

template< typename F >
/*nsel_constexpr14*/ auto make_expected_from_call( F f ) -> expected< typename std::result_of<F>::type >
{
    try
    {
        return make_expected( f() );
    }
    catch (...)
    {
        return make_unexpected_from_current_exception();
    }
}

} // namespace nonstd

namespace std {

// expected: hash support

template <typename T>
struct hash<nonstd::expected<T>>
{
    typedef typename hash<T>::result_type result_type;
    typedef nonstd::expected<T> argument_type;

    constexpr result_type operator()(argument_type const & arg) const
    {
        return arg ? std::hash<T>{}(*arg) : result_type{};
    }
};

template <typename T>
struct hash<nonstd::expected<T&>>
{
    typedef typename hash<T>::result_type result_type;
    typedef nonstd::expected<T&> argument_type;

    constexpr result_type operator()(argument_type const & arg) const
    {
        return arg ? std::hash<T>{}(*arg) : result_type{};
    }
};

} // namespace std

#endif // NONSTD_EXPECTED_LITE_HPP
