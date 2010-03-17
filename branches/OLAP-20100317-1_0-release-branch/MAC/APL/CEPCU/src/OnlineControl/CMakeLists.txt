# $Id$

# Add current source directory to -I path. This is needed because OnlineControl
# uses angle brackets for internal header files, instead of quotes.

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

lofar_add_bin_program(OnlineControl
  OnlineControlMain.cc
  OnlineControl.cc
  CEPApplMgr.cc)
