# $Id$

# All env. control python scripts and libs to sbin 
file (GLOB ecfiles *.py)

lofar_add_sbin_scripts(
  ${ecfiles}
  poweruphba.sh)
