# $Id$

# All env. control python scripts and libs to sbin 
file (GLOB ecfiles *.py)
install(PROGRAMS
  ${ecfiles}
  DESTINATION sbin)

install(PROGRAMS poweruphba.sh DESTINATION sbin)
