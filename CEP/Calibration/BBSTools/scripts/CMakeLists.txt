include(PythonInstall)


python_install(
    __init__.py
    testsip.py
    testdppp.py
    testbbs.py 
    DESTINATION lofar/bbs) 

lofar_add_bin_scripts(
	bbstests.sh 
	sbprogress.sh)
#lofar_add_bin_scripts(BBStiming.py)
