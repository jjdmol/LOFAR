# $Id$

include(LofarPackageVersion)

set(GUI_PLOT_FILES
  gui/plot/horizontalplotscale.cpp
  gui/plot/plot2d.cpp
  gui/plot/plotwidget.cpp
  gui/plot/verticalplotscale.cpp)

set(GUI_FILES
  gui/application.cpp
  gui/complexplaneplotwindow.cpp
  gui/editstrategywindow.cpp
  gui/gotowindow gui/highlightwindow.cpp
  gui/imageplanewindow gui/imagewidget.cpp
  gui/msoptionwindow.cpp
  gui/mswindow.cpp
  gui/newstrategyactionframe.cpp
  gui/plotframe.cpp
  gui/progresswindow.cpp
  gui/timefrequencywidget.cpp
  gui/zoomwindow.cpp
  ${GUI_PLOT_FILES})

set(MSIO_FILES
  msio/baselinematrixloader.cpp
  msio/colormap.cpp
  msio/fitsfile.cpp
  msio/image2d.cpp
  msio/mask2d.cpp
  msio/measurementset.cpp
  msio/pngfile.cpp
  msio/samplerow.cpp
  msio/segmentedimage.cpp
  msio/stokesimager.cpp
  msio/timefrequencydata.cpp
  msio/timefrequencyimager.cpp)

set(RFI_STRATEGY_FILES
  rfi/strategy/actionfactory.cpp
  rfi/strategy/changeresolutionaction.cpp
  rfi/strategy/fitsimageset.cpp
  rfi/strategy/foreachbaselineaction.cpp
  rfi/strategy/foreachmsaction.cpp
  rfi/strategy/frequencyselectionaction.cpp
  rfi/strategy/fringestopaction.cpp
  rfi/strategy/imageraction.cpp
  rfi/strategy/imageset.cpp
  rfi/strategy/loadflagsaction.cpp
  rfi/strategy/loadimageaction.cpp
  rfi/strategy/msimageset.cpp
  rfi/strategy/plotaction.cpp
  rfi/strategy/slidingwindowfitaction.cpp
  rfi/strategy/statisticalflagaction.cpp
  rfi/strategy/strategy.cpp
  rfi/strategy/svdaction.cpp
  rfi/strategy/timeselectionaction.cpp
  rfi/strategy/writeflagsaction.cpp
  rfi/strategy/xmlreader.cpp
  rfi/strategy/xmlwriter.cpp)

set(RFI_FILES
  rfi/antennaflagcountplot.cpp
  rfi/frequencyflagcountplot.cpp
  rfi/frequencypowerplot.cpp
  rfi/fringestoppingfitter.cpp
  rfi/fringetestcreater.cpp
  rfi/imagetile.cpp
  rfi/localfitmethod.cpp
  rfi/methoditerator.cpp
  rfi/mitigationtester.cpp
  rfi/morphology.cpp
  rfi/rfiplots.cpp
  rfi/rfistatistics.cpp
  rfi/sinusfitter.cpp
  rfi/statisticalflagger.cpp
  rfi/svdmitigater.cpp
  rfi/thresholdconfig.cpp
  rfi/thresholdmitigater.cpp
  rfi/thresholdtools.cpp
  rfi/tiledimage.cpp
  rfi/timeflagcountplot.cpp
  rfi/timefrequencystatistics.cpp
  ${RFI_STRATEGY_FILES})

set(IMAGING_FILES
  imaging/uvimager.cpp
  imaging/model.cpp
  imaging/fourproductcorrelatortester.cpp)

set(UTIL_FILES
  util/ffttools.cpp
  util/integerdomain.cpp
  util/multiplot.cpp
  util/plot.cpp
  util/rng.cpp
  util/statwriter.cpp
  util/stopwatch.cpp)

lofar_add_library(rfiengine
  ${IMAGING_FILES} ${MSIO_FILES} ${RFI_FILES} ${UTIL_FILES})

if(GTKMM_FOUND)
  include_directories(${GTKMM_INCLUDE_DIRS})
  lofar_add_bin_program(rfigui rfigui.cpp ${GUI_FILES})
  target_link_libraries(rfigui ${GTKMM_LIBRARIES})
else()
  message(WARNING " The graphical user interface library GTKMM was not found; rfigui will not be compiled.")
endif(GTKMM_FOUND)

lofar_add_bin_program(rficonsole rficonsole.cpp)
lofar_add_bin_program(rfistrategy rfistrategy.cpp)
lofar_add_bin_program(msinfo msinfo.cpp)
lofar_add_bin_program(colormapper colormapper.cpp)
lofar_add_bin_program(versionaoflagger versionaoflagger.cc Package__Version.cc)
