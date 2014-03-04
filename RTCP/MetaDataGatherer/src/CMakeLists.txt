# $Id$

include(LofarPackageVersion)

lofar_add_library(metadatagatherer 
  Package__Version.cc)

install(FILES
  FinalMetaDataGatherer.log_prop
  DESTINATION etc)

lofar_add_bin_program(FinalMetaDataGatherer FinalMetaDataGatherer.cc)
lofar_add_bin_program(failedtilesinfo failedtilesinfo.cc)
