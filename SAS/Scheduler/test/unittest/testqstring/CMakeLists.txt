# $Id$

qt4_generate_moc(testqstring.cpp testqstring.moc)
add_executable(testqstring testqstring.cpp testqstring.moc)
target_link_libraries(testqstring ${QT_LIBRARIES})
add_test(testqstring testqstring)
