# Copyright 2016-2018 by Martin Moene
#
# This code is licensed under the MIT License (MIT).

cmake_minimum_required( VERSION 3.0 )

project( expected_lite )

set( EXPEXTED_P0323R "99" STRING "Specify proposal revision compatibility (99: latest)" )

enable_testing()

add_subdirectory( test )
add_subdirectory( example )

# end of file
