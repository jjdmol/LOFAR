/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <AOFlagger/gui/mswindow.h>

#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/inputdialog.h>

#include <AOFlagger/msio/baselinematrixloader.h>
#include <AOFlagger/msio/measurementset.h>
#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/timefrequencydata.h>
#include <AOFlagger/msio/segmentedimage.h>
#include <AOFlagger/msio/spatialmatrixmetadata.h>

#include <AOFlagger/rfi/strategy/artifactset.h>
#include <AOFlagger/rfi/strategy/msimageset.h>
#include <AOFlagger/rfi/strategy/spatialmsimageset.h>
#include <AOFlagger/rfi/strategy/strategy.h>

#include <AOFlagger/rfi/antennaflagcountplot.h>
#include <AOFlagger/rfi/mitigationtester.h>
#include <AOFlagger/rfi/morphology.h>
#include <AOFlagger/rfi/frequencyflagcountplot.h>
#include <AOFlagger/rfi/frequencypowerplot.h>
#include <AOFlagger/rfi/fringetestcreater.h>
#include <AOFlagger/rfi/fringestoppingfitter.h>
#include <AOFlagger/rfi/rfiplots.h>
#include <AOFlagger/rfi/rfistatistics.h>
#include <AOFlagger/rfi/svdmitigater.h>
#include <AOFlagger/rfi/thresholdtools.h>
#include <AOFlagger/rfi/timeflagcountplot.h>

#include <AOFlagger/util/plot.h>
#include <AOFlagger/util/multiplot.h>

#include <AOFlagger/gui/plot/plot2d.h>

#include <AOFlagger/gui/msoptionwindow.h>
#include <AOFlagger/gui/highlightwindow.h>
#include <AOFlagger/gui/complexplaneplotwindow.h>
#include <AOFlagger/gui/editstrategywindow.h>
#include <AOFlagger/gui/gotowindow.h>
#include <AOFlagger/gui/imageplanewindow.h>
#include <AOFlagger/gui/progresswindow.h>
#include <AOFlagger/gui/zoomwindow.h>

#include <AOFlagger/imaging/model.h>
#include <AOFlagger/imaging/observatorium.h>
#include <AOFlagger/imaging/fourproductcorrelatortester.h>

#include <iostream>

MSWindow::MSWindow() : _imagePlaneWindow(0), _statistics(new RFIStatistics()),  _imageSet(0), _imageSetIndex(0), _gaussianTestSets(true), _spatialMetaData(0)
{
	createToolbar();

	_mainVBox.pack_start(_timeFrequencyWidget);
	_timeFrequencyWidget.add_events(Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON_PRESS_MASK);
	_timeFrequencyWidget.signal_motion_notify_event().connect(sigc::mem_fun(*this, &MSWindow::onTFWidgetMotion));
	_timeFrequencyWidget.signal_button_release_event().connect(sigc::mem_fun(*this, &MSWindow::onTFWidgetButtonReleased));
	_timeFrequencyWidget.Init();
	_timeFrequencyWidget.show();

	_mainVBox.pack_end(_statusbar, Gtk::PACK_SHRINK);
	_statusbar.push("Ready. For suggestions, contact offringa@astro.rug.nl .");
	_statusbar.show();

	add(_mainVBox);
	_mainVBox.show();

	set_default_size(800,600);

	_strategy = rfiStrategy::Strategy::CreateDefaultSingleStrategy();
	_imagePlaneWindow = new ImagePlaneWindow();
}

MSWindow::~MSWindow()
{
	for(std::vector<Gtk::Window*>::iterator i=_subWindows.begin();
	    i!=_subWindows.end(); ++i)
	{
		delete *i;
	}
	delete _statistics;
	delete _strategy;
	if(HasImageSet())
	{
		delete _imageSetIndex;
		delete _imageSet;
	}
	if(_spatialMetaData != 0)
		delete _spatialMetaData;
}

void MSWindow::onActionDirectoryOpen()
{
  Gtk::FileChooserDialog dialog("Select a measurement set",
          Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  int result = dialog.run();

  if(result == Gtk::RESPONSE_OK)
	{
		Gtk::Window *window = new MSOptionWindow(*this, dialog.get_filename());
		window->show();
		_subWindows.push_back(window);
	}
}

void MSWindow::onActionDirectoryOpenForSpatial()
{
  Gtk::FileChooserDialog dialog("Select a measurement set",
          Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  int result = dialog.run();

  if(result == Gtk::RESPONSE_OK)
	{
		SetImageSet(new rfiStrategy::SpatialMSImageSet(dialog.get_filename()));
	}
}

void MSWindow::onActionFileOpen()
{

  Gtk::FileChooserDialog dialog("Select a measurement set");
  dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  int result = dialog.run();

  if(result == Gtk::RESPONSE_OK)
	{
		Gtk::Window *window = new MSOptionWindow(*this, dialog.get_filename());
		window->show();
		_subWindows.push_back(window);
	}
}

void MSWindow::onToggleFlags()
{
	_timeFrequencyWidget.SetShowOriginalFlagging(_originalFlagsButton->get_active());
	_timeFrequencyWidget.SetShowAlternativeFlagging(_altFlagsButton->get_active());
	_timeFrequencyWidget.Update();
}

void MSWindow::onToggleMap()
{
	TimeFrequencyWidget::TFMap colorMap = TimeFrequencyWidget::TFBWMap;
	if(_mapLogButton->get_active())
		colorMap = TimeFrequencyWidget::TFLogMap;
	else if(_mapColorButton->get_active())
		colorMap = TimeFrequencyWidget::TFColorMap;
	_timeFrequencyWidget.SetColorMap(colorMap);
	_timeFrequencyWidget.Update();
}

void MSWindow::loadCurrentTFData()
{
	if(_imageSet != 0) {
		try {
			_metaData = _imageSet->LoadMetaData(*_imageSetIndex);
			TimeFrequencyData *data = _imageSet->LoadData(*_imageSetIndex);
			_timeFrequencyWidget.SetNewData(*data, _metaData);
			delete data;
			if(_spatialMetaData != 0)
			{
				delete _spatialMetaData;
				_spatialMetaData = 0;
			}
			if(dynamic_cast<rfiStrategy::SpatialMSImageSet*>(_imageSet) != 0)
			{
				_spatialMetaData = new SpatialMatrixMetaData(static_cast<rfiStrategy::SpatialMSImageSet*>(_imageSet)->SpatialMetaData());
			}
			_timeFrequencyWidget.Update();
			_statusbar.pop();
			_statusbar.push(std::string() + _imageSet->Name() + ": " + _imageSetIndex->Description());
		} catch(std::exception &e)
		{
			showError(e.what());
		}
	}
}

void MSWindow::onLoadPrevious()
{
	if(_imageSet != 0) {
		_imageSetIndex->Previous();
		loadCurrentTFData();
	}
}

void MSWindow::onLoadNext()
{
	if(_imageSet != 0) {
		_imageSetIndex->Next();
		loadCurrentTFData();
	}
}

void MSWindow::onLoadLargeStepPrevious()
{
	if(_imageSet != 0) {
		_imageSetIndex->LargeStepPrevious();
		loadCurrentTFData();
	}
}

void MSWindow::onLoadLargeStepNext()
{
	if(_imageSet != 0) {
		_imageSetIndex->LargeStepNext();
		loadCurrentTFData();
	}
}

void MSWindow::onEditStrategyPressed()
{
	Gtk::Window *editStrategy = new EditStrategyWindow(*this);
	editStrategy->show();
	_subWindows.push_back(editStrategy);
}

void MSWindow::onExecuteStrategyPressed()
{
	ProgressWindow *progressWindow = new ProgressWindow(*this);
	progressWindow->show();
	_subWindows.push_back(progressWindow);

	rfiStrategy::ArtifactSet artifacts(&_ioMutex);

	artifacts.SetAntennaFlagCountPlot(new AntennaFlagCountPlot());
	artifacts.SetFrequencyFlagCountPlot(new FrequencyFlagCountPlot());
	artifacts.SetFrequencyPowerPlot(new FrequencyPowerPlot());
	artifacts.SetTimeFlagCountPlot(new TimeFlagCountPlot());

	if(HasImage())
	{
		artifacts.SetOriginalData(GetOriginalData());
		artifacts.SetContaminatedData(GetContaminatedData());
		artifacts.SetImager(_imagePlaneWindow->GetImager());
		TimeFrequencyData *zero = new TimeFrequencyData(GetOriginalData());
		zero->SetImagesToZero();
		artifacts.SetRevisedData(*zero);
		delete zero;
	}
	if(HasImageSet())
	{
		if(dynamic_cast<rfiStrategy::MSImageSet*>(_imageSet) != 0)
		{
			artifacts.SetMetaData(_metaData);
			artifacts.SetImageSet(_imageSet);
			artifacts.SetImageSetIndex(_imageSetIndex);
		}
	}

	try {
		_strategy->StartPerformThread(artifacts, *progressWindow);
	}  catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onExecuteStrategyFinished()
{
	rfiStrategy::ArtifactSet *artifacts = _strategy->JoinThread();
	if(artifacts != 0)
	{
		bool update = false;
		if(!artifacts->RevisedData().IsEmpty())
		{
			std::cout << "Updating revised data..." << std::endl;
			_timeFrequencyWidget.SetRevisedData(artifacts->RevisedData());
			update = true;
		}

		if(!artifacts->ContaminatedData().IsEmpty())
		{
			std::cout << "Updating contaminated data..." << std::endl;
			_timeFrequencyWidget.SetContaminatedData(artifacts->ContaminatedData());
			update = true;
		}
		
		if(update)
			_timeFrequencyWidget.Update();
		
		_imagePlaneWindow->Update();
		
		if(artifacts->AntennaFlagCountPlot()->HasData())
			artifacts->AntennaFlagCountPlot()->MakePlot();
		if(artifacts->FrequencyFlagCountPlot()->HasData())
			artifacts->FrequencyFlagCountPlot()->MakePlot();
		if(artifacts->FrequencyPowerPlot()->HasData())
			artifacts->FrequencyPowerPlot()->MakePlot();
		if(artifacts->TimeFlagCountPlot()->HasData())
			artifacts->TimeFlagCountPlot()->MakePlot();

		delete artifacts->AntennaFlagCountPlot();
		delete artifacts->FrequencyFlagCountPlot();
		delete artifacts->FrequencyPowerPlot();
		delete artifacts->TimeFlagCountPlot();
		delete artifacts;
	}
}

void MSWindow::onToggleImage()
{
	TimeFrequencyWidget::TFImage image = TimeFrequencyWidget::TFOriginalImage;
	if(_backgroundImageButton->get_active())
		image = TimeFrequencyWidget::TFRevisedImage;
	else if(_diffImageButton->get_active())
		image = TimeFrequencyWidget::TFContaminatedImage;
	_timeFrequencyWidget.SetVisualizedImage(image);
	_timeFrequencyWidget.Update();
}

void MSWindow::SetImageSet(rfiStrategy::ImageSet *newImageSet)
{
	if(_imageSet != 0) {
		delete _imageSet;
		delete _imageSetIndex;
	}
	_imageSet = newImageSet;
	_imageSetIndex = _imageSet->StartIndex();
	
	loadCurrentTFData();
}

void MSWindow::SetImageSetIndex(rfiStrategy::ImageSetIndex *newImageSetIndex)
{
	if(HasImageSet())
	{
		delete _imageSetIndex;
		_imageSetIndex = newImageSetIndex;
		loadCurrentTFData();
	} else {
		delete newImageSetIndex;
	}
}

void MSWindow::openTestSet(unsigned index)
{
	unsigned width = 1024, height = 1024;
	if(HasImage())
	{
		width = _timeFrequencyWidget.Image()->Width();
		height = _timeFrequencyWidget.Image()->Height();
	}
	Mask2DPtr rfi = Mask2D::CreateSetMaskPtr<false>(width, height);
	Image2DPtr testSetReal(MitigationTester::CreateTestSet(index, rfi, width, height, _gaussianTestSets));
	Image2DPtr testSetImaginary(MitigationTester::CreateTestSet(index, rfi, width, height, _gaussianTestSets));
	TimeFrequencyData data(SinglePolarisation, testSetReal, testSetImaginary);
	data.SetGlobalMask(rfi);
	
	_timeFrequencyWidget.SetNewData(data, _metaData);
	_timeFrequencyWidget.Update();
}

void MSWindow::createToolbar()
{
	_actionGroup = Gtk::ActionGroup::create();
	_actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
	_actionGroup->add( Gtk::Action::create("MenuGo", "_Go") );
	_actionGroup->add( Gtk::Action::create("MenuView", "_View") );
	_actionGroup->add( Gtk::Action::create("MenuPlot", "_Plot") );
	_actionGroup->add( Gtk::Action::create("MenuActions", "_Actions") );
	_actionGroup->add( Gtk::Action::create("OpenFile", Gtk::Stock::OPEN, "Open _file"),
  sigc::mem_fun(*this, &MSWindow::onActionFileOpen) );
	_actionGroup->add( Gtk::Action::create("OpenDirectory", Gtk::Stock::OPEN, "Open _directory"),
  sigc::mem_fun(*this, &MSWindow::onActionDirectoryOpen) );
	_actionGroup->add( Gtk::Action::create("OpenDirectorySpatial", Gtk::Stock::OPEN, "Open _directory as spatial"),
  sigc::mem_fun(*this, &MSWindow::onActionDirectoryOpenForSpatial) );
	_actionGroup->add( Gtk::Action::create("OpenTestSet", "Open _testset") );

	Gtk::RadioButtonGroup testSetGroup;
	_gaussianTestSetsButton = Gtk::RadioAction::create(testSetGroup, "GaussianTestSets", "Gaussian");
	_gaussianTestSetsButton->set_active(true);
	_rayleighTestSetsButton = Gtk::RadioAction::create(testSetGroup, "RayleighTestSets", "Rayleigh");
	_actionGroup->add(_gaussianTestSetsButton, sigc::mem_fun(*this, &MSWindow::onGaussianTestSets) );
	_actionGroup->add(_rayleighTestSetsButton, sigc::mem_fun(*this, &MSWindow::onRayleighTestSets) );
	
	_actionGroup->add( Gtk::Action::create("OpenTestSetA", "Test set A"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetA) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetB", "Test set B"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetB) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetC", "Test set C"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetC) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetD", "Test set D"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetD) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetE", "Test set E"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetE) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetF", "Test set F"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetF) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetG", "Test set G"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetG) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetH", "Test set H"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetH) );
	_actionGroup->add( Gtk::Action::create("OpenTestSetNoise", "Noise"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetNoise));
	_actionGroup->add( Gtk::Action::create("OpenTestSetModel3", "3-stars model"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSet3Model));
	_actionGroup->add( Gtk::Action::create("OpenTestSetModel5", "5-stars model"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSet5Model));
	_actionGroup->add( Gtk::Action::create("OpenTestSetNoiseModel3", "3-stars model with noise"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetNoise3Model));
	_actionGroup->add( Gtk::Action::create("OpenTestSetNoiseModel5", "5-stars model with noise"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetNoise5Model));
	
	_actionGroup->add( Gtk::Action::create("AddTestModification", "Test modify") );
	_actionGroup->add( Gtk::Action::create("AddStaticFringe", "Static fringe"),
	sigc::mem_fun(*this, &MSWindow::onAddStaticFringe) );
	_actionGroup->add( Gtk::Action::create("Add1SigmaStaticFringe", "Static 1 sigma fringe"),
	sigc::mem_fun(*this, &MSWindow::onAdd1SigmaFringe) );
	_actionGroup->add( Gtk::Action::create("MultiplyData", "Multiply data..."),
	sigc::mem_fun(*this, &MSWindow::onMultiplyData) );
	_actionGroup->add( Gtk::Action::create("Quit", Gtk::Stock::QUIT),
	sigc::mem_fun(*this, &MSWindow::onQuit) );

	_actionGroup->add( Gtk::Action::create("Zoom", "Zoom"),
  	sigc::mem_fun(*this, &MSWindow::onZoomPressed) );
	Gtk::RadioButtonGroup mapGroup;
	_mapBWButton = Gtk::RadioAction::create(mapGroup, "MapBW", "BW map");
	_mapBWButton->set_active(true);
	_mapLogButton = Gtk::RadioAction::create(mapGroup, "MapLog", "Log map");
	_mapColorButton = Gtk::RadioAction::create(mapGroup, "MapColor", "Color map");
	_actionGroup->add(_mapLogButton, sigc::mem_fun(*this, &MSWindow::onToggleMap) );
	_actionGroup->add(_mapBWButton, sigc::mem_fun(*this, &MSWindow::onToggleMap) );
	_actionGroup->add(_mapColorButton, sigc::mem_fun(*this, &MSWindow::onToggleMap) );
	_timeGraphButton = Gtk::ToggleAction::create("TimeGraph", "Time graph");
	_timeGraphButton->set_active(false); 
	_actionGroup->add(_timeGraphButton, sigc::mem_fun(*this, &MSWindow::onTimeGraphButtonPressed) );

	_actionGroup->add( Gtk::Action::create("PlotDist", "Plot _distribution"),
  sigc::mem_fun(*this, &MSWindow::onPlotDistPressed) );
	_actionGroup->add( Gtk::Action::create("PlotComplexPlane", "Plot _complex plane"),
  sigc::mem_fun(*this, &MSWindow::onPlotComplexPlanePressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerSpectrum", "Plot _power spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerSpectrumPressed) );
	_actionGroup->add( Gtk::Action::create("PlotRMSSpectrum", "Plot _rms spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerRMSPressed) );
	_actionGroup->add( Gtk::Action::create("PlotSNRSpectrum", "Plot spectrum snr"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerSNRPressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerTime", "Plot power vs _time"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerTimePressed) );
	_actionGroup->add( Gtk::Action::create("PlotScatter", "Plot power scatter"),
  sigc::mem_fun(*this, &MSWindow::onPlotScatterPressed) );
	_actionGroup->add( Gtk::Action::create("PlotSingularValues", "Plot _singular values"),
  sigc::mem_fun(*this, &MSWindow::onPlotSingularValuesPressed) );
	_actionGroup->add( Gtk::Action::create("PlotSNRToFitVariance", "Plot SNR to fit variance"),
  sigc::mem_fun(*this, &MSWindow::onPlotSNRToFitVariance) );
	_actionGroup->add( Gtk::Action::create("PlotQuality25", "Plot quality (25)"),
  sigc::mem_fun(*this, &MSWindow::onPlotQuality25Pressed) );
	_actionGroup->add( Gtk::Action::create("PlotQualityAll", "Plot quality (all)"),
  sigc::mem_fun(*this, &MSWindow::onPlotQualityAllPressed) );
	_actionGroup->add( Gtk::Action::create("ShowImagePlane", "_Show image plane"),
  sigc::mem_fun(*this, &MSWindow::onShowImagePlane) );
	_actionGroup->add( Gtk::Action::create("AddToImagePlane", "Add to _image plane"),
  sigc::mem_fun(*this, &MSWindow::onAddToImagePlane) );
	_actionGroup->add( Gtk::Action::create("SimulateCorrelation", "Simulate correlation"),
  sigc::mem_fun(*this, &MSWindow::onSimulateCorrelation) );
	_actionGroup->add( Gtk::Action::create("SimulateTwoProductCorrelation", "Simulate 2-p correlation"),
  sigc::mem_fun(*this, &MSWindow::onSimulateDoubledBaselineCorrelation) );
	_actionGroup->add( Gtk::Action::create("SimulateFourProductCorrelation", "Simulate 4-p correlation"),
  sigc::mem_fun(*this, &MSWindow::onSimulateFourProductCorrelation) );

	_actionGroup->add( Gtk::Action::create("EditStrategy", "_Edit strategy"),
  sigc::mem_fun(*this, &MSWindow::onEditStrategyPressed) );
	_actionGroup->add( Gtk::Action::create("ExecuteStrategy", "E_xecute strategy"),
  sigc::mem_fun(*this, &MSWindow::onExecuteStrategyPressed) );
	_actionGroup->add( Gtk::Action::create("AddToStats", "Add to _stats"),
  sigc::mem_fun(*this, &MSWindow::onAddToStats) );
	_actionGroup->add( Gtk::Action::create("Previous", Gtk::Stock::GO_BACK),
  sigc::mem_fun(*this, &MSWindow::onLoadPrevious) );
	_actionGroup->add( Gtk::Action::create("Next", Gtk::Stock::GO_FORWARD),
  sigc::mem_fun(*this, &MSWindow::onLoadNext) );
	_actionGroup->add( Gtk::Action::create("LargeStepPrevious", Gtk::Stock::GOTO_FIRST),
  sigc::mem_fun(*this, &MSWindow::onLoadLargeStepPrevious) );
	_actionGroup->add( Gtk::Action::create("LargeStepNext", Gtk::Stock::GOTO_LAST),
  sigc::mem_fun(*this, &MSWindow::onLoadLargeStepNext) );
	_actionGroup->add( Gtk::Action::create("GoTo", "_Go to..."),
  sigc::mem_fun(*this, &MSWindow::onGoToPressed) );
  _originalFlagsButton = Gtk::ToggleAction::create("OriginalFlags", "Original\nflags");
	_originalFlagsButton->set_active(true); 
	_actionGroup->add(_originalFlagsButton, sigc::mem_fun(*this, &MSWindow::onToggleFlags) );
  _altFlagsButton = Gtk::ToggleAction::create("AlternativeFlags", "Alternative\nflags");
	_altFlagsButton->set_active(true); 
	_actionGroup->add(_altFlagsButton, sigc::mem_fun(*this, &MSWindow::onToggleFlags) );
	_actionGroup->add( Gtk::Action::create("ClearAltFlags", "Clear"),
  sigc::mem_fun(*this, &MSWindow::onClearAltFlagsPressed) );

	Gtk::RadioButtonGroup imageGroup;
	_originalImageButton = Gtk::RadioAction::create(imageGroup, "ImageOriginal", "Original");
	_originalImageButton->set_active(true);
	_backgroundImageButton = Gtk::RadioAction::create(imageGroup, "ImageBackground", "Background");
	_diffImageButton = Gtk::RadioAction::create(imageGroup, "ImageDiff", "Difference");
	_actionGroup->add(_originalImageButton, sigc::mem_fun(*this, &MSWindow::onToggleImage) );
	_actionGroup->add(_backgroundImageButton, sigc::mem_fun(*this, &MSWindow::onToggleImage) );
	_actionGroup->add(_diffImageButton, sigc::mem_fun(*this, &MSWindow::onToggleImage) );

	_actionGroup->add( Gtk::Action::create("DiffToOriginal", "Diff->Original"),
  sigc::mem_fun(*this, &MSWindow::onDifferenceToOriginalPressed) );
	_actionGroup->add( Gtk::Action::create("BackToOriginal", "Background->Original"),
  sigc::mem_fun(*this, &MSWindow::onBackgroundToOriginalPressed) );
	_actionGroup->add( Gtk::Action::create("Segment", "Segment"),
  sigc::mem_fun(*this, &MSWindow::onSegment) );
	_actionGroup->add( Gtk::Action::create("Cluster", "Cluster"),
  sigc::mem_fun(*this, &MSWindow::onCluster) );
	_actionGroup->add( Gtk::Action::create("RemoveSmallSegments", "Remove small segments"),
  sigc::mem_fun(*this, &MSWindow::onRemoveSmallSegments) );

	_actionGroup->add( Gtk::Action::create("Highlight", "Highlight"),
  sigc::mem_fun(*this, &MSWindow::onHightlightPressed) );

	_actionGroup->add( Gtk::Action::create("ShowReal", "Keep _real part"),
  sigc::mem_fun(*this, &MSWindow::onShowRealPressed) );
	_actionGroup->add( Gtk::Action::create("ShowImaginary", "Keep _imaginary part"),
  sigc::mem_fun(*this, &MSWindow::onShowImaginaryPressed) );
	_actionGroup->add( Gtk::Action::create("ShowPhase", "Keep _phase part"),
  sigc::mem_fun(*this, &MSWindow::onShowPhasePressed) );
	_actionGroup->add( Gtk::Action::create("ShowStokesI", "Keep _stokesI part"),
  sigc::mem_fun(*this, &MSWindow::onShowStokesIPressed) );
	_actionGroup->add( Gtk::Action::create("ShowAutoPol", "Keep xx+yy part"),
  sigc::mem_fun(*this, &MSWindow::onShowAutoDipolePressed) );
	_actionGroup->add( Gtk::Action::create("ShowCrossPol", "Keep xy+yx part"),
  sigc::mem_fun(*this, &MSWindow::onShowCrossDipolePressed) );
	_actionGroup->add( Gtk::Action::create("ShowXX", "Keep _xx part"),
  sigc::mem_fun(*this, &MSWindow::onShowXXPressed) );
	_actionGroup->add( Gtk::Action::create("ShowXY", "Keep xy part"),
  sigc::mem_fun(*this, &MSWindow::onShowXYPressed) );
	_actionGroup->add( Gtk::Action::create("ShowYX", "Keep yx part"),
  sigc::mem_fun(*this, &MSWindow::onShowYXPressed) );
	_actionGroup->add( Gtk::Action::create("ShowYY", "Keep _yy part"),
  sigc::mem_fun(*this, &MSWindow::onShowYYPressed) );
	_actionGroup->add( Gtk::Action::create("UnrollPhase", "_Unroll phase"),
  sigc::mem_fun(*this, &MSWindow::onUnrollPhaseButtonPressed) );

	Glib::RefPtr<Gtk::UIManager> uiManager =
		Gtk::UIManager::create();
	uiManager->insert_action_group(_actionGroup);
	add_accel_group(uiManager->get_accel_group());

	Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='MenuFile'>"
    "      <menuitem action='OpenFile'/>"
    "      <menuitem action='OpenDirectory'/>"
    "      <menuitem action='OpenDirectorySpatial'/>"
    "      <menu action='OpenTestSet'>"
		"        <menuitem action='GaussianTestSets'/>"
		"        <menuitem action='RayleighTestSets'/>"
    "        <separator/>"
		"        <menuitem action='OpenTestSetA'/>"
		"        <menuitem action='OpenTestSetB'/>"
		"        <menuitem action='OpenTestSetC'/>"
		"        <menuitem action='OpenTestSetD'/>"
		"        <menuitem action='OpenTestSetE'/>"
		"        <menuitem action='OpenTestSetF'/>"
		"        <menuitem action='OpenTestSetG'/>"
		"        <menuitem action='OpenTestSetH'/>"
		"        <menuitem action='OpenTestSetNoise'/>"
		"        <menuitem action='OpenTestSetModel3'/>"
		"        <menuitem action='OpenTestSetModel5'/>"
		"        <menuitem action='OpenTestSetNoiseModel3'/>"
		"        <menuitem action='OpenTestSetNoiseModel5'/>"
		"      </menu>"
		"      <menu action='AddTestModification'>"
		"        <menuitem action='AddStaticFringe'/>"
		"        <menuitem action='Add1SigmaStaticFringe'/>"
		"        <menuitem action='MultiplyData'/>"
		"      </menu>"
    "      <menuitem action='Quit'/>"
    "    </menu>"
	  "    <menu action='MenuView'>"
    "      <menuitem action='Zoom'/>"
    "      <separator/>"
    "      <menuitem action='MapBW'/>"
    "      <menuitem action='MapLog'/>"
    "      <menuitem action='MapColor'/>"
    "      <separator/>"
    "      <menuitem action='TimeGraph'/>"
	  "    </menu>"
	  "    <menu action='MenuPlot'>"
    "      <menuitem action='PlotDist'/>"
    "      <menuitem action='PlotComplexPlane'/>"
    "      <menuitem action='PlotPowerSpectrum'/>"
    "      <menuitem action='PlotRMSSpectrum'/>"
    "      <menuitem action='PlotSNRSpectrum'/>"
    "      <menuitem action='PlotPowerTime'/>"
    "      <menuitem action='PlotScatter'/>"
    "      <menuitem action='PlotSingularValues'/>"
    "      <menuitem action='PlotSNRToFitVariance'/>"
    "      <menuitem action='PlotQuality25'/>"
    "      <menuitem action='PlotQualityAll'/>"
    "      <separator/>"
    "      <menuitem action='ShowImagePlane'/>"
    "      <menuitem action='AddToImagePlane'/>"
    "      <separator/>"
    "      <menuitem action='SimulateCorrelation'/>"
    "      <menuitem action='SimulateTwoProductCorrelation'/>"
    "      <menuitem action='SimulateFourProductCorrelation'/>"
	  "    </menu>"
    "    <menu action='MenuGo'>"
    "      <menuitem action='LargeStepPrevious'/>"
    "      <menuitem action='Previous'/>"
    "      <menuitem action='Next'/>"
    "      <menuitem action='LargeStepNext'/>"
    "      <separator/>"
    "      <menuitem action='GoTo'/>"
    "    </menu>"
	  "    <menu action='MenuActions'>"
    "      <menuitem action='DiffToOriginal'/>"
    "      <menuitem action='BackToOriginal'/>"
    "      <menuitem action='Segment'/>"
    "      <menuitem action='Cluster'/>"
    "      <menuitem action='RemoveSmallSegments'/>"
    "      <separator/>"
    "      <menuitem action='EditStrategy'/>"
    "      <menuitem action='ExecuteStrategy'/>"
    "      <separator/>"
    "      <menuitem action='ShowReal'/>"
    "      <menuitem action='ShowImaginary'/>"
    "      <menuitem action='ShowPhase'/>"
    "      <separator/>"
    "      <menuitem action='ShowStokesI'/>"
    "      <menuitem action='ShowXX'/>"
    "      <menuitem action='ShowXY'/>"
    "      <menuitem action='ShowYX'/>"
    "      <menuitem action='ShowYY'/>"
    "      <menuitem action='ShowAutoPol'/>"
    "      <menuitem action='ShowCrossPol'/>"
    "      <separator/>"
    "      <menuitem action='UnrollPhase'/>"
	  "    </menu>"
    "  </menubar>"
    "  <toolbar  name='ToolBar'>"
    "    <toolitem action='OpenDirectory'/>"
    "    <separator/>"
    "    <toolitem action='ClearAltFlags'/>"
    "    <toolitem action='AddToStats'/>"
    "    <separator/>"
    "    <toolitem action='Previous'/>"
    "    <toolitem action='Next'/>"
    "    <toolitem action='OriginalFlags'/>"
    "    <toolitem action='AlternativeFlags'/>"
    "    <toolitem action='Highlight'/>"
    "    <separator/>"
    "    <toolitem action='ImageOriginal'/>"
    "    <toolitem action='ImageBackground'/>"
    "    <toolitem action='ImageDiff'/>"
    "  </toolbar>"
    "</ui>";

	uiManager->add_ui_from_string(ui_info);
	Gtk::Widget* pMenubar = uiManager->get_widget("/MenuBar");
	_mainVBox.pack_start(*pMenubar, Gtk::PACK_SHRINK);
	Gtk::Widget* pToolbar = uiManager->get_widget("/ToolBar");
	_mainVBox.pack_start(*pToolbar, Gtk::PACK_SHRINK);
	pMenubar->show();
}

void MSWindow::onClearAltFlagsPressed()
{
	_timeFrequencyWidget.Update();
}

void MSWindow::onDifferenceToOriginalPressed()
{
	if(HasImage())
	{
		TimeFrequencyData data(_timeFrequencyWidget.ContaminatedData());
		_timeFrequencyWidget.SetNewData(data, _metaData);
	}
	if(_originalImageButton->get_active())
		_timeFrequencyWidget.Update();
	else
		_originalImageButton->set_active();
}

void MSWindow::onBackgroundToOriginalPressed()
{
	if(HasImage())
	{
		TimeFrequencyData data(_timeFrequencyWidget.RevisedData());
		_timeFrequencyWidget.ClearBackground();
		_timeFrequencyWidget.SetNewData(data, _metaData);
	}
	if(_originalImageButton->get_active())
		_timeFrequencyWidget.Update();
	else
		_originalImageButton->set_active();
}

void MSWindow::onHightlightPressed()
{
	Gtk::Window *window = new HighlightWindow(*this);
	window->show();
	_subWindows.push_back(window);
}

void MSWindow::onAddStaticFringe()
{
	try {
		if(HasImage())
		{
			TimeFrequencyMetaDataCPtr metaData = TimeFrequencyMetaData();
			TimeFrequencyData data(GetActiveData());
			FringeTestCreater::AddStaticFringe(data, metaData, 1.0L);
			_timeFrequencyWidget.SetNewData(data, metaData);
			_timeFrequencyWidget.Update();
		}
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onAdd1SigmaFringe()
{
	try {
		if(HasImage())
		{
			TimeFrequencyMetaDataCPtr metaData = TimeFrequencyMetaData();
			long double mean, stddev;
			TimeFrequencyData data(GetActiveData());
			ThresholdTools::MeanAndStdDev(data.GetRealPart(), data.GetSingleMask(), mean, stddev);
			FringeTestCreater::AddStaticFringe(data, metaData, stddev);
			_timeFrequencyWidget.SetNewData(data, _metaData);
			_timeFrequencyWidget.Update();
		}
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onAddToStats()
{
	if(_timeFrequencyWidget.HasImage())
	{
		TimeFrequencyData activeData = GetActiveData();
		_statistics->Add(activeData.GetSingleImage(), activeData.GetSingleMask());
		std::cout
			<< "Average connected size: " << _statistics->GetAverageConnectedRFISize() << "\n"
			<< "Average connected RFI flux: " << _statistics->GetAverageConnectedRFIFlux() << "\n"
			<< "Average connected duration: " << _statistics->GetAverageConnectedRFIDuration() << "\n"
			<< "Average connected frequency coverage: " << _statistics->GetAverageConnectedRFIFrequencyCoverage() << "\n\n"
			<< "Average peeled size: " << _statistics->GetAveragePeeledRFISize() << "\n"
			<< "Average peeled RFI flux: " << _statistics->GetAveragePeeledRFIFlux() << "\n"
			<< "Average peeled duration: " << _statistics->GetAveragePeeledRFIDuration() << "\n"
			<< "Average peeled frequency coverage: " << _statistics->GetAveragePeeledRFIFrequencyCoverage() << "\n\n"
			<< "Average non-RFI flux: " << _statistics->GetAverageNonRFIFlux() << "\n"
			<< "Invalid value count: " << _statistics->GetInvalidValuesCount() << std::endl;
		_statistics->MakePlot();
	}
}

void MSWindow::onPlotDistPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot plot("dist.pdf");

		TimeFrequencyData activeData = GetActiveData();
		Image2DCPtr image = activeData.GetSingleImage();
		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(image->Width(), image->Height());
		plot.StartLine("Total");
		RFIPlots::MakeDistPlot(plot, image, mask);

		plot.StartLine("Uncontaminated");
		mask = Mask2D::CreateCopy(activeData.GetSingleMask());
		RFIPlots::MakeDistPlot(plot, image, mask);

		mask->Invert();
		plot.StartLine("RFI");
		RFIPlots::MakeDistPlot(plot, image, mask);

		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotComplexPlanePressed()
{
	if(HasImage()) {
		Gtk::Window *window = new ComplexPlanePlotWindow(*this);
		window->show();
		_subWindows.push_back(window);
	}
}

void MSWindow::onPlotPowerSpectrumPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot plot("power-spectrum.pdf");

		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(_timeFrequencyWidget.Image()->Width(), _timeFrequencyWidget.Image()->Height());
		plot.StartLine("Total");		
		RFIPlots::MakePowerSpectrumPlot(plot, _timeFrequencyWidget.Image(), mask);

		mask = Mask2D::CreateCopy(_timeFrequencyWidget.GetActiveData().GetSingleMask());
		if(!mask->AllFalse())
		{
			plot.StartLine("Uncontaminated");
			RFIPlots::MakePowerSpectrumPlot(plot, _timeFrequencyWidget.Image(), mask);
	
			mask->Invert();
			plot.StartLine("RFI");
			RFIPlots::MakePowerSpectrumPlot(plot, _timeFrequencyWidget.Image(), mask);
		}

		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotPowerRMSPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot plot("spectrum-rms.pdf");

		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(_timeFrequencyWidget.Image()->Width(), _timeFrequencyWidget.Image()->Height());
		plot.StartLine("Total");		
		RFIPlots::MakeRMSSpectrumPlot(plot, _timeFrequencyWidget.Image(), mask);

		mask = Mask2D::CreateCopy(_timeFrequencyWidget.GetActiveData().GetSingleMask());
		if(!mask->AllFalse())
		{
			plot.StartLine("Uncontaminated");
			RFIPlots::MakeRMSSpectrumPlot(plot, _timeFrequencyWidget.Image(), mask);
	
			mask->Invert();
			plot.StartLine("RFI");
			RFIPlots::MakeRMSSpectrumPlot(plot, _timeFrequencyWidget.Image(), mask);
		}

		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotPowerSNRPressed()
{
	Image2DCPtr
		image = _timeFrequencyWidget.GetActiveData().GetSingleImage(),
		model = _timeFrequencyWidget.RevisedData().GetSingleImage();
	if(_timeFrequencyWidget.HasImage())
	{
		Plot plot("spectrum-snr.pdf");

		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(image->Width(), image->Height());
		plot.StartLine("Total");		
		RFIPlots::MakeSNRSpectrumPlot(plot, image, model, mask);

		mask = Mask2D::CreateCopy(_timeFrequencyWidget.GetActiveData().GetSingleMask());
		if(!mask->AllFalse())
		{
			plot.StartLine("Uncontaminated");
			RFIPlots::MakeSNRSpectrumPlot(plot, image, model, mask);
	
			mask->Invert();
			plot.StartLine("RFI");
			RFIPlots::MakeSNRSpectrumPlot(plot, image, model, mask);
		}

		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotPowerTimePressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot plot("dist.pdf");

		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(_timeFrequencyWidget.Image()->Width(), _timeFrequencyWidget.Image()->Height());
		plot.StartLine("Total");		
		RFIPlots::MakePowerTimePlot(plot, _timeFrequencyWidget.Image(), mask);

		mask = Mask2D::CreateCopy(_timeFrequencyWidget.GetActiveData().GetSingleMask());
		if(!mask->AllFalse())
		{
			plot.StartLine("Uncontaminated");
			RFIPlots::MakePowerTimePlot(plot, _timeFrequencyWidget.Image(), mask);
	
			mask->Invert();
			plot.StartLine("RFI");
			RFIPlots::MakePowerTimePlot(plot, _timeFrequencyWidget.Image(), mask);
		}

		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotScatterPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		MultiPlot plot("scatter.pdf", 4);
		RFIPlots::MakeScatterPlot(plot, GetActiveData());
		plot.Finish();
		plot.Show();
	}
}

void MSWindow::onPlotSingularValuesPressed()
{
	if(HasImage())
	{
		Plot plot("singularvalues.pdf");

		SVDMitigater::CreateSingularValueGraph(GetActiveData(), plot);
		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotQuality25Pressed()
{
	if(HasImage())
	{
		Plot plot("quality.pdf");
		RFIPlots::MakeQualityPlot(plot, GetActiveData(), _timeFrequencyWidget.RevisedData(), 25);
		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotQualityAllPressed()
{
	if(HasImage())
	{
		Plot plot("quality.pdf");
		RFIPlots::MakeQualityPlot(plot, GetActiveData(), _timeFrequencyWidget.RevisedData(), _timeFrequencyWidget.RevisedData().ImageWidth());
		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotSNRToFitVariance()
{
	bool relative = false;

	FringeStoppingFitter fitter;
	fitter.SetMetaData(_metaData);
	
	Plot
		plotA("/tmp/snrplot-a.pdf"),
		plotB("/tmp/snrplot-b.pdf");
	plotA.StartLine("Stddev");
	plotA.SetTitle("Fit errors");
	plotA.SetXAxisText("SNR (dB)");
	plotA.SetYAxisText("Error (sigma-epsilon)");
	plotA.SetLogScale(false, false, false);
	plotB.StartLine("Stddev");
	plotB.SetTitle("Fit errors");
	plotB.SetXAxisText("SNR (ratio, non-logarithmic)");
	plotB.SetYAxisText("Error (sigma-epsilon)");
	plotB.SetLogScale(false, false, false);

	const unsigned iterations = 2500;
	std::vector<long double> medians, means, maxs, snrDbs, snrRatios;

	long double start = 4.6;
	long double stop = 0.0001;
	if(relative)
		stop = 0.01;

	plotA.SetXRange(10.0 * logl(stop) / logl(10.0L), 10.0 * logl(start) / logl(10.0L));
	plotB.SetXRange(stop, start);
	for(long double snr = start;snr>stop;snr *= 0.9) {
		long double amplitudes[iterations], mean = 0, stddev = 0;
		long double db = 10.0 * logl(snr) / logl(10.0L);
		long double max = -100e10;
		for(size_t i=0;i<iterations;++i)
		{
			unsigned width = 1024, height = 1;
			if(HasImage())
			{
				width = _timeFrequencyWidget.OriginalData().ImageWidth();
				//height = _timeFrequencyWidget.Image()->Height();
			}
			width /= 16;

			Mask2DPtr rfi = Mask2D::CreateSetMaskPtr<false>(width, height);
			Image2DPtr testSetReal(MitigationTester::CreateTestSet(2, rfi, width, height, _gaussianTestSets));
			Image2DPtr testSetImaginary(MitigationTester::CreateTestSet(2, rfi, width, height, _gaussianTestSets));
			TimeFrequencyData *data = new TimeFrequencyData(SinglePolarisation, testSetReal, testSetImaginary);
	
			TimeFrequencyMetaDataCPtr metaData = TimeFrequencyMetaData();
			FringeTestCreater::AddStaticFringe(*data, metaData, snr);
	
			fitter.Initialize(*data);
			amplitudes[i] =
				fitter.GetAmplitude(height/2, height/2+1) - snr;
			mean += amplitudes[i];
	
			delete data;
		}
		mean /= iterations;
		for(size_t i=0;i<iterations;++i)
		{
			stddev += (amplitudes[i] - mean) * (amplitudes[i] - mean);
			if(amplitudes[i] > max) max = amplitudes[i];
		}
		stddev = sqrtl(stddev / iterations);
		unsigned medianIndex = iterations/2;
		std::nth_element(amplitudes, amplitudes + medianIndex, amplitudes+iterations);
		std::cout << "Snr: " << snr << " (=" << db << " dB), stddev: " << stddev << ", median: " << amplitudes[medianIndex] << ", max: " << max << std::endl;
		if(relative)
		{
			plotA.PushDataPoint(db, stddev/snr);
			plotB.PushDataPoint(snr, stddev/snr);
		} else {
			plotA.PushDataPoint(db, stddev);
			plotB.PushDataPoint(snr, stddev);
		}
		medians.push_back(amplitudes[medianIndex]);
		snrRatios.push_back(snr);
		snrDbs.push_back(db);
		means.push_back(mean);
		maxs.push_back(max);
	}
	plotA.StartLine("median");
	plotB.StartLine("median");
	for(unsigned i=0;i<snrDbs.size();++i)
	{
		if(relative)
		{
			plotA.PushDataPoint(snrDbs[i], medians[i]/snrRatios[i]);
			plotB.PushDataPoint(snrRatios[i], medians[i]/snrRatios[i]);
		} else {
			plotA.PushDataPoint(snrDbs[i], medians[i]);
			plotB.PushDataPoint(snrRatios[i], medians[i]);
		}
	}
	plotA.StartLine("mean");
	plotB.StartLine("mean");
	for(unsigned i=0;i<means.size();++i)
	{
		if(relative)
		{
			plotA.PushDataPoint(snrDbs[i], means[i]/snrRatios[i]);
			plotB.PushDataPoint(snrRatios[i], means[i]/snrRatios[i]);
		} else {
			plotA.PushDataPoint(snrDbs[i], means[i]);
			plotB.PushDataPoint(snrRatios[i], means[i]);
		}
	}
	plotA.StartLine("max");
	plotB.StartLine("max");
	for(unsigned i=0;i<maxs.size();++i)
	{
		if(relative)
		{
			plotA.PushDataPoint(snrDbs[i], maxs[i]/snrRatios[i]);
			plotB.PushDataPoint(snrRatios[i], maxs[i]/snrRatios[i]);
		} else {
			plotA.PushDataPoint(snrDbs[i], maxs[i]);
			plotB.PushDataPoint(snrRatios[i], maxs[i]);
		}
	}
	plotA.StartLine("snr=error");
	plotB.StartLine("snr=error");
	for(unsigned i=0;i<means.size();++i)
	{
		if(relative)
		{
			plotA.PushDataPoint(snrDbs[i], powl(10.0L, snrDbs[i]/10.0L)/snrRatios[i]);
			plotB.PushDataPoint(snrRatios[i], 1);
		} else {
			if(powl(10.0L, snrDbs[i]/10.0L) < 0.45)
			{
				plotA.PushDataPoint(snrDbs[i], powl(10.0L, snrDbs[i]/10.0L));
			}
			plotB.PushDataPoint(snrRatios[i], snrRatios[i]);
		}
	}
	plotA.Close();
	plotA.Show();
	plotB.Close();
	plotB.Show();
}

void MSWindow::onZoomPressed()
{
	if(HasImageSet())
	{
		Gtk::Window *zoomWindow = new ZoomWindow(*this);
		zoomWindow->show();
		_subWindows.push_back(zoomWindow);
	}
}

void MSWindow::showPhasePart(enum TimeFrequencyData::PhaseRepresentation phaseRepresentation)
{
	if(HasImage())
	{
		TimeFrequencyData *newPart =  _timeFrequencyWidget.GetActiveData().CreateTFData(phaseRepresentation);
		_timeFrequencyWidget.SetNewData(*newPart, _metaData);
		delete newPart;
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::showPolarisation(enum PolarisationType polarisation)
{
	if(HasImage())
	{
		TimeFrequencyData *newData =
			_timeFrequencyWidget.GetActiveData().CreateTFData(polarisation);
		_timeFrequencyWidget.SetNewData(*newData, _metaData);
		delete newData;
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onGoToPressed()
{
	if(HasImageSet())
	{
		Gtk::Window *goToWindow = new GoToWindow(*this);
		goToWindow->show();
		_subWindows.push_back(goToWindow);
	}
}

bool MSWindow::onTFWidgetMotion(GdkEventMotion* event)
{
	if(HasImage() && _metaData != 0)
	{
		Image2DCPtr image = _timeFrequencyWidget.Image();
		size_t posX = (size_t) roundl((long double) event->x * image->Width() / _timeFrequencyWidget.get_width() - 0.5L);
		size_t posY = (size_t) roundl((long double) event->y * image->Height() / _timeFrequencyWidget.get_height() - 0.5L);
		if(posX >= image->Width()) posX = image->Width() - 1;
		if(posY >= image->Height()) posY = image->Height() - 1;
		long double v = image->Value(posX, posY);
		_statusbar.pop();
		std::stringstream s;
		s << "x=" << posX << ",y=" << posY << ",value=" << v;
		const std::vector<double> &times = _metaData->ObservationTimes();
		s << " (t=" << Date::AipsMJDToString(times[posX]) <<
		", f=" << Frequency::ToString(_metaData->Band().channels[posY].frequencyHz)
		<< ")";
		_statusbar.push(s.str(), 0);
	}

	return true;
}

void MSWindow::onShowImagePlane()
{
	_imagePlaneWindow->show();
}

void MSWindow::onAddToImagePlane()
{
	try {
		if(_metaData != 0)
			_imagePlaneWindow->AddData(GetActiveData(), _metaData);
		else if(_spatialMetaData != 0)
			_imagePlaneWindow->AddData(GetActiveData(), _spatialMetaData);
		else
			showError("No meta data found.");
	}  catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onMultiplyData()
{
	TimeFrequencyData data(GetActiveData());
	data.MultiplyImages(2.0L);
	_timeFrequencyWidget.SetNewData(data, _metaData);
	_timeFrequencyWidget.Update();
}

void MSWindow::onSegment()
{
	_segmentedImage = SegmentedImage::CreateUnsetPtr(GetOriginalData().ImageWidth(),  GetOriginalData().ImageHeight());
	Morphology morphology;
	morphology.SegmentByLengthRatio(GetActiveData().GetSingleMask(), _segmentedImage);
	_timeFrequencyWidget.SetSegmentedImage(_segmentedImage);
	Update();
}

void MSWindow::onCluster()
{
	if(_segmentedImage != 0)
	{
		Morphology morphology;
		morphology.Cluster(_segmentedImage);
	_timeFrequencyWidget.SetSegmentedImage(_segmentedImage);
		Update();
	}
}

void MSWindow::onRemoveSmallSegments()
{
	if(_segmentedImage != 0)
	{
		Morphology morphology;
		morphology.RemoveSmallSegments(_segmentedImage, 4);
	_timeFrequencyWidget.SetSegmentedImage(_segmentedImage);
		Update();
	}
}

void MSWindow::onTimeGraphButtonPressed()
{
	if(_timeGraphButton->get_active())
	{
		_mainVBox.remove(_timeFrequencyWidget);
		_mainVBox.pack_start(_panedArea);
		_panedArea.pack1(_timeFrequencyWidget, true, true);
		_panedArea.pack2(_plotFrame, true, true);

		_panedArea.show();
		_timeFrequencyWidget.show();
		_plotFrame.show();
	} else {
		_mainVBox.remove(_panedArea);
		_panedArea.remove(_timeFrequencyWidget);
		_panedArea.remove(_plotFrame);

		_mainVBox.pack_start(_timeFrequencyWidget);
		_timeFrequencyWidget.show();
	}
}

bool MSWindow::onTFWidgetButtonReleased(GdkEventButton *event)
{
	if(HasImage() && _plotFrame.is_visible())
	{
		size_t 
			width = GetOriginalData().ImageWidth(),
			height = GetOriginalData().ImageHeight();
		size_t posX = (size_t) roundl((long double) event->x * width / _timeFrequencyWidget.get_width() - 0.5L);
		size_t posY = (size_t) roundl((long double) event->y * height / _timeFrequencyWidget.get_height() - 0.5L);
		if(posX >= width)
			posX = width - 1;
		if(posY >= height)
			posY = height - 1;

		_plotFrame.SetTimeFrequencyData(GetActiveData());
		_plotFrame.SetSelectedSample(posX, posY);
	
		_plotFrame.Update();
	}

	return true;
}

void MSWindow::onUnrollPhaseButtonPressed()
{
	if(HasImage())
	{
		TimeFrequencyData *data =
			GetActiveData().CreateTFData(TimeFrequencyData::PhasePart);
		for(unsigned i=0;i<data->ImageCount();++i)
		{
			Image2DPtr image = Image2D::CreateCopy(data->GetImage(i));
			ThresholdTools::UnrollPhase(image);
			data->SetImage(i, image);
		}
		_timeFrequencyWidget.SetNewData(*data, _metaData);
		_timeFrequencyWidget.Update();
		delete data;
	}
}

void MSWindow::showError(const std::string &description)
{
	Gtk::MessageDialog dialog(*this, description, false, Gtk::MESSAGE_ERROR);
	dialog.run();
}

void MSWindow::onSimulateCorrelation()
{
	Model model;
	//model.AddSource(-M_PI - 0.04,0.04,0.5);
	//model.AddSource(-M_PI - 0.04075,0.04075,0.2);
	//model.AddSource(-M_PI + 0.1,0.0,0.35);
	//model.AddSource(-M_PI + .101,0.001,0.45);
	model.loadUrsaMajor();

	WSRTObservatorium wsrtObservatorium;
	model.SimulateObservation(*_imagePlaneWindow->GetImager(), wsrtObservatorium, -M_PI-0.05, 0.05, 147000000.0);
	_imagePlaneWindow->Update();
}

void MSWindow::onSimulateDoubledBaselineCorrelation()
{
	Model model;
	//model.AddSource(-M_PI - 0.04,0.04,0.5);
	//model.AddSource(-M_PI - 0.04075,0.04075,0.2);
	//model.AddSource(-M_PI + 0.1,0.0,0.35);
	//model.AddSource(-M_PI + .101,0.001,0.45);
	model.loadUrsaMajor();

	WSRTObservatorium wsrtObservatorium;
	FourProductCorrelatorTester fpcTester(model, *_imagePlaneWindow->GetImager(), wsrtObservatorium);
	fpcTester.SimulateTwoProdObservation(-M_PI-0.05, 0.05, 147000000.0); //TwoProd
	_imagePlaneWindow->Update();
}

void MSWindow::onSimulateFourProductCorrelation()
{
	Model model;
	//model.AddSource(-M_PI - 0.04,0.04,0.5);
	//model.AddSource(-M_PI - 0.04075,0.04075,0.2);
	//model.AddSource(-M_PI + 0.1,0.0,0.35);
	//model.AddSource(-M_PI + .101,0.001,0.45);
	model.loadUrsaMajor();

	WSRTObservatorium wsrtObservatorium;
	FourProductCorrelatorTester fpcTester(model, *_imagePlaneWindow->GetImager(), wsrtObservatorium);
	fpcTester.SimulateObservation(-M_PI-0.05, 0.05, 147000000.0);
	_imagePlaneWindow->Update();
}
