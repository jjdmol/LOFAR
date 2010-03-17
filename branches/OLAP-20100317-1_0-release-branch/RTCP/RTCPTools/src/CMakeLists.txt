# $Id$

find_library(M_LIBRARY m)

add_executable(generator generator.c)
target_link_libraries(generator ${M_LIBRARY})

add_executable(udp-copy udp-copy.c)
