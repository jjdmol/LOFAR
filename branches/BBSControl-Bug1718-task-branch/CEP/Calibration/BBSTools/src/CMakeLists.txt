# $Id: CMakeLists.txt 18231 2011-06-09 14:46:14Z duscha $

include(LofarPackageVersion)

# Because we're using dlopen()
#target_link_libraries(bbscontrol ${CMAKE_DL_LIBS})

lofar_add_library(progress
	Progress.cc
)

#lofar_find_package(PQXX REQUIRED)
#lofar_find_package(Boost REQUIRED)

lofar_add_bin_program(addUV2MS addUV2MS.cc)
lofar_add_bin_program(BBSProgress BBSProgress.cc progress)
