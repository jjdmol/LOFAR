# $Id$

include(LofarFindPackage)
include(LofarSphinx)

lofar_package(ExpIon 1.0 DEPENDS pyparameterset pyparmdb)

lofar_find_package(Pyrap REQUIRED)
lofar_find_package(Boost REQUIRED COMPONENTS python thread)
lofar_find_package(Casacore REQUIRED COMPONENTS scimath)

add_subdirectory(src)

lofar_add_sphinx_doc(sphinx-doc)
