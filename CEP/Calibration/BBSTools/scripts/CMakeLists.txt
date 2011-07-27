include(PythonInstall)


python_install(
    __init__.py
    testsip.py
    testbbs.py 
    DESTINATION lofar/bbs) 

install(PROGRAMS bbstests.sh DESTINATION bin)
