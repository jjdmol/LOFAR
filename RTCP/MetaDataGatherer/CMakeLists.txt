# $Id: CMakeLists.txt 22563 2012-10-30 18:50:15Z mol $

lofar_package(MetaDataGatherer 1.0 DEPENDS Common Stream OTDB Interface Storage)

include(LofarFindPackage)
lofar_find_package(Casacore COMPONENTS casa ms tables REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
