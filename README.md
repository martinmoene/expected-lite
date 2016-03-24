expected lite - expected objects for C++11 and later&ensp;[![Build Status](https://travis-ci.org/martinmoene/expected-lite.png?branch=master)](https://travis-ci.org/martinmoene/expected-lite)
============================================

*expected lite* is a single-file header-only library for objects that either present a valid value or an error that you can pass by value. It is intended for use with C++11 and later. The library is based on the std:&#58;experimental:&#58;expected proposal [1] .

**Contents**  
- [Example usage](#example-usage)
- [In a nutshell](#in-a-nutshell)
- [License](#license)
- [Dependencies](#dependencies)
- [Installation](#installation)
- [Synopsis](#synopsis)
- [Comparison of std:&#58;experimental:&#58;expected, expected lite and ...](#comparison)
- [Reported to work with](#reported-to-work-with)
- [Implementation notes](#implementation-notes)
- [Notes and references](#notes-and-references)
- [Appendix](#appendix)


Example usage
-------------
```C++
#include "expected.hpp"
```
### Compile and run
```
...
```

In a nutshell
---------------

License
-------
*expected lite* uses the [MIT](LICENSE) license.


Dependencies
------------
*expected lite* has no other dependencies than the [C++ standard library](http://en.cppreference.com/w/cpp/header).


Installation
------------

*expected lite* is a single-file header-only library. Put `expected.hpp` directly into the project source tree or somewhere reachable from your project.


Synopsis
--------

**Contents**  
- [Configuration macros](#configuration-macros)
- [Types in namespace nonstd](#types-in-namespace-nonstd)  
- [Interface of expected](#interface-of-expected)  
- [Algorithms for expected](#algorithms-for-expected)  
- [Interface of unexpected_type](#interface-of-unexpected_type)  
- [Algorithms for unexpected_type](#algorithms-for-unexpected_type)  

### Configuration macros

\-D<b>nsel\_CONFIG\_CONFIRMS\_COMPILATION\_ERRORS</b>=0  
Define this macro to 1 to experience the by-design compile-time errors of the library in the test suite. Default is 0.

### Types in namespace nonstd

| Purpose         | Type | Object |
|-----------------|------|--------|
| To be, or not   | template< typename T, typename E = std::exception_ptr ><br>class expected; |&nbsp;|
| Error type      | template< typename E ><br>class unexpected_type; | &nbsp; |
| Traits          | template< typename E ><br>struct is_unexpected;    | &nbsp; |
| In-place value construction | struct in_place_t;            | in_place_t in_place{}; |
| In-place error construction | struct in_place_unexpected_t; | in_place_unexpected_t unexpect{}; |
| In-place error construction | struct in_place_unexpected_t; | in_place_unexpected_t in_place_unexpected{}; |
| Error reporting             | class bad_expected_access;    |&nbsp; |

### Interface of expected

| Kind         | Method                                                              | Result |
|--------------|---------------------------------------------------------------------|--------|
| Construction | [constexpr] expected() noexcept(...)                                | an object with default value |
| &nbsp;       | [constexpr] expected( expected const & other )                      | initialize to contents of other |
| &nbsp;       | [constexpr] expected( expected && other )                           | move contents from other |
| &nbsp;       | [constexpr] expected( value_type const & value )                    | initialize to value |
| &nbsp;       | [constexpr] expected( value_type && value ) noexcept(...)           | move from value |
| &nbsp;       | [constexpr] explicit expected( in_place_t, Args&&... args )         | construct value in-place from args |
| &nbsp;       | [constexpr] explicit expected( in_place_t,<br>&emsp;std::initializer_list&lt;U> il, Args&&... args ) | construct value in-place from args |
| &nbsp;       | [constexpr] expected( unexpected_type<E> const & error )            | initialize to error |
| &nbsp;       | [constexpr] expected( unexpected_type<E> && error )                 | move from error |
| &nbsp;       | [constexpr] explicit expected( in_place_unexpected_t,<br>&emsp;Args&&... args ) | construct error in-place from args |
| &nbsp;       | [constexpr] explicit expected( in_place_unexpected_t,<br>&emsp;std::initializer_list&lt;U> il, Args&&... args )| construct error in-place from args |
| Destruction  | ~expected()                                                         | destruct current content |
| Assignment   | expected operator=( expected const & other )                        | assign contents of other;<br>destruct current content, if any |
| &nbsp;       | expected & operator=( expected && other ) noexcept(...)             | move contents of other |
| &nbsp;       | expected & operator=( U && v )                                      | move value from v |
| &nbsp;       | expected & operator=( unexpected_type<E> const & u )                | initialize to unexpected |
| &nbsp;       | expected & operator=( unexpected_type<E> && u )                     | move from unexpected |
| &nbsp;       | template< typename... Args ><br>void emplace( Args &&... args )     | emplace from args |
| &nbsp;       | template< typename U, typename... Args ><br>void emplace( std::initializer_list&lt;U> il, Args &&... args )  | emplace from args |
| Swap         | void swap( expected & other ) noexcept                              | swap with other  |
| Observers    | constexpr value_type const * operator ->() const                    | pointer to current content (const);<br>must contain value |
| &nbsp;       | value_type * operator ->()                                          | pointer to current content (non-const);<br>must contain value |
| &nbsp;       | constexpr value_type const & operator *() const &                   | the current content (const ref);<br>must contain value |
| &nbsp;       | constexpr value_type && operator *() &&                             | the current content (non-const ref);<br>must contain value |
| &nbsp;       | constexpr explicit operator bool() const noexcept                   | true if contains value |
| &nbsp;       | constexpr value_type const & value() const &                        | current content (const ref);<br>see [note 1](#note1) |
| &nbsp;       | value_type & value() &                                              | current content (non-const ref);<br>see [note 1](#note1) |
| &nbsp;       | constexpr value_type && value() &&                                  | move from current content;<br>see [note 1](#note1) |
| &nbsp;       | constexpr error_type const & error() const &                        | current error (const ref);<br>must contain error |
| &nbsp;       | error_type & error() &                                              | current error (non-const ref);<br>must contain error |
| &nbsp;       | constexpr error_type && error() &&                                  | move from current error;<br>must contain error |
| &nbsp;       | constexpr unexpected_type<E> get_unexpected() const                 | the error as unexpected&lt;>;<br>must contain error |
| &nbsp;       | template< typename Ex ><br>bool has_exception() const               | true of contains exception (as base) |
| &nbsp;       | value_type value_or( U && v ) const &                               | value or move from v |
| &nbsp;       | value_type value_or( U && v ) &&                                    | move from value or move from v |
| &nbsp;       | ... | &nbsp; |

<a id="note1"></a>Note 1: if no content, for std::exception_ptr rethrows error(), otherwise throws bad_expected_access(error()).

### Algorithms for expected

| Kind                   | Function |
|------------------------|----------|
| Relational operators   | &nbsp;   | 
| ==&ensp;!=&ensp;<&ensp;>&ensp;<=&ensp;>= | template< typename T, typename E ><br>constexpr bool operator *op*(<br>&emsp;expected&lt;T,E> const & x,<br>&emsp;expected&lt;T,E> const & y ) |
| Comparison with unexpected_type | &nbsp; | 
| ==&ensp;!=&ensp;<&ensp;>&ensp;<=&ensp;>= | template< typename T, typename E ><br>constexpr bool operator *op*(<br>&emsp;expected&lt;T,E> const & x,<br>&emsp;unexpected_type&lt;E> const & u ) | 
| &nbsp;                                   | template< typename T, typename E ><br>constexpr bool operator *op*(<br>&emsp;unexpected_type&lt;E> const & u,<br>&emsp;expected&lt;T,E> const & x ) | 
| Comparison with T                        | &nbsp;   | 
| ==&ensp;!=&ensp;<&ensp;>&ensp;<=&ensp;>= | template< typename T, typename E ><br>constexpr bool operator *op*(<br>&emsp;expected&lt;T,E> const & x,<br>&emsp;T const & v ) | 
| &nbsp;                                   | template< typename T, typename E ><br>constexpr bool operator *op*(<br>&emsp;T const & v,<br>&emsp;expected&lt;T,E> const & x ) | 
| Specialized algorithms | &nbsp;   | 
| Swap                   | template< typename T, typename E ><br>void swap(<br>&emsp;expected&lt;T,E> & x,<br>&emsp;expected&lt;T,E> & y )&emsp;noexcept( noexcept( x.swap(y) ) ) | 
| Make expected from     | &nbsp;   | 
| &emsp;Value            | template< typename T><br>constexpr auto make_expected( T && v ) -><br>&emsp;expected< typename std::decay&lt;T>::type > | 
| &emsp;Nothing          | auto make_expected() -> expected&lt;void> | 
| &emsp;Current exception| template< typename T><br>constexpr auto make_expected_from_current_exception() -> expected&lt;T> | 
| &emsp;Exception        | template< typename T><br>auto make_expected_from_exception( std::exception_ptr v ) -> expected&lt;T>| 
| &emsp;Error            | template< typename T, typename E ><br>constexpr auto make_expected_from_error( E e ) -><br>&emsp;expected&lt;T, typename std::decay&lt;E>::type> | 
| &emsp;Call             | template< typename F ><br>auto make_expected_from_call( F f ) -><br>&emsp;expected< typename std::result_of&lt;F()>::type >| 
| &emsp;Call, void specialization | template< typename F ><br>auto make_expected_from_call( F f ) -> expected&lt;void> | 

### Interface of unexpected_type

| Kind         | Method                                                | Result |
|--------------|-------------------------------------------------------|--------|
| Construction | unexpected_type() = delete;                           | no default construction |
| &nbsp;       | constexpr explicit unexpected_type( E const & error ) | copy-constructed from an E |
| &nbsp;       | constexpr explicit unexpected_type( E && error )      | move-constructed from an E |
| Observers    | constexpr error_type const & value() const            | can observe contained error |
| &nbsp;       | error_type & value()                                  | can modify contained error |

### Algorithms for unexpected_type

| Kind                   | Function |
|------------------------|----------|
| Relational operators   | &nbsp;   | 
| ==&ensp;!=&ensp;<&ensp;>&ensp;<=&ensp;>= | template< typename E ><br>constexpr bool operator *op*(<br>&emsp;unexpected_type&lt;E> const & x,<br>&emsp;unexpected_type&lt;E> const & y ) |
| ==&ensp;!=&ensp;<&ensp;>&ensp;<=&ensp;>= | constexpr bool operator *op*(<br>&emsp;unexpected_type&lt;std::exception_ptr> const & x,<br>&emsp;unexpected_type&lt;std::exception_ptr> const & y ) |
| Specialized algorithms | &nbsp;   | 
| Make unexpected from   | &nbsp;   | 
| &emsp;Current exception| [constexpr] auto make_unexpected_from_current_exception() -><br>&emsp;unexpected_type< std::exception_ptr >| 
| &emsp;Error            | template< typename E><br>[constexpr] auto make_unexpected( E && v) -><br>&emsp;unexpected_type< typename std::decay&lt;E>::type >| 


<a id="comparison"></a>
Comparison with std:&#58;expected, std:&#58;optional and std::pair
------------------------------------------------------------------

|Feature               | std::pair | std::optional | std::expected | nonstd::expected |
|----------------------|-----------|---------------|---------------|------------------|
|More information      | see [x]   | see [x]       | see [x]       | this work        | 
|                      |           |               |               |                  |               

std:&#58;*experimental*:&#58;expected

Reported to work with
---------------------


Implementation notes
--------------------


Notes and references
--------------------

[1] Vicente J. Botet Escriba. [Dxxxxr0 - A proposal to add a utility class to represent expected monad (Revision 2)](https://github.com/viboes/std-make/blob/master/doc/proposal/expected/DXXXXR0_expected.pdf) (PDF).  
[2] Vicente J. Botet Escriba. [Expected - An exception-friendly Error Monad](https://www.youtube.com/watch?v=Zdlt1rgYdMQ). C++Now 2014. 24 September 2014.  

Appendix
--------
### A.1 expected lite test specification

```
unexpected_type<>: Disallows default construction
unexpected_type<>: Disallows default construction, std::exception_ptr specialization
unexpected_type<>: Allows to copy-construct from error_type
unexpected_type<>: Allows to copy-construct from error_type, std::exception_ptr specialization
unexpected_type<>: Allows to move-construct from error_type
unexpected_type<>: Allows to move-construct from error_type, std::exception_ptr specialization
unexpected_type<>: Allows to copy-construct from an exception, std::exception_ptr specialization
unexpected_type<>: Allows to observe its value
unexpected_type<>: Allows to observe its value, std::exception_ptr specialization
unexpected_type<>: Allows to modify its value
unexpected_type<>: Allows to modify its value, std::exception_ptr specialization
unexpected_type<>: Provides relational operators
unexpected_type<>: Supports relational operators, std::exception_ptr specialization
is_unexpected<X>: Is true for unexpected_type
is_unexpected<X>: Is false for non-unexpected_type (int)
make_unexpected(): Allows to create an unexpected_type<E> from an E
make_unexpected_from_current_exception(): Allows to create an unexpected_type<std::exception_ptr> from the current exception
bad_expected_access<>: Disallows default construction
bad_expected_access<>: Allows construction from error_type
bad_expected_access<>: Allows to observe its error
bad_expected_access<>: Allows to change its error
expected<>: Allows default construction
expected<>: Allows to copy-construct from value_type
expected<>: Allows to move-construct from value_type
expected<>: Allows to copy-construct from expected<>
expected<>: Allows to move-construct from expected<>
expected<>: Allows to in-place-construct value_type
expected<>: Allows to copy-construct from unexpected_type<>
expected<>: Allows to move-construct from unexpected_type<>
expected<>: Allows to in-place-construct unexpected_type<>
expected<>: Allows to copy-assign from expected<>
expected<>: Allows to move-assign from expected<>
expected<>: Allows to copy-assign from unexpected_type<>
expected<>: Allows to move-assign from unexpected_type<>
expected<>: Allows to copy-assign from type convertible to value_type
expected<>: Allows to move-assign from type convertible to value_type
expected<>: Allows to emplace a value_type
expected<>: Allows to be swapped
expected<>: Allows to observe its value via a pointer
expected<>: Allows to modify its value via a pointer
expected<>: Allows to observe its value via a reference
expected<>: Allows to observe its value via a r-value reference
expected<>: Allows to modify its value via a reference
expected<>: Allows to observe if it contains a value (or error)
expected<>: Allows to observe its value
expected<>: Allows to modify its value
expected<>: Allows to move its value
expected<>: Allows to observe its error
expected<>: Allows to modify its error
expected<>: Allows to move its error
expected<>: Allows to observe its error as unexpected<>
expected<>: Allows to observe its value if available, or obtain a specified value otherwise
expected<>: Allows to move its value if available, or obtain a specified value otherwise
expected<>: Provides relational operators
...
```
