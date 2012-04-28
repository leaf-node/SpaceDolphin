#   Copyright 2011 Andrew Engelbrecht <sudoman@ninthfloor.org>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required (VERSION 2.6)
project (spacedolphin)


option (USE_LOCAL_CP "statically link against CP in source tree" ON)

if (USE_LOCAL_CP)

    add_subdirectory ("${PROJECT_SOURCE_DIR}/lib/chipmunk" EXCLUDE_FROM_ALL)

    include_directories ("${PROJECT_SOURCE_DIR}/lib/chipmunk/include/chipmunk")
    find_package (SDL REQUIRED)
    set (EXTRA_LIBS ${EXTRA_LIBS} chipmunk_static)

else(USE_LOCAL_CP)

    include_directories (/usr/include/chipmunk /usr/local/include/chipmunk/)
    set (EXTRA_LIBS ${EXTRA_LIBS} chipmunk)

endif(USE_LOCAL_CP)

set (EXTRA_LIBS ${EXTRA_LIBS} m SDL cairo)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

include_directories("${PROJECT_SOURCE_DIR}/include")
file (GLOB spacedolphin_source "${PROJECT_SOURCE_DIR}/src/*.c")
add_executable (spacedolphin ${spacedolphin_source})
target_link_libraries (spacedolphin ${EXTRA_LIBS})

install (TARGETS spacedolphin RUNTIME DESTINATION bin)
include (CPack)

