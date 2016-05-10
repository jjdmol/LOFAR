# $Id$

python_install(
    __init__.py
    webservice.py
    utils.py
    radbchangeshandler.py
    fakedata.py
    mom.py
    DESTINATION lofar/sas/resourceassignment/resourceassignmenteditor)

file(GLOB_RECURSE jquery_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/jquery/*)
file(GLOB_RECURSE angular_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular/*)
file(GLOB_RECURSE angular_route_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-route/*)
file(GLOB_RECURSE angular_touch_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-touch/*)
file(GLOB_RECURSE angular_resource_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-resource/*)
file(GLOB_RECURSE angular_ui_grid_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-ui-grid/*)
file(GLOB_RECURSE angular_ui_tree_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-ui-tree/*)
file(GLOB_RECURSE angular_ui_layout_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-ui-layout/*)
file(GLOB_RECURSE angular_ui_tabs_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-ui-tabs/*)
file(GLOB_RECURSE angular_gantt_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-gantt/*)
file(GLOB_RECURSE angular_moment_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/angular-moment/*)
file(GLOB_RECURSE jsplumb_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/jsplumb/*)
file(GLOB_RECURSE moment_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/moment/*)
file(GLOB_RECURSE bootstrap_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/css/bootstrap.min.css)
file(GLOB_RECURSE fonts_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/fonts/*)
file(GLOB_RECURSE utils_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/utils/*)
file(GLOB_RECURSE highcharts_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} static/js/highcharts/*)

set(app_files
    static/favicon.ico
    static/app/app.js
    static/app/controllers/datacontroller.js
    static/app/controllers/gridcontroller.js
    static/app/controllers/ganttresourcecontroller.js
    static/app/controllers/chartresourceusagecontroller.js
    static/app/controllers/ganttprojectcontroller.js
    static/app/gantt-plugins/angular-gantt-contextmenu-plugin.js
    static/css/main.css
    templates/index.html)

set(web_files 
    ${jquery_files}
    ${angular_files}
    ${angular_route_files}
    ${angular_touch_files}
    ${angular_resource_files} 
    ${angular_ui_grid_files}
    ${angular_ui_tree_files}
    ${angular_ui_layout_files}
    ${angular_ui_tabs_files}
    ${angular_gantt_files}
    ${angular_moment_files}
    ${moment_files}
    ${jsplumb_files}
    ${bootstrap_files} 
    ${fonts_files}
    ${utils_files}
    ${highcharts_files}
    ${app_files})

foreach(web_file ${web_files})
    get_filename_component(web_file_path ${web_file} PATH)
    get_filename_component(web_file_abs ${web_file} ABSOLUTE)

    file(MAKE_DIRECTORY ${_build_dir}/${web_file_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${web_file_abs} ${_build_dir}/${web_file})

    install(FILES ${web_file_abs} DESTINATION ${PYTHON_INSTALL_DIR}/lofar/sas/resourceassignment/resourceassignmenteditor/${web_file_path})
endforeach(web_file ${web_files})

