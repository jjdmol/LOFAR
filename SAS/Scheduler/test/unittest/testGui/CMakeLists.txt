# $Id$

include(LofarCTest)

qt4_generate_moc(testgui.cpp testgui.moc)
lofar_add_test(testGui testgui.cpp testgui.moc)
