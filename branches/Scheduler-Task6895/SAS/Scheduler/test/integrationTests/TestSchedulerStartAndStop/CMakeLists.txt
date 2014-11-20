# $Id$

add_executable(TestSchedulerStartAndStop main.cpp)
target_link_libraries(TestSchedulerStartAndStop lofar_scheduler)
add_test(TestSchedulerStartAndStop TestSchedulerStartAndStop)
