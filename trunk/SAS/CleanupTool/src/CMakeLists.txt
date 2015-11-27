# $Id: CMakeLists.txt 31203 2015-03-13 13:41:45Z schoenmakers $

# Create symbolic link to include directory.

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

set(cleanuptool_SRCS
astrodate.cpp
astrodatetime.cpp
astrotime.cpp
cepcleanmainwindow.cpp
cepdeletedialog.cpp
Controller.cpp
DataHandler.cpp
DataMonitorConnection.cpp
dataproductinfodialog.cpp
DataTreeWidgetItem.cpp
debug_lofar.cpp
FileUtils.cpp
filterdatadialog.cpp
lofar_utils.cpp
main.cpp
OTDBnode.cpp
OTDBtree.cpp
parsettreeviewer.cpp
pieview.cpp
ProjectDataProducts.cpp
qrc_scheduler_resources.cpp
sasconnectdialog.cpp
SASConnection.cpp
sasprogressdialog.cpp
schedulersettings.cpp
schedulesettingsdialog.cpp
TaskDataProducts.cpp)


set(cleanuptool_MOC_HDRS
astrodate.h
astrodatetime.h
astrotime.h
cepcleanmainwindow.h
cepdeletedialog.h
Controller.h
DataHandler.h
DataMonitorConnection.h
dataproductinfodialog.h
DataTreeWidgetItem.h
FileUtils.h
filterdatadialog.h
lofar_scheduler.h
lofar_utils.h
OTDBnode.h
OTDBtree.h
parsettreeviewer.h
pieview.h
ProjectDataProducts.h
sasconnectdialog.h
SASConnection.h
sasprogressdialog.h
schedulersettings.h
schedulesettingsdialog.h
TaskDataProducts.h)


set(cleanuptool_UIS 
cepcleanmainwindow.ui
cepdeletedialog.ui
dataproductinfodialog.ui
filterdatadialog.ui
parsettreeviewer.ui
sasconnectdialog.ui
sasprogressdialog.ui
schedulesettingsdialog.ui)

qt4_wrap_cpp(cleanuptool_MOC_SRCS ${cleanuptool_MOC_HDRS})
qt4_wrap_ui(cleanuptool_UI_HDRS ${cleanuptool_UIS})

lofar_add_library(lofar_cleanuptool
  ${cleanuptool_SRCS} 
  ${cleanuptool_MOC_SRCS} 
  ${cleanuptool_UI_HDRS})

target_link_libraries(lofar_cleanuptool ${QT_LIBRARIES})

lofar_add_bin_program(cleanuptool
  main.cpp)


