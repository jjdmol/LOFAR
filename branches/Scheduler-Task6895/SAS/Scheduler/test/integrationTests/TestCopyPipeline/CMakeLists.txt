# $Id$

add_executable(TestCopyPipeline main.cpp)
target_link_libraries(TestCopyPipeline lofar_scheduler)
add_test(TestCopyPipeline TestCopyPipeline)
