# $Id$

include(LofarCTest)

lofar_add_test(tCorrelate_1sec_1st_5sb_noflagging DEPENDS rtcp outputProc)
lofar_add_test(tCorrelate_3sec_1st_5sb DEPENDS rtcp outputProc)
lofar_add_test(tCorrelate_3sec_2st_5sb DEPENDS rtcp outputProc)
