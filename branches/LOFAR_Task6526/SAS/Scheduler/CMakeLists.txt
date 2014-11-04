# $Id$

lofar_package(Scheduler 1.0)

# Search for Qt4 and setup CMake environment for it
find_package(Qt4 COMPONENTS QtCore QtGui QtSql REQUIRED)
include(${QT_USE_FILE})

# Add current source- and binary directory to include path
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(scheduler_SOURCES
  main.cpp
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
  qcustomplot.cpp
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
  schedulersettings.cpp
  schedulesettingsdialog.cpp
  scheduletabledelegate.cpp
  shifttasksdialog.cpp
  SpinBox.cpp
  statehistorydialog.cpp
  station.cpp
  stationlistwidget.cpp
  stationtask.cpp
  stationtreewidget.cpp
  Storage.cpp
  StorageNode.cpp
  storageplot.cpp
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

set(scheduler_HEADERS
#  Angle.h
#  astrodate.h
#  astrodatetime.h
#  astrotime.h
#  blocksize.h
#  calibrationpipeline.h
  CheckBox.h
  ComboBox.h
  conflictdialog.h
  Controller.h
#  DataHandler.h
#  DataMonitorConnection.h
  dataslotdialog.h
#  DataTreeWidgetItem.h
  DateEdit.h
  DateTimeEdit.h
#  demixingsettings.h
  digitalbeamdialog.h
#  DigitalBeam.h
  doublespinbox.h
#  FileUtils.h
#  GraphicCurrentTimeLine.h
  GraphicResourceScene.h
#  GraphicStationTaskLine.h
  graphicstoragescene.h
#  GraphicStorageTimeLine.h
  GraphicTask.h
#  GraphicTimeLine.h
#  imagingpipeline.h
  LineEdit.h
  ListWidget.h
#  lofar_scheduler.h
#  lofar_utils.h
#  longbaselinepipeline.h
#  neighboursolution.h
#  observation.h
#  OTDBnode.h
#  OTDBtree.h
  parsettreeviewer.h
#  pipeline.h
  publishdialog.h
#  pulsarpipeline.h
  qcustomplot.h
  qlofardatamodel.h
  redistributetasksdialog.h
  sasconnectdialog.h
#  SASConnection.h
  sasprogressdialog.h
  sasstatusdialog.h
  sasuploaddialog.h
#  schedulerdatablock.h
#  schedulerdata.h
  schedulergui.h
  Scheduler.h
#  schedulersettings.h
  schedulesettingsdialog.h
  scheduletabledelegate.h
  shifttasksdialog.h
  SpinBox.h
  statehistorydialog.h
#  station.h
  stationlistwidget.h
#  stationtask.h
  stationtreewidget.h
#  storage_definitions.h
#  Storage.h
#  StorageNode.h
  storageplot.h
  tablecolumnselectdialog.h
  tableview.h
  taskcopydialog.h
  taskdialog.h
#  task.h
#  taskstorage.h
  thrashbin.h
  tiedarraybeamdialog.h
#  TiedArrayBeam.h
  TimeEdit.h)

set(scheduler_FORMS 
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
  storageresourceview.ui
  tablecolumnselectdialog.ui
  taskcopydialog.ui
  taskdialog.ui
  thrashbin.ui
  tiedarraybeamdialog.ui)

set(scheduler_RESOURCES
  scheduler_resources.qrc)

qt4_wrap_cpp(scheduler_HEADERS_MOC ${scheduler_HEADERS})
qt4_wrap_ui(scheduler_FORMS_HEADERS ${scheduler_FORMS})
qt4_add_resources(scheduler_RESOURCES_RCC ${scheduler_RESOURCES})

add_executable(scheduler
  ${scheduler_SOURCES} 
  ${scheduler_HEADERS_MOC} 
  ${scheduler_FORMS_HEADERS} 
  ${scheduler_RESOURCES_RCC})

target_link_libraries(scheduler ${QT_LIBRARIES})
