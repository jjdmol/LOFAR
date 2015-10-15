# $Id: CMakeLists.txt 32341 2015-08-28 11:59:26Z schaap $

python_install(
    __init__.py
    scraper.py
    store.py
    webservice/webservice.py
    webservice/__init__.py
    DESTINATION ltastorageoverview)

set(web_files webservice/templates/index.html)

install(FILES ${web_files}
  DESTINATION ltastorageoverview/webservice/templates/)

foreach(web_file ${web_files})
    get_filename_component(web_file_path ${web_file} PATH)
    get_filename_component(web_file_abs ${web_file} ABSOLUTE)

    file(MAKE_DIRECTORY ${_build_dir}/${web_file_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${web_file_abs} ${_build_dir}/${web_file})
endforeach(web_file ${web_files})

