include_directories(${CMAKE_BINARY_DIR})

add_executable(LimitExec LimitExec.cc)
add_executable(pkgdep pkgdep.cc)

install(TARGETS
  LimitExec
  pkgdep
  DESTINATION sbin)

install(PROGRAMS
  checkrun
  finddep
  lofarcvsmv
  lofarcvsx
  makedoc
  makedep.sh
  makeClass.py
  makePackage.py
  makeTest.py
  DESTINATION sbin)

install(FILES
  templates/header.cc_template
  templates/header.h_template
  templates/main.cc_template  		      
  templates/package_bootstrap_template  		      
  templates/package_configure.in_template  		      
  templates/package_spec.in_template  		      
  templates/package_lofarconf.in_template
  templates/package_makefile.am_template  		      
  templates/include_makefile.am_template  		      
  templates/include_pkg_makefile.am_template  		      
  templates/src_makefile.am_template  	              
  templates/templated_header.cc_template  		      
  templates/templated_header.h_template  		      
  templates/templated_header.tcc_template
  templates/test.cc_template
  templates/test.in_template
  templates/test.out_template
  templates/test.sh_template  		      
  templates/test_makefile.am_template
  DESTINATION share/templates)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lofarinit.sh.in
               ${CMAKE_BINARY_DIR}/lofarinit.sh @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lofarinit.csh.in
               ${CMAKE_BINARY_DIR}/lofarinit.csh @ONLY)

install(FILES
  ${CMAKE_BINARY_DIR}/lofarinit.sh
  ${CMAKE_BINARY_DIR}/lofarinit.csh
  DESTINATION .)
