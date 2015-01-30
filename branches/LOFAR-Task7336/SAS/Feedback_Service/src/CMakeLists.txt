# $Id: CMakeLists.txt 30676 2015-01-07 14:09:34Z mol $

include(LofarPackageVersion)

lofar_add_library(feedbackservice Package__Version.cc)

lofar_add_bin_program(versionsas_feedback versionsas_feedback.cc)
lofar_add_bin_program(FeedbackService Feedback.cc FeedbackMain.cc)
#lofar_add_bin_program(FeedbackService Package__Version.cc Feedback.cc FeedbackMain.cc)

install(PROGRAMS FeedbackService DESTINATION bin)
install(FILES FeedbackService.conf DESTINATION etc)
