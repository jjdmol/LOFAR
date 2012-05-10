# $Id$

lofar_package(OTDB_COMPS 0.1)
install(PROGRAMS
  create_OTDB_comps
  createPICfile
  loadComponents
  loadPICtree
DESTINATION sbin)

file(GLOB otdb_comps
  *.comp
  *.list
  *.base)
install(FILES
  ${otdb_comps}
  DESTINATION etc/otdb)
