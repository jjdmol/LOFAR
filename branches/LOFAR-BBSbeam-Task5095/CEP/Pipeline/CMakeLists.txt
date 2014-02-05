# $Id$

lofar_add_package(Pipeline-Framework framework)
lofar_add_package(Pipeline-Recipes recipes)
#lofar_add_package(Pipeline-MAC mac)

add_subdirectory(test)

INSTALL(DIRECTORY DESTINATION var/run/pipeline)
INSTALL(DIRECTORY DESTINATION var/log)
