# $Id: CMakeLists.txt 32333 2015-08-28 08:15:24Z schaap $
include(LofarCTest)

lofar_add_test(test_store)
lofar_add_test(test_webservice)

python_install(
    test_store.py
    test_webservice.py
    DESTINATION ltastorageoverview/test)

