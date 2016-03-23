# $Id: CMakeLists.txt 30355 2014-11-04 13:46:05Z loose $

lofar_package(QPIDInfrastructure 0.1)

include(PythonInstall)

add_subdirectory(lib)
add_subdirectory(bin)
# add_subdirectory(sql)
# add_subdirectory(test)
