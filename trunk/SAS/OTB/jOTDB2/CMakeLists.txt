# $Id$

lofar_package(jOTDB2 1.0 DEPENDS Common OTDB)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(PQXX REQUIRED)

# Don't use lofar_find_package(JNI), because we don't need to link in the
# JNI libraries; we only need the JNI header files.
find_package(JNI)
include_directories(${JNI_INCLUDE_DIRS})

add_subdirectory(include/jOTDB2)
add_subdirectory(src)
#add_subdirectory(test)
