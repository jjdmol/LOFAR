# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

install(FILES
  baseexception.h
  blaswrap.h
  cleaner.h
  f2c.h
  types.h
  vectortypes.h
  DESTINATION include/${PACKAGE_NAME})

install(FILES
  gui/application.h
  gui/complexplaneplotwindow.h
  gui/editstrategywindow.h
  gui/gotowindow.h
  gui/highlightwindow.h
  gui/imageplanewindow.h
  gui/imagewidget.h
  gui/msoptionwindow.h
  gui/mswindow.h
  gui/newstrategyactionframe.h
  gui/plotframe.h
  gui/progresswindow.h
  gui/timefrequencywidget.h
  gui/zoomwindow.h
  DESTINATION include/${PACKAGE_NAME}/gui)

install(FILES
  gui/plot/dimension.h
  gui/plot/plot2d.h
  gui/plot/plot2dpointset.h
  gui/plot/plotable.h
  gui/plot/plotwidget.h
  gui/plot/system.h
  gui/plot/verticalnumericscale.h
  gui/plot/verticalplotscale.h
  DESTINATION include/${PACKAGE_NAME}/gui/plot)

install(FILES
  gui/strategyframes/changeresolutionframe.h
  gui/strategyframes/foreachbaselineframe.h
  gui/strategyframes/foreachmsframe.h
  gui/strategyframes/foreachpolarisationframe.h
  gui/strategyframes/fringestoppingframe.h
  gui/strategyframes/iterationframe.h
  gui/strategyframes/loadflaggingframe.h
  gui/strategyframes/loadimageframe.h
  gui/strategyframes/plotframe.h
  gui/strategyframes/setflaggingframe.h
  gui/strategyframes/setimageframe.h
  gui/strategyframes/slidingwindowfitframe.h
  gui/strategyframes/statisticalflaggingframe.h
  gui/strategyframes/svdframe.h
  gui/strategyframes/thresholdframe.h
  gui/strategyframes/timeselectionframe.h
  DESTINATION include/${PACKAGE_NAME}/gui/strategyframes)

install(FILES
  imaging/fourproductcorrelatortester.h
  imaging/model.h
  imaging/observatorium.h
  imaging/uvimager.h
  DESTINATION include/${PACKAGE_NAME}/imaging)

install(FILES
  msio/antennainfo.h
  msio/arraycolumniterator.h
  msio/baselinematrixloader.h
  msio/colormap.h
  msio/date.h
  msio/fitsfile.h
  msio/image2d.h
  msio/mask2d.h
  msio/measurementset.h
  msio/pngfile.h
  msio/samplerow.h
  msio/scalarcolumniterator.h
  msio/segmentedimage.h
  msio/spatialmatrixmetadata.h
  msio/stokesimager.h
  msio/timefrequencydata.h
  msio/timefrequencyimager.h
  msio/timefrequencymetadata.h
  msio/types.h
  DESTINATION include/${PACKAGE_NAME}/msio)

install(FILES
  rfi/antennaflagcountplot.h
  rfi/frequencyflagcountplot.h
  rfi/frequencypowerplot.h
  rfi/fringestoppingfitter.h
  rfi/fringetestcreater.h
  rfi/image.h
  rfi/imagetile.h
  rfi/localfitmethod.h
  rfi/medianwindow.h
  rfi/methoditerator.h
  rfi/mitigationtester.h
  rfi/morphology.h
  rfi/rfiplots.h
  rfi/rfistatistics.h
  rfi/sinusfitter.h
  rfi/statisticalflagger.h
  rfi/svdmitigater.h
  rfi/thresholdconfig.h
  rfi/thresholdmitigater.h
  rfi/thresholdtools.h
  rfi/tiledimage.h
  rfi/timeflagcountplot.h
  rfi/timefrequencystatistics.h
  rfi/types.h
  DESTINATION include/${PACKAGE_NAME}/rfi)

install(FILES
  rfi/strategy/actionblock.h
  rfi/strategy/actioncontainer.h
  rfi/strategy/actionfactory.h
  rfi/strategy/action.h
  rfi/strategy/adapter.h
  rfi/strategy/artifactset.h
  rfi/strategy/changeresolutionaction.h
  rfi/strategy/combineflagresults.h
  rfi/strategy/fitsimageset.h
  rfi/strategy/foreachbaselineaction.h
  rfi/strategy/foreachmsaction.h
  rfi/strategy/foreachpolarisationblock.h
  rfi/strategy/frequencyselectionaction.h
  rfi/strategy/fringestopaction.h
  rfi/strategy/imageraction.h
  rfi/strategy/imageset.h
  rfi/strategy/iterationblock.h
  rfi/strategy/loadflagsaction.h
  rfi/strategy/loadimageaction.h
  rfi/strategy/msimageset.h
  rfi/strategy/plotaction.h
  rfi/strategy/setflaggingaction.h
  rfi/strategy/setimageaction.h
  rfi/strategy/slidingwindowfitaction.h
  rfi/strategy/slidingwindowfitparameters.h
  rfi/strategy/statisticalflagaction.h
  rfi/strategy/strategy.h
  rfi/strategy/strategyiterator.h
  rfi/strategy/svdaction.h
  rfi/strategy/thresholdaction.h
  rfi/strategy/timeselectionaction.h
  rfi/strategy/types.h
  rfi/strategy/writeflagsaction.h
  rfi/strategy/xmlreader.h
  rfi/strategy/xmlwriter.h
  DESTINATION include/${PACKAGE_NAME}/rfi/strategy)
