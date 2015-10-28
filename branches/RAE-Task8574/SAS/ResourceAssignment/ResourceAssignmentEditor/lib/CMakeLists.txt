# $Id$

python_install(
    __init__.py
    webservice.py
    utils.py
    DESTINATION resourceassignementeditor)

set(web_files
    static/favicon.ico
    static/js/angular-1.4.7.min.js
    static/js/angular.min.js
    static/js/jquery-2.1.4.min.js
    static/js/jquery.min.js
    static/app/app.js
    static/app/controllers/controller.js
    templates/index.html)

install(FILES ${web_files}
  DESTINATION ${PYTHON_INSTALL_DIR}/resourceassignementeditor)

foreach(web_file ${web_files})
    get_filename_component(web_file_path ${web_file} PATH)
    get_filename_component(web_file_abs ${web_file} ABSOLUTE)

    file(MAKE_DIRECTORY ${_build_dir}/${web_file_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${web_file_abs} ${_build_dir}/${web_file})
endforeach(web_file ${web_files})

