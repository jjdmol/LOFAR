# $Id: CMakeLists.txt 32905 2015-11-17 15:31:54Z schaap $

python_install(
    __init__.py
    rotspservice.py
    translator.py
    config.py
    propagator.py
    rabuslistener.py
    otdbrpc.py
    DESTINATION lofar/sas/resourceassignment/ratootdbtaskspecificationpropagator)

