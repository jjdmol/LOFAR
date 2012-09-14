# $Id$

include(LofarPackageVersion)

link_libraries(${BOOST_LIBRARIES})

set(CMAKE_CXX_FLAGS "-g -Wall -DNDEBUG -funroll-loops")

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)

# -march=native speeds up the flagger with about 6% - 7%, but is not supported prior gcc 4.2.
# Hence, only enable it if it is supported.
# Disabled setting of -march=native, because it breaks binary compatibility between CEP-I and CEP-II hardware.
#if(COMPILER_SUPPORTS_MARCH_NATIVE)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
#else()
#  message(STATUS " CXX compiler does not support -march=native : your code will not be optimized with -march=native.")
#  message(STATUS " This probably means your gcc is old ( < 4.2).")
#endif(COMPILER_SUPPORTS_MARCH_NATIVE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")

include(CheckIncludeFileCXX)

FIND_PATH(BOOST_ASIO_H_FOUND "boost/asio.hpp")
if(BOOST_ASIO_H_FOUND)
  message(STATUS "Boost ASIO library found.")
else()
  message(STATUS "Boost ASIO library not found, remote access of measurement sets disabled.")
endif(BOOST_ASIO_H_FOUND)

# To enable assembly .s files:
#enable_language(ASM-ATT)

if(GTKMM_FOUND)
  add_definitions(-DHAVE_GTKMM)
endif(GTKMM_FOUND)

if(GSL_FOUND)
  add_definitions(-DHAVE_GSL)
  link_libraries(${GSL_LIBRARIES})
endif(GSL_FOUND)

lofar_add_bin_program(rfihistory rfihistory.cpp)

lofar_add_bin_program(aorefscript aorefscript.cpp)

set(GUI_PLOT_FILES
  gui/plot/colorscale.cpp
  gui/plot/horizontalplotscale.cpp
  gui/plot/plot2d.cpp
  gui/plot/plotpropertieswindow.cpp
  gui/plot/plotwidget.cpp
  gui/plot/verticalplotscale.cpp)

set(GUI_FILES
  gui/antennamap/antenna.cpp
  gui/antennamap/antennamap.cpp
  gui/application.cpp
  gui/complexplaneplotwindow.cpp
  gui/editstrategywindow.cpp
  gui/gotowindow
  gui/highlightwindow.cpp
  gui/imagecomparisonwidget.cpp
  gui/imageplanewindow
  gui/imagepropertieswindow.cpp
  gui/imagewidget.cpp
  gui/msoptionwindow.cpp
  gui/mswindow.cpp
  gui/noisestatoptionwindow.cpp
  gui/plotframe.cpp
  gui/progresswindow.cpp
  gui/rawoptionwindow.cpp
  gui/tfstatoptionwindow.cpp
  gui/quality/datawindow.cpp
  gui/quality/histogrampage.cpp
  ${GUI_PLOT_FILES})

set(MSIO_FILES
  msio/baselinematrixloader.cpp
  msio/baselinereader.cpp
  msio/colormap.cpp
  msio/directbaselinereader.cpp
  msio/fitsfile.cpp
  msio/image2d.cpp
  msio/indirectbaselinereader.cpp
  msio/mask2d.cpp
  msio/measurementset.cpp
  msio/memorybaselinereader.cpp
  msio/pngfile.cpp
  msio/rspreader.cpp
  msio/samplerow.cpp
  msio/segmentedimage.cpp
  msio/sortedtimestepaccessor.cpp
  msio/spatialtimeloader.cpp
  msio/stokesimager.cpp
  msio/timefrequencydata.cpp
  msio/timefrequencyimager.cpp
  msio/timestepaccessor.cpp)
  
set(QUALITY_FILES
  quality/histogramtablesformatter.cpp
	quality/rayleighfitter.cpp
  quality/qualitytablesformatter.cpp)

set(REMOTEAO_FILES
  remote/server.cpp)

set(STRATEGY_ACTION_FILES
  strategy/actions/baselineselectionaction.cpp
  strategy/actions/changeresolutionaction.cpp
  strategy/actions/foreachbaselineaction.cpp
  strategy/actions/foreachmsaction.cpp
  strategy/actions/frequencyselectionaction.cpp
  strategy/actions/fringestopaction.cpp
	strategy/actions/highpassfilteraction.cpp
  strategy/actions/imageraction.cpp
  strategy/actions/normalizevarianceaction.cpp
  strategy/actions/plotaction.cpp
  strategy/actions/slidingwindowfitaction.cpp
  strategy/actions/spatialcompositionaction.cpp
  strategy/actions/statisticalflagaction.cpp
  strategy/actions/strategyaction.cpp
  strategy/actions/svdaction.cpp
  strategy/actions/timeconvolutionaction.cpp
  strategy/actions/timeselectionaction.cpp
  strategy/actions/writeflagsaction.cpp)

set(STRATEGY_ALGORITHMS_FILES
  strategy/algorithms/baselineselector.cpp
  strategy/algorithms/eigenvalue.cpp
  strategy/algorithms/fringestoppingfitter.cpp
  strategy/algorithms/fringetestcreater.cpp
	strategy/algorithms/highpassfilter.cpp
  strategy/algorithms/localfitmethod.cpp
  strategy/algorithms/methoditerator.cpp
  strategy/algorithms/mitigationtester.cpp
  strategy/algorithms/morphology.cpp
  strategy/algorithms/rfistatistics.cpp
  strategy/algorithms/sinusfitter.cpp
  strategy/algorithms/statisticalflagger.cpp
  strategy/algorithms/sumthreshold.cpp
  strategy/algorithms/svdmitigater.cpp
  strategy/algorithms/thresholdconfig.cpp
  strategy/algorithms/thresholdmitigater.cpp
  strategy/algorithms/thresholdtools.cpp
  strategy/algorithms/timefrequencystatistics.cpp
  strategy/plots/antennaflagcountplot.cpp
  strategy/plots/frequencyflagcountplot.cpp)

if(GTKMM_FOUND)
set(STRATEGY_PLOTS_FILES
  strategy/plots/frequencypowerplot.cpp
  strategy/plots/iterationsplot.cpp
  strategy/plots/rfiplots.cpp
  strategy/plots/timeflagcountplot.cpp
  util/multiplot.cpp)
set(QUALITY_FILES 
  ${QUALITY_FILES} 
  quality/histogramcollection.cpp)
else()
 set(STRATEGY_PLOTS_FILES)
endif(GTKMM_FOUND)

set(STRATEGY_CONTROL_FILES
  strategy/control/actionblock.cpp
  strategy/control/actionfactory.cpp
  strategy/control/strategyreader.cpp
  strategy/control/strategywriter.cpp)

set(STRATEGY_IMAGESETS_FILES
  strategy/imagesets/fitsimageset.cpp
  strategy/imagesets/imageset.cpp
  strategy/imagesets/msimageset.cpp
  strategy/imagesets/parmimageset.cpp)

set(STRATEGY_FILES
  ${STRATEGY_ACTION_FILES}
  ${STRATEGY_ALGORITHMS_FILES}
  ${STRATEGY_CONTROL_FILES}
  ${STRATEGY_IMAGESETS_FILES}
  ${STRATEGY_PLOTS_FILES})

set(IMAGING_FILES
  imaging/uvimager.cpp
  imaging/model.cpp
  imaging/fourproductcorrelatortester.cpp)

set(UTIL_FILES
  util/aologger.cpp
  util/compress.cpp
  util/ffttools.cpp
  util/integerdomain.cpp
  util/plot.cpp
  util/rng.cpp
  util/statwriter.cpp
  util/stopwatch.cpp)

if(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)
	set(REMOTE_FILES
		remote/client.cpp
		remote/server.cpp
		remote/serverconnection.cpp
		remote/processcommander.cpp
		remote/clusteredobservation.cpp)
else()
	set(REMOTE_FILES
		remote/clusteredobservation.cpp)
endif(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)

lofar_add_library(rfiengine
  Package__Version.cc
  ${IMAGING_FILES} ${MSIO_FILES} ${QUALITY_FILES} ${STRATEGY_FILES} ${UTIL_FILES})

if(GTKMM_FOUND)
  include_directories(${GTKMM_INCLUDE_DIRS})
  link_directories(${GTKMM_LIBDIR})
  
  lofar_add_bin_program(rfigui rfigui.cpp ${GUI_FILES})
  target_link_libraries(rfigui ${GTKMM_LIBRARIES})
  
	if(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)
		lofar_add_bin_program(aoqplot aoqplot.cpp
		${REMOTE_FILES}
		gui/plot/colorscale.cpp
		gui/plot/horizontalplotscale.cpp
		gui/plot/verticalplotscale.cpp
		gui/plot/plot2d.cpp
		gui/plot/plotpropertieswindow.cpp
		gui/quality/aoqplotwindow.cpp
		gui/quality/baselineplotpage.cpp
		gui/quality/datawindow.cpp
		gui/quality/grayscaleplotpage.cpp
		gui/quality/histogrampage.cpp
		gui/quality/timefrequencyplotpage.cpp
		gui/quality/twodimensionalplotpage.cpp
		gui/imagewidget.cpp
		gui/imagepropertieswindow.cpp
		)
		target_link_libraries(aoqplot ${GTKMM_LIBRARIES})
		lofar_add_bin_program(badstations badstations.cpp ${REMOTE_FILES})
		target_link_libraries(badstations ${GTKMM_LIBRARIES})
		lofar_add_bin_program(aofrequencyfilter aofrequencyfilter.cpp ${REMOTE_FILES})
		target_link_libraries(aofrequencyfilter ${GTKMM_LIBRARIES})
	endif(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)
else()
  message(WARNING " The graphical user interface library GTKMM was not found; rfigui and aoqplot will not be compiled.")
endif(GTKMM_FOUND)

lofar_add_bin_program(rficonsole rficonsole.cpp)
lofar_add_bin_program(rfistrategy rfistrategy.cpp)
lofar_add_bin_program(rfistatcollect rfistatcollect.cpp)
lofar_add_bin_program(aostats aostats.cpp)
lofar_add_bin_program(msinfo msinfo.cpp)
lofar_add_bin_program(ns2bbs ns2bbs.cpp)
lofar_add_bin_program(colormapper colormapper.cpp)
lofar_add_bin_program(versionaoflagger versionaoflagger.cc Package__Version.cc)
lofar_add_bin_program(imgzenith imgzenith.cpp imaging/zenithimager.cpp)

if(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)
  lofar_add_bin_program(aoquality aoquality.cpp ${REMOTE_FILES})
  target_link_libraries(aoquality ${SIGCXX_LIBRARIES})

	lofar_add_bin_program(aoremoteclient aoremoteclient.cpp
  ${REMOTE_FILES} )
  target_link_libraries(aoremoteclient ${SIGCXX_LIBRARIES})
endif(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)
