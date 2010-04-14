# $Id: CMakeLists.txt 14714 2009-12-21 13:32:38Z loose $

lofar_package(jOTDB3 1.0 DEPENDS Common OTDB)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS date_time)
lofar_find_package(PQXX REQUIRED)

# Don't use lofar_find_package(JNI), because we don't need to link in the
# JNI libraries; we only need the JNI header files.
find_package(JNI)
if(JNI_INCLUDE_DIRS)
  include_directories(${JNI_INCLUDE_DIRS})
endif(JNI_INCLUDE_DIRS)

add_subdirectory(include/jOTDB3)
add_subdirectory(src)
#add_subdirectory(test)
