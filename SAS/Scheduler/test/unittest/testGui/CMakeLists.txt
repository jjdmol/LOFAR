# $Id$

qt4_generate_moc(testgui.cpp testgui.moc)
add_executable(testGui testgui.cpp testgui.moc)
target_link_libraries(testGui ${QT_LIBRARIES})
add_test(testGui testGui)
