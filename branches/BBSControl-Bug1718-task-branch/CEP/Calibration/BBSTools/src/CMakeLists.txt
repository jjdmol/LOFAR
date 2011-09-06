# $Id: CMakeLists.txt 18231 2011-06-09 14:46:14Z duscha $

include(LofarPackageVersion)

lofar_add_library(progress
	Package__Version.cc
	Progress.cc)

#lofar_find_package(PQXX REQUIRED)
#lofar_find_package(Boost REQUIRED)

lofar_add_bin_program(addUV2MS addUV2MS.cc)

#lofar_add_bin_program(BBSProgress BBSProgress.cc progress)
#lofar_add_bin_program(BBSProgress BBSProgress.cc)
