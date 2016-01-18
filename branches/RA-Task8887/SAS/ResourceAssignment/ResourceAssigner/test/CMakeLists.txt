# $Id: CMakeLists.txt 32333 2015-08-28 08:15:24Z schaap $
include(LofarCTest)

lofar_add_test(test_ra_test1)

python_install(
    test_ra_test1.py
    EXECUTABLE
    DESTINATION lofar/sas/resourceassignment/resourceassigner/test)

