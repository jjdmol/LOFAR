include(PythonInstall)


python_install(
    __init__.py
    testsip.py
    testdppp.py
    testbbs.py 
    DESTINATION lofar/bbs) 

install(PROGRAMS 
	bbstests.sh 
	sbprogress.sh
	DESTINATION bin)
#install(PROGRAMS BBStiming.py DESTINATION bin)
