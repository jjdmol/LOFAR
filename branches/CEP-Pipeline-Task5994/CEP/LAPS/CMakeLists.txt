# $Id$

lofar_add_package(Laps-GRIDInterface GRIDInterface)
lofar_add_package(Laps-ParsetCombiner ParsetCombiner)
lofar_add_package(Laps-DBToQDeamon DBToQDeamon)
lofar_add_package(Laps-QToPipeline QToPipeline)

#add_subdirectory(test)

INSTALL(DIRECTORY DESTINATION var/run/laps)

