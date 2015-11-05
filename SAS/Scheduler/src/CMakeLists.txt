# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

set(scheduler_SRCS
  Angle.cpp
  astrodate.cpp
  astrodatetime.cpp
  astrotime.cpp
  blocksize.cpp
  calibrationpipeline.cpp
  CheckBox.cpp
  ComboBox.cpp
  conflictdialog.cpp
  Controller.cpp
  DataHandler.cpp
  DataMonitorConnection.cpp
  dataslotdialog.cpp
  DataTreeWidgetItem.cpp
  DateEdit.cpp
  DateTimeEdit.cpp
  debug_lofar.cpp
  demixingsettings.cpp
  DigitalBeam.cpp
  digitalbeamdialog.cpp
  doublespinbox.cpp
  FileUtils.cpp
  GraphicCurrentTimeLine.cpp
  GraphicResourceScene.cpp
  GraphicStationTaskLine.cpp
  graphicstoragescene.cpp
  GraphicStorageTimeLine.cpp
  GraphicTask.cpp
  GraphicTimeLine.cpp
  imagingpipeline.cpp
  LineEdit.cpp
  ListWidget.cpp
  lofar_utils.cpp
  longbaselinepipeline.cpp
  neighboursolution.cpp
  observation.cpp
  OTDBnode.cpp
  OTDBtree.cpp
  parsettreeviewer.cpp
  pipeline.cpp
  publishdialog.cpp
  pulsarpipeline.cpp
  qlofardatamodel.cpp
  redistributetasksdialog.cpp
  sasconnectdialog.cpp
  SASConnection.cpp
  sasprogressdialog.cpp
  sasstatusdialog.cpp
  sasuploaddialog.cpp
  Scheduler.cpp
  schedulerdatablock.cpp
  schedulerdata.cpp
  schedulergui.cpp
  schedulerLib.cpp
  schedulersettings.cpp
  schedulesettingsdialog.cpp
  scheduletabledelegate.cpp
  shifttasksdialog.cpp
  signalhandler.cpp
  SpinBox.cpp
  statehistorydialog.cpp
  station.cpp
  stationlistwidget.cpp
  stationtask.cpp
  stationtreewidget.cpp
  Storage.cpp
  StorageNode.cpp
  tablecolumnselectdialog.cpp
  tableview.cpp
  taskcopydialog.cpp
  task.cpp
  taskdialog.cpp
  taskstorage.cpp
  thrashbin.cpp
  TiedArrayBeam.cpp
  tiedarraybeamdialog.cpp
  TimeEdit.cpp)

set(scheduler_MOC_HDRS
  CheckBox.h
  ComboBox.h
  conflictdialog.h
  Controller.h
  dataslotdialog.h
  DateEdit.h
  DateTimeEdit.h
  digitalbeamdialog.h
  doublespinbox.h
  GraphicResourceScene.h
  graphicstoragescene.h
  GraphicTask.h
  LineEdit.h
  ListWidget.h
  parsettreeviewer.h
  publishdialog.h
  qlofardatamodel.h
  redistributetasksdialog.h
  sasconnectdialog.h
  sasprogressdialog.h
  sasstatusdialog.h
  sasuploaddialog.h
  schedulergui.h
  Scheduler.h
  schedulesettingsdialog.h
  scheduletabledelegate.h
  shifttasksdialog.h
  signalhandler.h
  SpinBox.h
  statehistorydialog.h
  stationlistwidget.h
  stationtreewidget.h
  tablecolumnselectdialog.h
  tableview.h
  taskcopydialog.h
  taskdialog.h
  thrashbin.h
  tiedarraybeamdialog.h
  TimeEdit.h)

set(scheduler_UIS 
  conflictdialog.ui
  dataslotdialog.ui
  digitalbeamdialog.ui
  graphicstoragescene.ui
  parsettreeviewer.ui
  publishdialog.ui
  redistributetasksdialog.ui
  sasconnectdialog.ui
  sasprogressdialog.ui
  sasstatusdialog.ui
  sasuploaddialog.ui
  schedulergui.ui
  schedulesettingsdialog.ui
  shifttasksdialog.ui
  statehistorydialog.ui
  stationlistwidget.ui
  stationtreewidget.ui
  tablecolumnselectdialog.ui
  taskcopydialog.ui
  taskdialog.ui
  thrashbin.ui
  tiedarraybeamdialog.ui)

set(scheduler_RCS
  scheduler_resources.qrc)

qt4_wrap_cpp(scheduler_MOC_SRCS ${scheduler_MOC_HDRS})
qt4_wrap_ui(scheduler_UI_HDRS ${scheduler_UIS})
qt4_add_resources(scheduler_RC_SRCS ${scheduler_RCS})

lofar_add_library(lofar_scheduler
  ${scheduler_SRCS} 
  ${scheduler_MOC_SRCS} 
  ${scheduler_UI_HDRS})

target_link_libraries(lofar_scheduler ${QT_LIBRARIES})

lofar_add_bin_program(scheduler
  main.cpp
  ${scheduler_RC_SRCS})

