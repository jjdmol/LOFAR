# $Id: CMakeLists.txt 32905 2015-11-17 15:31:54Z schaap $

python_install(
    __init__.py
    raservice.py
    assignment.py
    config.py
    rapublisher.py
    DESTINATION lofar/sas/resourceassignment/resourceassigner)

