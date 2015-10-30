# $Id$

add_executable(TestSASConnection main.cpp)
target_link_libraries(TestSASConnection lofar_scheduler)
add_test(TestSASConnection TestSASConnection)
