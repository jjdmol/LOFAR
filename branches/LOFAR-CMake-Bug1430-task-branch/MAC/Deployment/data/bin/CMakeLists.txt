# $Id$

# Create dummy library. This is just a kludge to pass all the required link
# libraries to the sbin programs.
lofar_add_library(deployment dummy.cc)

lofar_add_sbin_program(load_OTDB_comps load_OTDB_comps.cc)
lofar_add_sbin_program(load_PICtree load_PICtree.cc)
lofar_add_sbin_program(PVSS2SAS PVSS2SAS.cc)
