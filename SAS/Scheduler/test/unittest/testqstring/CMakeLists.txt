# $Id$

include(LofarCTest)

qt4_generate_moc(testqstring.cpp testqstring.moc)
lofar_add_test(testqstring testqstring.cpp testqstring.moc)
