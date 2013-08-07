# $Id$

# All env. control python scripts and libs to sbin 
file (GLOB ecfiles *.py)
install(FILES
  ${ecfiles}
  DESTINATION sbin)

install(PROGRAMS poweruphba.sh DESTINATION sbin)
