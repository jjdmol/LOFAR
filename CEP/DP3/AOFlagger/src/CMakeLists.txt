INCLUDE_DIRECTORIES(${Casacore_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIRS} ${PTHREADS_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS})

LINK_DIRECTORIES(${LIBXML2_LIBRARY_DIRS} ${Casacore_LIBRARY_DIRS} ${LAPACK_LIBRARY_DIRS} ${BLAS_LIBRARY_DIRS} ${PTHREADS_LIBRARY_DIRS} ${BOOST_LIBRARY_DIRS})

SET (GUI_PLOT_FILES gui/plot/plot2d gui/plot/plotwidget gui/plot/verticalplotscale)
SET (GUI_FILES gui/application gui/complexplaneplotwindow gui/editstrategywindow gui/gotowindow gui/highlightwindow gui/imageplanewindow gui/imagewidget gui/msoptionwindow gui/mswindow gui/newstrategyactionframe gui/plotframe gui/progresswindow gui/timefrequencywidget gui/zoomwindow ${GUI_PLOT_FILES})
SET (LOFAR_FILES lofar/LofarColumn.cc lofar/LofarStMan.cc lofar/Register.cc )
SET (MSIO_FILES msio/colormap msio/fitsfile msio/image2d msio/mask2d msio/measurementset msio/pngfile msio/samplerow msio/segmentedimage msio/stokesimager msio/timefrequencydata msio/timefrequencyimager)
SET (RFI_STRATEGY_FILES rfi/strategy/actionfactory rfi/strategy/changeresolutionaction rfi/strategy/fitsimageset rfi/strategy/foreachbaselineaction rfi/strategy/foreachmsaction rfi/strategy/frequencyselectionaction rfi/strategy/fringestopaction rfi/strategy/imageraction rfi/strategy/imageset rfi/strategy/loadflagsaction rfi/strategy/loadimageaction rfi/strategy/msimageset rfi/strategy/plotaction rfi/strategy/slidingwindowfitaction rfi/strategy/statisticalflagaction rfi/strategy/strategy rfi/strategy/svdaction rfi/strategy/timeselectionaction rfi/strategy/writeflagsaction rfi/strategy/xmlreader rfi/strategy/xmlwriter)
SET (RFI_FILES rfi/antennaflagcountplot rfi/frequencyflagcountplot rfi/frequencypowerplot rfi/fringestoppingfitter rfi/fringetestcreater rfi/imagetile rfi/localfitmethod rfi/methoditerator rfi/mitigationtester rfi/morphology rfi/rfiplots rfi/rfistatistics rfi/sinusfitter rfi/statisticalflagger rfi/svdmitigater rfi/thresholdconfig rfi/thresholdmitigater rfi/thresholdtools rfi/tiledimage rfi/timeflagcountplot ${RFI_STRATEGY_FILES})
SET (IMAGING_FILES imaging/uvimager imaging/model)
SET (UTIL_FILES util/ffttools util/integerdomain util/multiplot util/plot util/rng util/statwriter util/stopwatch)

#SET (CASA_LIBS casa_ms casa_measures casa_tables casa_scimath casa_scimath_f casa_casa)
SET (CASA_LIBS ${CASA_LIBRARIES})

lofar_add_library(rfiengine ${IMAGING_FILES} ${LOFAR_FILES} ${MSIO_FILES} ${RFI_FILES} ${UTIL_FILES})

set(GENERIC_LIBRARIES rfiengine ${BOOST_LIBRARIES} ${CASA_LIBS} cfitsio dl ${FFTW3_LIBRARIES} gsl gslcblas m nsl png util z ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES} ${PTHREADS_LIBRARIES} xml2 ${GTKMM_LIBRARIES})

IF(GTKMM_FOUND)
  INCLUDE_DIRECTORIES(${GTKMM_INCLUDE_DIRS})
  LINK_DIRECTORIES(${GTKMM_LIBRARY_DIRS})

  lofar_add_executable(rfigui rfigui ${GUI_FILES})
  TARGET_LINK_LIBRARIES(rfigui ${GENERIC_LIBRARIES})
  SET_TARGET_PROPERTIES(rfigui PROPERTIES LINKER_LANGUAGE C)

  install(PROGRAMS rfigui DESTINATION bin)
ELSE()
  message(WARNING "The graphical user interface library GTKMM was not found; rfigui will not be compiled.")
ENDIF(GTKMM_FOUND)

lofar_add_executable(rficonsole rficonsole)
# f2c
TARGET_LINK_LIBRARIES(rficonsole ${GENERIC_LIBRARIES})
SET_TARGET_PROPERTIES(rficonsole PROPERTIES LINKER_LANGUAGE C)

lofar_add_executable(rfistrategy rfistrategy)
TARGET_LINK_LIBRARIES(rfistrategy ${GENERIC_LIBRARIES})
SET_TARGET_PROPERTIES(rfistrategy PROPERTIES LINKER_LANGUAGE C)

install(PROGRAMS
  rfistrategy
  rficonsole
  DESTINATION bin)
