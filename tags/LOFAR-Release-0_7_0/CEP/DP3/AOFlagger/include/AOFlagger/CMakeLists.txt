# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

install(FILES
  baseexception.h
  blaswrap.h
  cleaner.h
  configuration.h
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
  gui/imagewidget.h
  gui/imageplanewindow.h
  gui/imagepropertieswindow.h
  gui/msoptionwindow.h
  gui/mswindow.h
  gui/plotframe.h
  gui/progresswindow.h
  gui/rawoptionwindow.h
  DESTINATION include/${PACKAGE_NAME}/gui)

install(FILES
  gui/plot/dimension.h
  gui/plot/horizontalplotscale.h
  gui/plot/plot2d.h
  gui/plot/plot2dpointset.h
  gui/plot/plotable.h
  gui/plot/plotwidget.h
  gui/plot/system.h
  gui/plot/tickset.h
  gui/plot/verticalplotscale.h
  DESTINATION include/${PACKAGE_NAME}/gui/plot)

install(FILES
  gui/strategyframes/baselineselectionframe.h
  gui/strategyframes/changeresolutionframe.h
  gui/strategyframes/cutareaframe.h
  gui/strategyframes/foreachbaselineframe.h
  gui/strategyframes/foreachmsframe.h
  gui/strategyframes/foreachpolarisationframe.h
  gui/strategyframes/fringestoppingframe.h
  gui/strategyframes/iterationframe.h
  gui/strategyframes/plotframe.h
  gui/strategyframes/setflaggingframe.h
  gui/strategyframes/setimageframe.h
  gui/strategyframes/slidingwindowfitframe.h
  gui/strategyframes/spatialcompositionframe.h
  gui/strategyframes/statisticalflaggingframe.h
  gui/strategyframes/svdframe.h
  gui/strategyframes/timeconvolutionframe.h
  gui/strategyframes/timeselectionframe.h
  gui/strategyframes/uvprojectframe.h
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
  msio/baselinereader.h
  msio/baselinematrixloader.h
  msio/colormap.h
  msio/date.h
  msio/directbaselinereader.h
  msio/fitsfile.h
  msio/image2d.h
  msio/indirectbaselinereader.h
  msio/mask2d.h
  msio/measurementset.h
  msio/pngfile.h
  msio/rspreader.h
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
  strategy/actions/action.h
  strategy/actions/adapter.h
  strategy/actions/changeresolutionaction.h
  strategy/actions/combineflagresultsaction.h
  strategy/actions/directionalcleanaction.h
  strategy/actions/foreachbaselineaction.h
  strategy/actions/foreachcomplexcomponentaction.h
  strategy/actions/foreachmsaction.h
  strategy/actions/foreachpolarisationaction.h
  strategy/actions/fouriertransformaction.h
  strategy/actions/frequencyconvolutionaction.h
  strategy/actions/frequencyselectionaction.h
  strategy/actions/fringestopaction.h
  strategy/actions/imageraction.h
  strategy/actions/iterationaction.h
  strategy/actions/plotaction.h
  strategy/actions/setflaggingaction.h
  strategy/actions/setimageaction.h
  strategy/actions/slidingwindowfitaction.h
  strategy/actions/slidingwindowfitparameters.h
  strategy/actions/statisticalflagaction.h
  strategy/actions/strategyaction.h
  strategy/actions/svdaction.h
  strategy/actions/timeselectionaction.h
  strategy/actions/uvprojectaction.h
  strategy/actions/writedataaction.h
  strategy/actions/writeflagsaction.h
  DESTINATION include/${PACKAGE_NAME}/strategy/actions)

install(FILES
  strategy/algorithms/fringestoppingfitter.h
  strategy/algorithms/fringetestcreater.h
  strategy/algorithms/localfitmethod.h
  strategy/algorithms/medianwindow.h
  strategy/algorithms/methoditerator.h
  strategy/algorithms/mitigationtester.h
  strategy/algorithms/morphology.h
  strategy/algorithms/rfistatistics.h
  strategy/algorithms/sinusfitter.h
  strategy/algorithms/statisticalflagger.h
  strategy/algorithms/surfacefitmethod.h
  strategy/algorithms/svdmitigater.h
  strategy/algorithms/thresholdconfig.h
  strategy/algorithms/thresholdmitigater.h
  strategy/algorithms/thresholdtools.h
  strategy/algorithms/timefrequencystatistics.h
  strategy/algorithms/types.h
  strategy/algorithms/uvprojection.h
  DESTINATION include/${PACKAGE_NAME}/strategy/algorithms)

install(FILES
  strategy/plots/antennaflagcountplot.h
  strategy/plots/frequencyflagcountplot.h
  strategy/plots/frequencypowerplot.h
  strategy/plots/rfiplots.h
  strategy/plots/timeflagcountplot.h
  DESTINATION include/${PACKAGE_NAME}/strategy/plots)

install(FILES
  strategy/control/actionblock.h
  strategy/control/actioncontainer.h
  strategy/control/actionfactory.h
  strategy/control/artifactset.h
  strategy/control/strategyiterator.h
  strategy/control/strategyreader.h
  strategy/control/strategywriter.h
  strategy/control/types.h
  DESTINATION include/${PACKAGE_NAME}/strategy/control)

install(FILES
  strategy/imagesets/bandcombinedset.h
  strategy/imagesets/fitsimageset.h
  strategy/imagesets/imageset.h
  strategy/imagesets/msimageset.h
  strategy/imagesets/rspimageset.h
  DESTINATION include/${PACKAGE_NAME}/strategy/imagesets)

install(FILES
	ref/copyallscript.h
	ref/reffile.h
	ref/reffileentry.h
	ref/reffileexception.h
  DESTINATION include/${PACKAGE_NAME}/ref)
