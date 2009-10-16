# $Id$

include(LofarCTest)

lofar_add_test(tMSSplit tMSSplit.cc)
lofar_add_test(tcombinevds DEPENDS combinevds)
lofar_add_test(tmakems DEPENDS makems)
