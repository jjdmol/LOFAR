include(PythonInstall)

python_install(
    __init__.py
    testbbs.py 
    DESTINATION lofar/bbs)
