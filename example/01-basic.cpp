#include "expected.hpp"
#include <iostream>
#include <string>

using namespace nonstd;
using namespace std::literals;

auto produce() -> expected<int, std::string> 
{
    static int x = 0;
    
    if ( x < 5 ) return ++x;
    else         return make_unexpected( "couldn't produce"s );
}

int main()
{
    while ( auto ei = produce() )
    {
        std::cout << *ei << ", ";
    }
    std::cout << produce().error(); 
}

// cl -EHsc -wd4814 -I../include/nonstd 01-basic.cpp && 01-basic.exe
// g++ -std=c++14 -Wall -I../include/nonstd -o 01-basic.exe 01-basic.cpp && 01-basic.exe
// 1, 2, 3, 4, 5, couldn't produce
