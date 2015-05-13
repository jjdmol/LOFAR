# $Id: CMakeLists.txt 30676 2015-01-07 14:09:34Z mol $

include(LofarPackageVersion)

lofar_add_library(sas_feedback Package__Version.cc)

lofar_add_bin_program(versionsas_feedback versionsas_feedback.cc)
lofar_add_bin_program(FeedbackService Feedback.cc FeedbackMain.cc)

install(FILES FeedbackService.conf DESTINATION etc)
