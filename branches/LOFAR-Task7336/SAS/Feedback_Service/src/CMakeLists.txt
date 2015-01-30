# $Id: CMakeLists.txt 30676 2015-01-07 14:09:34Z mol $

include(LofarPackageVersion)

#@lofar_add_bin_program(FeedbackService Package__Version.cc Feedback.cc FeedbackMain.cc)
#lofar_add_bin_program(versionfeedback versionfeedback.cc)

install(FILES FeedbackService.conf DESTINATION etc)
