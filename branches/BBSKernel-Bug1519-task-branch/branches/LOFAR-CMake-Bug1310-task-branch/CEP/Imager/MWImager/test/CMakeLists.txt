# $Id$

include(LofarCTest)

lofar_add_test(tconvertimagerparset DEPENDS convertimagerparset)
lofar_add_test(tmwimager DEPENDS convertimagerparset)
lofar_add_test(tmwimager-dd DEPENDS convertimagerparset)
