# $Id: CMakeLists.txt 32341 2015-08-28 11:59:26Z schaap $

python_install(
    __init__.py
    webservice.py
    static/favicon.ico
    templates/index.html
    DESTINATION resourceassignementeditor)

