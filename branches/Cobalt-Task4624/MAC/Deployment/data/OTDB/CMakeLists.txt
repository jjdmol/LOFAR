# $Id$

lofar_package(OTDB_COMPS 1.0)

install(PROGRAMS
  create_OTDB_comps
  createPICfile
  loadComponents
  loadPICtree
DESTINATION sbin)

install(PROGRAMS Component_diff.sh DESTINATION sbin RENAME Component_diff)

file(GLOB otdb_comps
  *.comp
  *.list
  *.base
  *.tmpl)
install(FILES
  ${otdb_comps}
  DESTINATION etc/otdb)
