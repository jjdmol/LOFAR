
#make_package_version (${LCS_SOURCE_DIR}/Common/src TRUE TRUE)

file (GLOB libcommon_SOURCES *.cc Net/*.cc)

add_library (common ${libcommon_SOURCES})

install (
  TARGETS common
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

