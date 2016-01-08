# $Id$
include(LofarCTest)

lofar_add_test(test_store)
lofar_add_test(test_lso_webservice)

python_install(
    test_store.py
    test_lso_webservice.py
    DESTINATION ltastorageoverview/test)

