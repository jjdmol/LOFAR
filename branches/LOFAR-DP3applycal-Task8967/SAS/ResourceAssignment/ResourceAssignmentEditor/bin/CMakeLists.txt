# $Id: CMakeLists.txt 32341 2015-08-28 11:59:26Z schaap $

set(progs resourceassignementeditorwebservice)

install(PROGRAMS
  ${progs}
  DESTINATION bin)

#create 'virtual' install tree in build dir, just as python install does.add_custom_command(
#TODO: replace by LOFAR_ADD_BIN_SCRIPT when marcel loose implemented that marco
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
foreach(prog ${progs})
    get_filename_component(prog_abs ${prog} ABSOLUTE)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${prog_abs} ${CMAKE_BINARY_DIR}/bin/${prog})
endforeach(prog ${progs})




