# Copyright 2016-2018 by Martin Moene
#
# Distributed under the Boost Software License, Version 1.0. 
# (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required( VERSION 3.0 )

project( expected_lite )

set( EXPEXTED_P0323R "99" STRING "Specify proposal revision compatibility (99: latest)" )

enable_testing()

add_subdirectory( test )
add_subdirectory( example )

# end of file
