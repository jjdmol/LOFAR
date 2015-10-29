# $Id$

python_install(
    __init__.py
    webservice.py
    utils.py
    DESTINATION resourceassignementeditor)

file(GLOB_RECURSE jquery_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/jquery/*)
file(GLOB_RECURSE angular_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular/*)
file(GLOB_RECURSE angular_route_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-route/*)
file(GLOB_RECURSE angular_touch_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-touch/*)
file(GLOB_RECURSE angular_ui_grid_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-ui-grid/*)

set(web_files
    ${jquery_files}
    ${angular_files}
    ${angular_route_files}
    ${angular_touch_files}
    ${angular_ui_grid_files}
    static/favicon.ico
    static/app/app.js
    static/app/controllers/controller.js
    static/css/main.css
    templates/index.html)

install(FILES ${web_files}
  DESTINATION ${PYTHON_INSTALL_DIR}/resourceassignementeditor)

foreach(web_file ${web_files})
    get_filename_component(web_file_path ${web_file} PATH)
    get_filename_component(web_file_abs ${web_file} ABSOLUTE)

    file(MAKE_DIRECTORY ${_build_dir}/${web_file_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${web_file_abs} ${_build_dir}/${web_file})
endforeach(web_file ${web_files})

