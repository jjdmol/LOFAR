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
#include <AOFlagger/msio/timefrequencymetadata.h>
#include <AOFlagger/msio/segmentedimage.h>
#include <AOFlagger/msio/spatialmatrixmetadata.h>

#include <AOFlagger/strategy/actions/baselineselectionaction.h>
#include <AOFlagger/strategy/actions/strategyaction.h>

#include <AOFlagger/strategy/control/artifactset.h>

#include <AOFlagger/strategy/imagesets/msimageset.h>
#include <AOFlagger/strategy/imagesets/noisestatimageset.h>
#include <AOFlagger/strategy/imagesets/bandcombinedset.h>
#include <AOFlagger/strategy/imagesets/spatialmsimageset.h>
#include <AOFlagger/strategy/imagesets/spatialtimeimageset.h>

#include <AOFlagger/strategy/algorithms/mitigationtester.h>
#include <AOFlagger/strategy/algorithms/morphology.h>
#include <AOFlagger/strategy/algorithms/fringetestcreater.h>
#include <AOFlagger/strategy/algorithms/fringestoppingfitter.h>
#include <AOFlagger/strategy/algorithms/polarizationstatistics.h>
#include <AOFlagger/strategy/algorithms/rfistatistics.h>
#include <AOFlagger/strategy/algorithms/svdmitigater.h>
#include <AOFlagger/strategy/algorithms/thresholdtools.h>
#include <AOFlagger/strategy/algorithms/timefrequencystatistics.h>
#include <AOFlagger/strategy/algorithms/vertevd.h>

#include <AOFlagger/strategy/plots/antennaflagcountplot.h>
#include <AOFlagger/strategy/plots/frequencyflagcountplot.h>
#include <AOFlagger/strategy/plots/frequencypowerplot.h>
#include <AOFlagger/strategy/plots/iterationsplot.h>
#include <AOFlagger/strategy/plots/rfiplots.h>
#include <AOFlagger/strategy/plots/timeflagcountplot.h>

#include <AOFlagger/util/compress.h>
#include <AOFlagger/util/plot.h>
#include <AOFlagger/util/multiplot.h>

#include <AOFlagger/gui/plot/plot2d.h>

#include <AOFlagger/gui/antennamapwindow.h>
#include <AOFlagger/gui/complexplaneplotwindow.h>
#include <AOFlagger/gui/editstrategywindow.h>
#include <AOFlagger/gui/gotowindow.h>
#include <AOFlagger/gui/highlightwindow.h>
#include <AOFlagger/gui/imageplanewindow.h>
#include <AOFlagger/gui/imagepropertieswindow.h>
#include <AOFlagger/gui/msoptionwindow.h>
#include <AOFlagger/gui/noisestatoptionwindow.h>
#include <AOFlagger/gui/numinputdialog.h>
#include <AOFlagger/gui/progresswindow.h>
#include <AOFlagger/gui/rawoptionwindow.h>
#include <AOFlagger/gui/tfstatoptionwindow.h>

#include <AOFlagger/imaging/model.h>
#include <AOFlagger/imaging/observatorium.h>

#include <iostream>

MSWindow::MSWindow() : _imagePlaneWindow(0), _optionWindow(0), _editStrategyWindow(0), _gotoWindow(0), _progressWindow(0), _highlightWindow(0), _plotComplexPlaneWindow(0), _imagePropertiesWindow(0), _antennaMapWindow(0), _statistics(new RFIStatistics()),  _imageSet(0), _imageSetIndex(0), _gaussianTestSets(true), _spatialMetaData(0)
{
	createToolbar();

	_mainVBox.pack_start(_timeFrequencyWidget);
	_timeFrequencyWidget.OnMouseMovedEvent().connect(sigc::mem_fun(*this, &MSWindow::onTFWidgetMouseMoved));
	_timeFrequencyWidget.OnButtonReleasedEvent().connect(sigc::mem_fun(*this, &MSWindow::onTFWidgetButtonReleased));
	_timeFrequencyWidget.SetShowAxisDescriptions(false);
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
	delete _imagePlaneWindow;
	if(_optionWindow != 0)
		delete _optionWindow;
	if(_editStrategyWindow != 0)
		delete _editStrategyWindow;
	if(_gotoWindow != 0)
		delete _gotoWindow;
	if(_progressWindow != 0)
		delete _progressWindow;
	if(_highlightWindow != 0)
		delete _highlightWindow;
	if(_imagePropertiesWindow != 0)
		delete _imagePropertiesWindow;
	if(_antennaMapWindow != 0)
		delete _antennaMapWindow;
	
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
		openPath(dialog.get_filename());
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
		rfiStrategy::SpatialMSImageSet *imageSet = new rfiStrategy::SpatialMSImageSet(dialog.get_filename());
		imageSet->Initialize();
		SetImageSet(imageSet);
	}
}

void MSWindow::onActionDirectoryOpenForST()
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
		rfiStrategy::SpatialTimeImageSet *imageSet = new rfiStrategy::SpatialTimeImageSet(dialog.get_filename());
		imageSet->Initialize();
		SetImageSet(imageSet);
	}
}

void MSWindow::onOpenBandCombined()
{
	std::vector<std::string> names;
	int result;
	do
	{
		Gtk::FileChooserDialog dialog("Select a measurement set",
						Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
		dialog.set_transient_for(*this);
	
		//Add response buttons the the dialog:
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button("Open", Gtk::RESPONSE_OK);
	
		result = dialog.run();
		if(result == Gtk::RESPONSE_OK)
			names.push_back(dialog.get_filename());
	}
  while(result == Gtk::RESPONSE_OK);
	if(names.size() > 0)
	{
		rfiStrategy::BandCombinedSet *imageSet = new rfiStrategy::BandCombinedSet(names);
		imageSet->Initialize();
		SetImageSet(imageSet);
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
		openPath(dialog.get_filename());
	}
}

void MSWindow::openPath(const std::string &path)
{
	if(_optionWindow != 0)
		delete _optionWindow;
	if(rfiStrategy::ImageSet::IsRawFile(path))
	{
		_optionWindow = new RawOptionWindow(*this, path);
		_optionWindow->show();
	}
	else if(rfiStrategy::ImageSet::IsMSFile(path))
	{
		_optionWindow = new MSOptionWindow(*this, path);
		_optionWindow->show();
	}
	else if(rfiStrategy::ImageSet::IsTimeFrequencyStatFile(path))
	{
		_optionWindow = new TFStatOptionWindow(*this, path);
		_optionWindow->show();
	}
	else if(rfiStrategy::ImageSet::IsNoiseStatFile(path))
	{
		_optionWindow = new NoiseStatOptionWindow(*this, path);
		_optionWindow->show();
	}
	else
	{
		rfiStrategy::ImageSet *imageSet = rfiStrategy::ImageSet::Create(path);
		imageSet->Initialize();
		SetImageSet(imageSet);
	}
}

void MSWindow::onToggleFlags()
{
	_timeFrequencyWidget.SetShowOriginalMask(_originalFlagsButton->get_active());
	_timeFrequencyWidget.SetShowAlternativeMask(_altFlagsButton->get_active());
	_timeFrequencyWidget.Update();
}

void MSWindow::onToggleMap()
{
	ImageWidget::TFMap colorMap = ImageWidget::BWMap;
	if(_mapInvertedButton->get_active())
		colorMap = ImageWidget::InvertedMap;
	else if(_mapColorButton->get_active())
		colorMap = ImageWidget::ColorMap;
	else if(_mapRedBlueButton->get_active())
		colorMap = ImageWidget::RedBlueMap;
	else if(_mapRedYellowBlueButton->get_active())
		colorMap = ImageWidget::RedYellowBlueMap;
	_timeFrequencyWidget.SetColorMap(colorMap);
	_timeFrequencyWidget.Update();
}

void MSWindow::loadCurrentTFData()
{
	if(_imageSet != 0) {
		try {
			_imageSet->AddReadRequest(*_imageSetIndex);
			_imageSet->PerformReadRequests();
			rfiStrategy::BaselineData *baseline = _imageSet->GetNextRequested();
			_timeFrequencyWidget.SetNewData(baseline->Data(), baseline->MetaData());
			delete baseline;
			if(_spatialMetaData != 0)
			{
				delete _spatialMetaData;
				_spatialMetaData = 0;
			}
			if(dynamic_cast<rfiStrategy::SpatialMSImageSet*>(_imageSet) != 0)
			{
				_spatialMetaData = new SpatialMatrixMetaData(static_cast<rfiStrategy::SpatialMSImageSet*>(_imageSet)->SpatialMetaData(*_imageSetIndex));
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
	if(_editStrategyWindow != 0)
		delete _editStrategyWindow;
	_editStrategyWindow = new EditStrategyWindow(*this);
	_editStrategyWindow->show();
}

void MSWindow::onExecuteStrategyPressed()
{
	if(_progressWindow != 0)
		delete _progressWindow;

	ProgressWindow *window = new ProgressWindow(*this);
	_progressWindow = window;
	_progressWindow->show();

	rfiStrategy::ArtifactSet artifacts(&_ioMutex);

	artifacts.SetAntennaFlagCountPlot(new AntennaFlagCountPlot());
	artifacts.SetFrequencyFlagCountPlot(new FrequencyFlagCountPlot());
	artifacts.SetFrequencyPowerPlot(new FrequencyPowerPlot());
	artifacts.SetTimeFlagCountPlot(new TimeFlagCountPlot());
	artifacts.SetIterationsPlot(new IterationsPlot());
	
	artifacts.SetPolarizationStatistics(new PolarizationStatistics());
	artifacts.SetBaselineSelectionInfo(new rfiStrategy::BaselineSelectionInfo());
	artifacts.SetImager(_imagePlaneWindow->GetImager());

	if(HasImage())
	{
		artifacts.SetOriginalData(GetOriginalData());
		artifacts.SetContaminatedData(GetContaminatedData());
		TimeFrequencyData *zero = new TimeFrequencyData(GetOriginalData());
		zero->SetImagesToZero();
		artifacts.SetRevisedData(*zero);
		delete zero;
	}
	if(_timeFrequencyWidget.GetMetaData() != 0)
			artifacts.SetMetaData(_timeFrequencyWidget.GetMetaData());
	if(HasImageSet())
	{
		if(dynamic_cast<rfiStrategy::MSImageSet*>(_imageSet) != 0)
		{
			artifacts.SetImageSet(_imageSet);
			artifacts.SetImageSetIndex(_imageSetIndex);
		}
	}
	rfiStrategy::Strategy::DisableOptimizations(*_strategy);
	try {
		_strategy->StartPerformThread(artifacts, *window);
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
		if(artifacts->PolarizationStatistics()->HasData())
			artifacts->PolarizationStatistics()->Report();
		if(artifacts->IterationsPlot()->HasData())
			artifacts->IterationsPlot()->MakePlot();

		delete artifacts->AntennaFlagCountPlot();
		delete artifacts->FrequencyFlagCountPlot();
		delete artifacts->FrequencyPowerPlot();
		delete artifacts->TimeFlagCountPlot();
		delete artifacts->PolarizationStatistics();
		delete artifacts->BaselineSelectionInfo();
		delete artifacts->IterationsPlot();
		delete artifacts;
	}
}

void MSWindow::onToggleImage()
{
	ImageComparisonWidget::TFImage image = ImageComparisonWidget::TFOriginalImage;
	if(_backgroundImageButton->get_active())
		image = ImageComparisonWidget::TFRevisedImage;
	else if(_diffImageButton->get_active())
		image = ImageComparisonWidget::TFContaminatedImage;
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
	Image2DPtr testSetImaginary(MitigationTester::CreateTestSet(2, rfi, width, height, _gaussianTestSets));
	TimeFrequencyData data(SinglePolarisation, testSetReal, testSetImaginary);
	data.SetGlobalMask(rfi);
	
	_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
	_timeFrequencyWidget.Update();
}

void MSWindow::createToolbar()
{
	_actionGroup = Gtk::ActionGroup::create();
	_actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
	_actionGroup->add( Gtk::Action::create("MenuGo", "_Go") );
	_actionGroup->add( Gtk::Action::create("MenuView", "_View") );
	_actionGroup->add( Gtk::Action::create("MenuPlot", "_Plot") );
	_actionGroup->add( Gtk::Action::create("MenuPlotFlagComparison", "_Compare flags") );
	_actionGroup->add( Gtk::Action::create("MenuActions", "_Actions") );
	_actionGroup->add( Gtk::Action::create("MenuData", "_Data") );
	_actionGroup->add( Gtk::Action::create("OpenFile", Gtk::Stock::OPEN, "Open _file"),
  sigc::mem_fun(*this, &MSWindow::onActionFileOpen) );
	_actionGroup->add( Gtk::Action::create("OpenDirectory", Gtk::Stock::OPEN, "Open _directory"),
  sigc::mem_fun(*this, &MSWindow::onActionDirectoryOpen) );
	_actionGroup->add( Gtk::Action::create("OpenDirectorySpatial", Gtk::Stock::OPEN, "Open _directory as spatial"),
  sigc::mem_fun(*this, &MSWindow::onActionDirectoryOpenForSpatial) );
	_actionGroup->add( Gtk::Action::create("OpenDirectoryST", Gtk::Stock::OPEN, "Open _directory as spatial/time"),
  sigc::mem_fun(*this, &MSWindow::onActionDirectoryOpenForST) );
	_actionGroup->add( Gtk::Action::create("OpenBandCombined", Gtk::Stock::OPEN, "Open/combine bands"),
  sigc::mem_fun(*this, &MSWindow::onOpenBandCombined) );
	_actionGroup->add( Gtk::Action::create("OpenTestSet", "Open _testset") );

	Gtk::RadioButtonGroup testSetGroup;
	_gaussianTestSetsButton = Gtk::RadioAction::create(testSetGroup, "GaussianTestSets", "Gaussian");
	_gaussianTestSetsButton->set_active(true);
	_rayleighTestSetsButton = Gtk::RadioAction::create(testSetGroup, "RayleighTestSets", "Rayleigh");
	_zeroTestSetsButton = Gtk::RadioAction::create(testSetGroup, "ZeroTestSets", "Zero");
	_actionGroup->add(_gaussianTestSetsButton, sigc::mem_fun(*this, &MSWindow::onGaussianTestSets) );
	_actionGroup->add(_rayleighTestSetsButton, sigc::mem_fun(*this, &MSWindow::onRayleighTestSets) );
	_actionGroup->add(_zeroTestSetsButton, sigc::mem_fun(*this, &MSWindow::onZeroTestSets) );
	
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
	_actionGroup->add( Gtk::Action::create("OpenTestSetBStrong", "Test set B (strong RFI)"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetBStrong));
	_actionGroup->add( Gtk::Action::create("OpenTestSetBWeak", "Test set B (weak RFI)"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetBWeak));
	_actionGroup->add( Gtk::Action::create("OpenTestSetBAligned", "Test set B (aligned)"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetBAligned));
	_actionGroup->add( Gtk::Action::create("OpenTestSetGaussianBroadband", "Gaussian broadband"),
	sigc::mem_fun(*this, &MSWindow::onOpenTestSetGaussianBroadband));
	_actionGroup->add( Gtk::Action::create("AddTestModification", "Test modify") );
	_actionGroup->add( Gtk::Action::create("AddStaticFringe", "Static fringe"),
	sigc::mem_fun(*this, &MSWindow::onAddStaticFringe) );
	_actionGroup->add( Gtk::Action::create("Add1SigmaStaticFringe", "Static 1 sigma fringe"),
	sigc::mem_fun(*this, &MSWindow::onAdd1SigmaFringe) );
	_actionGroup->add( Gtk::Action::create("SetToOne", "Set to 1"),
	sigc::mem_fun(*this, &MSWindow::onSetToOne) );
	_actionGroup->add( Gtk::Action::create("SetToI", "Set to i"),
	sigc::mem_fun(*this, &MSWindow::onSetToI) );
	_actionGroup->add( Gtk::Action::create("SetToOnePlusI", "Set to 1+i"),
	sigc::mem_fun(*this, &MSWindow::onSetToOnePlusI) );
	_actionGroup->add( Gtk::Action::create("MultiplyData", "Multiply data..."),
	sigc::mem_fun(*this, &MSWindow::onMultiplyData) );
	_actionGroup->add( Gtk::Action::create("Compress", "Compress"),
	sigc::mem_fun(*this, &MSWindow::onCompress) );
	_actionGroup->add( Gtk::Action::create("Quit", Gtk::Stock::QUIT),
	sigc::mem_fun(*this, &MSWindow::onQuit) );

	_actionGroup->add( Gtk::Action::create("ImageProperties", "Plot properties..."),
  	sigc::mem_fun(*this, &MSWindow::onImagePropertiesPressed) );
	Gtk::RadioButtonGroup mapGroup;
	_mapBWButton = Gtk::RadioAction::create(mapGroup, "MapBW", "BW map");
	_mapBWButton->set_active(true);
	_mapInvertedButton = Gtk::RadioAction::create(mapGroup, "MapInverted", "Inverted map");
	_mapColorButton = Gtk::RadioAction::create(mapGroup, "MapColor", "Color map");
	_mapRedBlueButton = Gtk::RadioAction::create(mapGroup, "MapRedBlue", "Red-blue map");
	_mapRedYellowBlueButton = Gtk::RadioAction::create(mapGroup, "MapRedYellowBlue", "Red-yellow-blue map");
	
	_actionGroup->add(_mapBWButton, sigc::mem_fun(*this, &MSWindow::onToggleMap) );
	_actionGroup->add(_mapInvertedButton, sigc::mem_fun(*this, &MSWindow::onToggleMap) );
	_actionGroup->add(_mapColorButton, sigc::mem_fun(*this, &MSWindow::onToggleMap) );
	_actionGroup->add(_mapRedBlueButton, sigc::mem_fun(*this, &MSWindow::onToggleMap) );
	_actionGroup->add(_mapRedYellowBlueButton, sigc::mem_fun(*this, &MSWindow::onToggleMap) );
	
	_useLogScaleButton = Gtk::ToggleAction::create("UseLogScale", "Use log scale");
	_actionGroup->add(_useLogScaleButton, sigc::mem_fun(*this, &MSWindow::onToggleUseLogScale) );
	_showAxisDescriptionsButton = Gtk::ToggleAction::create("ShowAxisDescriptions", "Show axis descriptions");
	_actionGroup->add(_showAxisDescriptionsButton, sigc::mem_fun(*this, &MSWindow::onToggleShowAxisDescriptions) );
	_timeGraphButton = Gtk::ToggleAction::create("TimeGraph", "Time graph");
	_timeGraphButton->set_active(false); 
	_actionGroup->add(_timeGraphButton, sigc::mem_fun(*this, &MSWindow::onTimeGraphButtonPressed) );
	_actionGroup->add( Gtk::Action::create("ShowAntennaMapWindow", "Show antenna map"), sigc::mem_fun(*this, &MSWindow::onShowAntennaMapWindow) );
	_actionGroup->add( Gtk::Action::create("ExportImage", "Export image"),
	sigc::mem_fun(*this, &MSWindow::onExportImage) );
	
	Gtk::RadioButtonGroup rangeGroup;
	_rangeFullButton = Gtk::RadioAction::create(rangeGroup, "RangeMinMax", "Min-max range");
	_rangeWinsorizedButton = Gtk::RadioAction::create(rangeGroup, "RangeWinsorized", "Winsorized range");
	_rangeSpecifiedButton = Gtk::RadioAction::create(rangeGroup, "RangeSpecified", "Specified range");
	_rangeWinsorizedButton->set_active(true); 
	_actionGroup->add(_rangeFullButton, sigc::mem_fun(*this, &MSWindow::onRangeChanged) );
	_actionGroup->add(_rangeWinsorizedButton, sigc::mem_fun(*this, &MSWindow::onRangeChanged) );
	_actionGroup->add(_rangeSpecifiedButton, sigc::mem_fun(*this, &MSWindow::onRangeChanged) );

	_actionGroup->add( Gtk::Action::create("PlotDist", "Plot _distribution"),
  sigc::mem_fun(*this, &MSWindow::onPlotDistPressed) );
	_actionGroup->add( Gtk::Action::create("PlotComplexPlane", "Plot _complex plane"),
  sigc::mem_fun(*this, &MSWindow::onPlotComplexPlanePressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerSpectrum", "Plot _power spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerSpectrumPressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerSpectrumComparison", "_Power spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerSpectrumComparisonPressed) );
	_actionGroup->add( Gtk::Action::create("PlotRMSSpectrum", "Plot _rms spectrum"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerRMSPressed) );
	_actionGroup->add( Gtk::Action::create("PlotSNRSpectrum", "Plot spectrum snr"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerSNRPressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerTime", "Plot power vs _time"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerTimePressed) );
	_actionGroup->add( Gtk::Action::create("PlotPowerTimeComparison", "Power vs _time"),
  sigc::mem_fun(*this, &MSWindow::onPlotPowerTimeComparisonPressed) );
	_actionGroup->add( Gtk::Action::create("PlotTimeScatter", "Plot time scatter"),
  sigc::mem_fun(*this, &MSWindow::onPlotTimeScatterPressed) );
	_actionGroup->add( Gtk::Action::create("PlotTimeScatterComparison", "Time _scatter"),
  sigc::mem_fun(*this, &MSWindow::onPlotTimeScatterComparisonPressed) );
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
	_actionGroup->add( Gtk::Action::create("SetAndShowImagePlane", "S_et and show image plane"),
  sigc::mem_fun(*this, &MSWindow::onSetAndShowImagePlane) );
	_actionGroup->add( Gtk::Action::create("AddToImagePlane", "Add to _image plane"),
  sigc::mem_fun(*this, &MSWindow::onAddToImagePlane) );
	
	Gtk::RadioButtonGroup setGroup;
	_ncpSetButton = Gtk::RadioAction::create(setGroup, "NCPSet", "Use NCP set");
	_b1834SetButton = Gtk::RadioAction::create(setGroup, "B1834Set", "Use B1834 set");
	_emptySetButton = Gtk::RadioAction::create(setGroup, "EmptySet", "Use empty set");
	_ncpSetButton->set_active(true); 
	_actionGroup->add(_ncpSetButton);
	_actionGroup->add(_b1834SetButton);
	_actionGroup->add(_emptySetButton);
	
	_actionGroup->add( Gtk::Action::create("SimulateCorrelation", "Simulate correlation"),
  sigc::mem_fun(*this, &MSWindow::onSimulateCorrelation) );
	_actionGroup->add( Gtk::Action::create("SimulateSourceSetA", "Simulate source set A"),
  sigc::mem_fun(*this, &MSWindow::onSimulateSourceSetA) );
	_actionGroup->add( Gtk::Action::create("SimulateSourceSetB", "Simulate source set B"),
  sigc::mem_fun(*this, &MSWindow::onSimulateSourceSetB) );
	_actionGroup->add( Gtk::Action::create("SimulateSourceSetC", "Simulate source set C"),
  sigc::mem_fun(*this, &MSWindow::onSimulateSourceSetC) );
	_actionGroup->add( Gtk::Action::create("SimulateSourceSetD", "Simulate source set D"),
  sigc::mem_fun(*this, &MSWindow::onSimulateSourceSetD) );
	_actionGroup->add( Gtk::Action::create("SimulateSourceSetALarge", "Simulate source set A+"),
  sigc::mem_fun(*this, &MSWindow::onSimulateSourceSetALarge) );
	_actionGroup->add( Gtk::Action::create("SimulateOffAxisSource", "Simulate off-axis source"),
  sigc::mem_fun(*this, &MSWindow::onSimulateOffAxisSource) );
	_actionGroup->add( Gtk::Action::create("SimulateOnAxisSource", "Simulate on-axis source"),
  sigc::mem_fun(*this, &MSWindow::onSimulateOnAxisSource) );

	_actionGroup->add( Gtk::Action::create("EditStrategy", "_Edit strategy"),
  sigc::mem_fun(*this, &MSWindow::onEditStrategyPressed) );
	_actionGroup->add( Gtk::Action::create("ExecuteStrategy", "E_xecute strategy"),
  sigc::mem_fun(*this, &MSWindow::onExecuteStrategyPressed) );
	_actionGroup->add( Gtk::Action::create("ShowStats", "Show _stats"),
  sigc::mem_fun(*this, &MSWindow::onShowStats) );
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

	_actionGroup->add( Gtk::Action::create("ShowReal", "Keep _real part"),
  sigc::mem_fun(*this, &MSWindow::onShowRealPressed) );
	_actionGroup->add( Gtk::Action::create("ShowImaginary", "Keep _imaginary part"),
  sigc::mem_fun(*this, &MSWindow::onShowImaginaryPressed) );
	_actionGroup->add( Gtk::Action::create("ShowPhase", "Keep _phase part"),
  sigc::mem_fun(*this, &MSWindow::onShowPhasePressed) );
	_actionGroup->add( Gtk::Action::create("ShowStokesI", "Keep _stokesI part"),
  sigc::mem_fun(*this, &MSWindow::onShowStokesIPressed) );
	_actionGroup->add( Gtk::Action::create("ShowStokesQ", "Keep stokes_Q part"),
  sigc::mem_fun(*this, &MSWindow::onShowStokesQPressed) );
	_actionGroup->add( Gtk::Action::create("ShowStokesU", "Keep stokes_U part"),
  sigc::mem_fun(*this, &MSWindow::onShowStokesUPressed) );
	_actionGroup->add( Gtk::Action::create("ShowStokesV", "Keep stokes_V part"),
  sigc::mem_fun(*this, &MSWindow::onShowStokesVPressed) );
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

	_actionGroup->add( Gtk::Action::create("Segment", "Segment"),
  sigc::mem_fun(*this, &MSWindow::onSegment) );
	_actionGroup->add( Gtk::Action::create("Cluster", "Cluster"),
  sigc::mem_fun(*this, &MSWindow::onCluster) );
	_actionGroup->add( Gtk::Action::create("Classify", "Classify"),
  sigc::mem_fun(*this, &MSWindow::onClassify) );
	_actionGroup->add( Gtk::Action::create("RemoveSmallSegments", "Remove small segments"),
  sigc::mem_fun(*this, &MSWindow::onRemoveSmallSegments) );
	_actionGroup->add( Gtk::Action::create("StoreData", "Store"),
  sigc::mem_fun(*this, &MSWindow::onStoreData) );
	_actionGroup->add( Gtk::Action::create("RecallData", "Recall"),
  sigc::mem_fun(*this, &MSWindow::onRecallData) );
	_actionGroup->add( Gtk::Action::create("SubtractDataFromMem", "Subtract from mem"),
  sigc::mem_fun(*this, &MSWindow::onSubtractDataFromMem) );

	_actionGroup->add( Gtk::Action::create("Highlight", "Highlight"),
  sigc::mem_fun(*this, &MSWindow::onHightlightPressed) );
	_actionGroup->add( Gtk::Action::create("TimeMergeUnsetValues", "Merge unset values in time"),
  sigc::mem_fun(*this, &MSWindow::onTimeMergeUnsetValues) );
	_actionGroup->add( Gtk::Action::create("VertEVD", "Vert EVD"),
  sigc::mem_fun(*this, &MSWindow::onVertEVD) );
	_actionGroup->add( Gtk::Action::create("ApplyTimeProfile", "Apply time profile"),
  sigc::mem_fun(*this, &MSWindow::onApplyTimeProfile) );
	_actionGroup->add( Gtk::Action::create("ApplyVertProfile", "Apply vert profile"),
  sigc::mem_fun(*this, &MSWindow::onApplyVertProfile) );
	_actionGroup->add( Gtk::Action::create("RestoreTimeProfile", "Restore time profile"),
  sigc::mem_fun(*this, &MSWindow::onRestoreTimeProfile) );
	_actionGroup->add( Gtk::Action::create("RestoreVertProfile", "Restore vert profile"),
  sigc::mem_fun(*this, &MSWindow::onRestoreVertProfile) );
	_actionGroup->add( Gtk::Action::create("ReapplyTimeProfile", "Reapply time profile"),
  sigc::mem_fun(*this, &MSWindow::onReapplyTimeProfile) );
	_actionGroup->add( Gtk::Action::create("ReapplyVertProfile", "Reapply vert profile"),
  sigc::mem_fun(*this, &MSWindow::onReapplyVertProfile) );

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
    "      <menuitem action='OpenDirectoryST'/>"
    "      <menuitem action='OpenBandCombined'/>"
    "      <menu action='OpenTestSet'>"
		"        <menuitem action='GaussianTestSets'/>"
		"        <menuitem action='RayleighTestSets'/>"
		"        <menuitem action='ZeroTestSets'/>"
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
		"        <menuitem action='OpenTestSetBStrong'/>"
		"        <menuitem action='OpenTestSetBWeak'/>"
		"        <menuitem action='OpenTestSetBAligned'/>"
		"        <menuitem action='OpenTestSetGaussianBroadband'/>"
		"      </menu>"
		"      <menu action='AddTestModification'>"
		"        <menuitem action='AddStaticFringe'/>"
		"        <menuitem action='Add1SigmaStaticFringe'/>"
		"        <menuitem action='SetToOne'/>"
		"        <menuitem action='SetToI'/>"
		"        <menuitem action='SetToOnePlusI'/>"
		"        <menuitem action='MultiplyData'/>"
		"      </menu>"
    "      <menuitem action='Compress'/>"
    "      <menuitem action='Quit'/>"
    "    </menu>"
	  "    <menu action='MenuView'>"
    "      <menuitem action='ImageProperties'/>"
    "      <menuitem action='ShowAntennaMapWindow'/>"
    "      <separator/>"
    "      <menuitem action='MapBW'/>"
    "      <menuitem action='MapInverted'/>"
    "      <menuitem action='MapColor'/>"
    "      <menuitem action='MapRedBlue'/>"
    "      <menuitem action='MapRedYellowBlue'/>"
    "      <separator/>"
    "      <menuitem action='ShowAxisDescriptions'/>"
    "      <menuitem action='UseLogScale'/>"
    "      <menuitem action='TimeGraph'/>"
    "      <separator/>"
    "      <menuitem action='RangeMinMax'/>"
    "      <menuitem action='RangeWinsorized'/>"
    "      <menuitem action='RangeSpecified'/>"
    "      <separator/>"
    "      <menuitem action='ExportImage'/>"
	  "    </menu>"
	  "    <menu action='MenuPlot'>"
    "      <menu action='MenuPlotFlagComparison'>"
    "        <menuitem action='PlotPowerSpectrumComparison'/>"
    "        <menuitem action='PlotPowerTimeComparison'/>"
    "        <menuitem action='PlotTimeScatterComparison'/>"
		"      </menu>"
    "      <separator/>"
    "      <menuitem action='PlotDist'/>"
    "      <menuitem action='PlotComplexPlane'/>"
    "      <menuitem action='PlotPowerSpectrum'/>"
    "      <menuitem action='PlotRMSSpectrum'/>"
    "      <menuitem action='PlotSNRSpectrum'/>"
    "      <menuitem action='PlotPowerTime'/>"
    "      <menuitem action='PlotTimeScatter'/>"
    "      <menuitem action='PlotSingularValues'/>"
    "      <menuitem action='PlotSNRToFitVariance'/>"
    "      <menuitem action='PlotQuality25'/>"
    "      <menuitem action='PlotQualityAll'/>"
    "      <separator/>"
    "      <menuitem action='ShowImagePlane'/>"
    "      <menuitem action='SetAndShowImagePlane'/>"
    "      <menuitem action='AddToImagePlane'/>"
    "      <separator/>"
    "      <menuitem action='NCPSet'/>"
    "      <menuitem action='B1834Set'/>"
    "      <menuitem action='EmptySet'/>"
    "      <menuitem action='SimulateCorrelation'/>"
    "      <menuitem action='SimulateSourceSetA'/>"
    "      <menuitem action='SimulateSourceSetB'/>"
    "      <menuitem action='SimulateSourceSetC'/>"
    "      <menuitem action='SimulateSourceSetD'/>"
    "      <menuitem action='SimulateSourceSetALarge'/>"
    "      <menuitem action='SimulateOffAxisSource'/>"
    "      <menuitem action='SimulateOnAxisSource'/>"
	  "    </menu>"
    "    <menu action='MenuGo'>"
    "      <menuitem action='LargeStepPrevious'/>"
    "      <menuitem action='Previous'/>"
    "      <menuitem action='Next'/>"
    "      <menuitem action='LargeStepNext'/>"
    "      <separator/>"
    "      <menuitem action='GoTo'/>"
    "    </menu>"
	  "    <menu action='MenuData'>"
    "      <menuitem action='DiffToOriginal'/>"
    "      <menuitem action='BackToOriginal'/>"
    "      <separator/>"
    "      <menuitem action='ShowReal'/>"
    "      <menuitem action='ShowImaginary'/>"
    "      <menuitem action='ShowPhase'/>"
    "      <separator/>"
    "      <menuitem action='ShowStokesI'/>"
    "      <menuitem action='ShowStokesQ'/>"
    "      <menuitem action='ShowStokesU'/>"
    "      <menuitem action='ShowStokesV'/>"
    "      <separator/>"
    "      <menuitem action='ShowXX'/>"
    "      <menuitem action='ShowXY'/>"
    "      <menuitem action='ShowYX'/>"
    "      <menuitem action='ShowYY'/>"
    "      <menuitem action='ShowAutoPol'/>"
    "      <menuitem action='ShowCrossPol'/>"
    "      <menuitem action='UnrollPhase'/>"
    "      <separator/>"
    "      <menuitem action='StoreData'/>"
    "      <menuitem action='RecallData'/>"
    "      <menuitem action='SubtractDataFromMem'/>"
	  "    </menu>"
	  "    <menu action='MenuActions'>"
    "      <menuitem action='EditStrategy'/>"
    "      <menuitem action='ExecuteStrategy'/>"
    "      <separator/>"
    "      <menuitem action='Segment'/>"
    "      <menuitem action='Cluster'/>"
    "      <menuitem action='Classify'/>"
    "      <menuitem action='RemoveSmallSegments'/>"
    "      <separator/>"
    "      <menuitem action='TimeMergeUnsetValues'/>"
    "      <menuitem action='VertEVD'/>"
    "      <menuitem action='ApplyTimeProfile'/>"
    "      <menuitem action='ApplyVertProfile'/>"
    "      <menuitem action='RestoreTimeProfile'/>"
    "      <menuitem action='RestoreVertProfile'/>"
    "      <menuitem action='ReapplyTimeProfile'/>"
    "      <menuitem action='ReapplyVertProfile'/>"
	  "    </menu>"
    "  </menubar>"
    "  <toolbar  name='ToolBar'>"
    "    <toolitem action='OpenDirectory'/>"
    "    <separator/>"
    "    <toolitem action='ClearAltFlags'/>"
    "    <toolitem action='ShowStats'/>"
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
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
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
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
	}
	if(_originalImageButton->get_active())
		_timeFrequencyWidget.Update();
	else
		_originalImageButton->set_active();
}

void MSWindow::onHightlightPressed()
{
	if(_highlightWindow != 0)
		delete _highlightWindow;
	_highlightWindow = new HighlightWindow(*this);
	_highlightWindow->show();
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
			num_t mean, stddev;
			TimeFrequencyData data(GetActiveData());
			ThresholdTools::MeanAndStdDev(data.GetRealPart(), data.GetSingleMask(), mean, stddev);
			FringeTestCreater::AddStaticFringe(data, metaData, stddev);
			_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
			_timeFrequencyWidget.Update();
		}
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onSetToOne()
{
	try {
		TimeFrequencyData data(GetActiveData());
		std::pair<Image2DCPtr, Image2DCPtr> images = data.GetSingleComplexImage();
		Image2DPtr
			real = Image2D::CreateCopy(images.first),
			imaginary = Image2D::CreateCopy(images.first);
		real->SetAll(1.0);
		imaginary->SetAll(0.0);
		TimeFrequencyData newData(data.Polarisation(), real, imaginary);
		_timeFrequencyWidget.SetNewData(newData, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onSetToI()
{
	try {
		TimeFrequencyData data(GetActiveData());
		std::pair<Image2DCPtr, Image2DCPtr> images = data.GetSingleComplexImage();
		Image2DPtr
			real = Image2D::CreateCopy(images.first),
			imaginary = Image2D::CreateCopy(images.first);
		real->SetAll(0.0);
		imaginary->SetAll(1.0);
		TimeFrequencyData newData(data.Polarisation(), real, imaginary);
		_timeFrequencyWidget.SetNewData(newData, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onSetToOnePlusI()
{
	try {
		TimeFrequencyData data(GetActiveData());
		std::pair<Image2DCPtr, Image2DCPtr> images = data.GetSingleComplexImage();
		Image2DPtr
			real = Image2D::CreateCopy(images.first),
			imaginary = Image2D::CreateCopy(images.first);
		real->SetAll(1.0);
		imaginary->SetAll(1.0);
		TimeFrequencyData newData(data.Polarisation(), real, imaginary);
		_timeFrequencyWidget.SetNewData(newData, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	} catch(std::exception &e)
	{
		showError(e.what());
	}
}

void MSWindow::onShowStats()
{
	if(_timeFrequencyWidget.HasImage())
	{
		TimeFrequencyData activeData = GetActiveData();
		TimeFrequencyStatistics statistics(activeData);
		std::stringstream s;
		s << "Percentage flagged: " << TimeFrequencyStatistics::FormatRatio(statistics.GetFlaggedRatio()) << "\n";
			
		Mask2DCPtr
			original = _timeFrequencyWidget.OriginalMask(),
			alternative = _timeFrequencyWidget.AlternativeMask();
		Mask2DPtr
			intersect = Mask2D::CreateCopy(original);
		intersect->Intersect(alternative);
		unsigned intCount = intersect->GetCount<true>();
		if(intCount != 0)
		{
			if(!original->Equals(alternative))
			{
				s << "Overlap between original and alternative: " << TimeFrequencyStatistics::FormatRatio((double) intCount / ((double) (original->Width() * original->Height()))) << "\n"
				<< "(relative to alternative flags: " << TimeFrequencyStatistics::FormatRatio((double) intCount / ((double) (alternative->GetCount<true>()))) << ")\n";
				
			}
		}
		
		Image2DCPtr powerImg = activeData.GetSingleImage();	
		Mask2DCPtr mask = activeData.GetSingleMask();
		double power = 0.0;
		for(unsigned y=0;y<powerImg->Height();++y)
		{
			for(unsigned x=0;x<powerImg->Width();++x)
			{
				if(!mask->Value(x, y) && std::isfinite(powerImg->Value(x, y)))
				{
					power += powerImg->Value(x, y);
				}
			}
		}
		s
			<< "Total unflagged power: " << power << "\n";

		Gtk::MessageDialog dialog(*this, s.str(), false, Gtk::MESSAGE_INFO);
		dialog.run();
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
		if(_plotComplexPlaneWindow != 0)
			delete _plotComplexPlaneWindow;
		_plotComplexPlaneWindow = new ComplexPlanePlotWindow(*this);
		_plotComplexPlaneWindow->show();
	}
}

void MSWindow::onPlotPowerSpectrumPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot plot("Power-spectrum.pdf");

		TimeFrequencyData data = _timeFrequencyWidget.GetActiveData();
		Image2DCPtr image = data.GetSingleImage();
		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(image->Width(), image->Height());
		plot.StartLine("Before");
		RFIPlots::MakePowerSpectrumPlot(plot, image, mask, _timeFrequencyWidget.GetMetaData());

		mask = Mask2D::CreateCopy(data.GetSingleMask());
		if(!mask->AllFalse())
		{
			plot.StartLine("After");
			RFIPlots::MakePowerSpectrumPlot(plot, image, mask, _timeFrequencyWidget.GetMetaData());
		}

		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotPowerSpectrumComparisonPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot plot("Power-spectrum-comparison.pdf");

		TimeFrequencyData data = _timeFrequencyWidget.OriginalData();
		Image2DCPtr image = data.GetSingleImage();
		Mask2DCPtr mask = data.GetSingleMask();
		plot.StartLine("Original");
		RFIPlots::MakePowerSpectrumPlot(plot, image, mask, _timeFrequencyWidget.GetMetaData());

		data = _timeFrequencyWidget.ContaminatedData();
		image = data.GetSingleImage();
		mask = data.GetSingleMask();
		plot.StartLine("Alternative");
		RFIPlots::MakePowerSpectrumPlot(plot, image, mask, _timeFrequencyWidget.GetMetaData());
	
		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotPowerRMSPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot plot("Spectrum-rms.pdf");

		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(_timeFrequencyWidget.Image()->Width(), _timeFrequencyWidget.Image()->Height());
		plot.StartLine("Before");		
		RFIPlots::MakeRMSSpectrumPlot(plot, _timeFrequencyWidget.Image(), mask);

		mask = Mask2D::CreateCopy(_timeFrequencyWidget.GetActiveData().GetSingleMask());
		if(!mask->AllFalse())
		{
			plot.StartLine("After");
			RFIPlots::MakeRMSSpectrumPlot(plot, _timeFrequencyWidget.Image(), mask);
	
			//mask->Invert();
			//plot.StartLine("RFI");
			//RFIPlots::MakeRMSSpectrumPlot(plot, _timeFrequencyWidget.Image(), mask);
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
		Plot plot("Spectrum-snr.pdf");

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
		Plot plot("Time.pdf");

		Mask2DPtr mask =
			Mask2D::CreateSetMaskPtr<false>(_timeFrequencyWidget.Image()->Width(), _timeFrequencyWidget.Image()->Height());
		plot.StartLine("Total");		
		RFIPlots::MakePowerTimePlot(plot, _timeFrequencyWidget.Image(), mask, _timeFrequencyWidget.GetMetaData());

		mask = Mask2D::CreateCopy(_timeFrequencyWidget.GetActiveData().GetSingleMask());
		if(!mask->AllFalse())
		{
			plot.StartLine("Uncontaminated");
			RFIPlots::MakePowerTimePlot(plot, _timeFrequencyWidget.Image(), mask, _timeFrequencyWidget.GetMetaData());
	
			mask->Invert();
			plot.StartLine("RFI");
			RFIPlots::MakePowerTimePlot(plot, _timeFrequencyWidget.Image(), mask, _timeFrequencyWidget.GetMetaData());
		}

		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotPowerTimeComparisonPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		Plot plot("Time-comparison.pdf");

		TimeFrequencyData data = _timeFrequencyWidget.OriginalData();
		Mask2DCPtr mask = data.GetSingleMask();
		Image2DCPtr image = data.GetSingleImage();
		plot.StartLine("Original");
		RFIPlots::MakePowerTimePlot(plot, image, mask, _timeFrequencyWidget.GetMetaData());

		data = _timeFrequencyWidget.ContaminatedData();
		mask = data.GetSingleMask();
		image = data.GetSingleImage();
		plot.StartLine("Alternative");
		RFIPlots::MakePowerTimePlot(plot, image, mask, _timeFrequencyWidget.GetMetaData());

		plot.Close();
		plot.Show();
	}
}

void MSWindow::onPlotTimeScatterPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		MultiPlot plot("Scatter.pdf", 4);
		RFIPlots::MakeScatterPlot(plot, GetActiveData(), _timeFrequencyWidget.GetMetaData());
		plot.Finish();
		plot.Show();
	}
}

void MSWindow::onPlotTimeScatterComparisonPressed()
{
	if(_timeFrequencyWidget.HasImage())
	{
		MultiPlot plot("Scatter-comparison.pdf", 8);
		RFIPlots::MakeScatterPlot(plot, GetOriginalData(), _timeFrequencyWidget.GetMetaData(), 0);
		RFIPlots::MakeScatterPlot(plot, GetContaminatedData(), _timeFrequencyWidget.GetMetaData(), 4);
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
	fitter.SetMetaData(_timeFrequencyWidget.GetMetaData());
	
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

void MSWindow::onImagePropertiesPressed()
{
	if(HasImage())
	{
		if(_imagePropertiesWindow != 0)
			delete _imagePropertiesWindow;
		_imagePropertiesWindow = new ImagePropertiesWindow(_timeFrequencyWidget, "Time-frequency plotting options");
		_imagePropertiesWindow->show();
	}
}

void MSWindow::showPhasePart(enum TimeFrequencyData::PhaseRepresentation phaseRepresentation)
{
	if(HasImage())
	{
		try {
			TimeFrequencyData *newPart =  _timeFrequencyWidget.GetActiveData().CreateTFData(phaseRepresentation);
			_timeFrequencyWidget.SetNewData(*newPart, _timeFrequencyWidget.GetMetaData());
			delete newPart;
			_timeFrequencyWidget.Update();
		} catch(std::exception &e)
		{
			showError(e.what());
		}
	}
}

void MSWindow::showPolarisation(enum PolarisationType polarisation)
{
	if(HasImage())
	{
		try {
			TimeFrequencyData *newData =
				_timeFrequencyWidget.GetActiveData().CreateTFData(polarisation);
			_timeFrequencyWidget.SetNewData(*newData, _timeFrequencyWidget.GetMetaData());
			delete newData;
			_timeFrequencyWidget.Update();
		} catch(std::exception &e)
		{
			showError(e.what());
		}
	}
}

void MSWindow::onGoToPressed()
{
	if(HasImageSet())
	{
		rfiStrategy::MSImageSet *msSet = dynamic_cast<rfiStrategy::MSImageSet*>(_imageSet);
		if(msSet != 0)
		{
			if(_gotoWindow != 0)
				delete _gotoWindow;
			_gotoWindow = new GoToWindow(*this);
			_gotoWindow->show();
			} else {
			showError("Can not goto in this image set; format does not support goto");
		}
	}
}

void MSWindow::onTFWidgetMouseMoved(size_t x, size_t y)
{
	if(_timeFrequencyWidget.GetMetaData() != 0)
	{
		Image2DCPtr image = _timeFrequencyWidget.Image();
		num_t v = image->Value(x, y);
		_statusbar.pop();
		std::stringstream s;
			s << "x=" << x << ",y=" << y << ",value=" << v;
			const std::vector<double> &times = _timeFrequencyWidget.GetMetaData()->ObservationTimes();
			s << " (t=" << Date::AipsMJDToString(times[x]) <<
			", f=" << Frequency::ToString(_timeFrequencyWidget.GetMetaData()->Band().channels[y].frequencyHz);
		if(_timeFrequencyWidget.GetMetaData()->HasUVW())
		{
			UVW uvw = _timeFrequencyWidget.GetMetaData()->UVW()[x];
			s << ", uvw=" << uvw.u << "," << uvw.v << "," << uvw.w;
		}
		s << ')';
		_statusbar.push(s.str(), 0);
	}
}

void MSWindow::onShowImagePlane()
{
	_imagePlaneWindow->show();
}

void MSWindow::onSetAndShowImagePlane()
{
	_imagePlaneWindow->GetImager()->Empty();
	onAddToImagePlane();
	_imagePlaneWindow->show();
	_imagePlaneWindow->GetImager()->ApplyWeightsToUV();
	_imagePlaneWindow->Update();
}

void MSWindow::onAddToImagePlane()
{
	try {
		if(_timeFrequencyWidget.GetMetaData() != 0 && _timeFrequencyWidget.GetMetaData()->HasUVW())
		{
			TimeFrequencyData activeData = GetActiveData();
			if(activeData.PolarisationCount() != 1)
			{
				TimeFrequencyData *singlePolarization = activeData.CreateTFData(StokesIPolarisation);
				activeData = *singlePolarization;
				delete singlePolarization;
			}
			_imagePlaneWindow->AddData(activeData, _timeFrequencyWidget.GetMetaData());
		}
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
	_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
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

void MSWindow::onClassify()
{
	if(_segmentedImage != 0)
	{
		Morphology morphology;
		morphology.Classify(_segmentedImage);
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

void MSWindow::onTFWidgetButtonReleased(size_t x, size_t y)
{
	if(HasImage())
	{
		if(_plotFrame.is_visible())
		{
			_plotFrame.SetTimeFrequencyData(GetActiveData());
			_plotFrame.SetSelectedSample(x, y);
		
			_plotFrame.Update();
		}
	}
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
		_timeFrequencyWidget.SetNewData(*data, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
		delete data;
	}
}

void MSWindow::showError(const std::string &description)
{
	Gtk::MessageDialog dialog(*this, description, false, Gtk::MESSAGE_ERROR);
	dialog.run();
}

DefaultModels::SetLocation MSWindow::getSetLocation(bool empty)
{
	if(empty)
		return DefaultModels::EmptySet;
	if(_ncpSetButton->get_active())
		return DefaultModels::NCPSet;
	else if(_b1834SetButton->get_active())
		return DefaultModels::B1834Set;
	else
		return DefaultModels::EmptySet;
}

void MSWindow::loadDefaultModel(DefaultModels::Distortion distortion, bool withNoise, bool empty, unsigned channelCount)
{
	std::pair<TimeFrequencyData, TimeFrequencyMetaDataPtr> pair = DefaultModels::LoadSet(getSetLocation(empty), distortion, withNoise ? 1.0 : 0.0, channelCount);
	TimeFrequencyData data = pair.first;
	TimeFrequencyMetaDataCPtr metaData = pair.second;
	
	_timeFrequencyWidget.SetNewData(data, metaData);
	_timeFrequencyWidget.Update();
}

void MSWindow::onCompress()
{
	Compress compress = Compress(GetActiveData());
	compress.AllToStdOut();
}

void MSWindow::onToggleShowAxisDescriptions()
{
	_timeFrequencyWidget.SetShowAxisDescriptions(_showAxisDescriptionsButton->get_active());
	_timeFrequencyWidget.Update();
}

void MSWindow::onToggleUseLogScale()
{
	if(_useLogScaleButton->get_active())
		_timeFrequencyWidget.SetScaleOption(ImageWidget::LogScale);
	else
		_timeFrequencyWidget.SetScaleOption(ImageWidget::NormalScale);
	_timeFrequencyWidget.Update();
}

void MSWindow::onRangeChanged()
{
	if(_rangeFullButton->get_active())
		_timeFrequencyWidget.SetRange(ImageWidget::MinMax);
	else if(_rangeWinsorizedButton->get_active())
		_timeFrequencyWidget.SetRange(ImageWidget::Winsorized);
	else
	{
		if(_timeFrequencyWidget.Range() != ImageWidget::Specified)
		{
			NumInputDialog minDialog("Set range", "Minimum value:", _timeFrequencyWidget.Min());
			int result = minDialog.run();
			if(result == Gtk::RESPONSE_OK)
			{
				double min = minDialog.Value();
				NumInputDialog maxDialog("Set range", "Maximum value:", _timeFrequencyWidget.Max());
				result = maxDialog.run();
				if(result == Gtk::RESPONSE_OK)
				{
					double max = maxDialog.Value();
					_timeFrequencyWidget.SetRange(ImageWidget::Specified);
					_timeFrequencyWidget.SetMin(min);
					_timeFrequencyWidget.SetMax(max);
				}
			}
		}
	}
	_timeFrequencyWidget.Update();
}

void MSWindow::onShowAntennaMapWindow()
{
	if(_antennaMapWindow != 0)
		delete _antennaMapWindow;
	AntennaMapWindow *newWindow = new AntennaMapWindow();
	_antennaMapWindow = newWindow;
	rfiStrategy::MSImageSet *msImageSet = dynamic_cast<rfiStrategy::MSImageSet*>(_imageSet);
	if(msImageSet != 0)
	{
		MeasurementSet &set = msImageSet->Reader()->Set();
		newWindow->SetMeasurementSet(set);
	}
	newWindow->show();
}

void MSWindow::onVertEVD()
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		TimeFrequencyData old(data);
		VertEVD::Perform(data, true);
		TimeFrequencyData *diff = TimeFrequencyData::CreateTFDataFromDiff(old, data);
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.SetRevisedData(*diff);
		delete diff;
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onApplyTimeProfile()
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		if(_horProfile.size() != data.ImageWidth())
		{
			_horProfile.clear();
			for(unsigned i=0;i<data.ImageWidth();++i)
				_horProfile.push_back(1.0);
		}
		
		Image2DCPtr weights = data.GetSingleImage();
		for(unsigned i=0;i<data.ImageCount();++i)
		{
			Image2DCPtr input = data.GetImage(i);
			Image2DPtr output = Image2D::CreateEmptyImagePtr(input->Width(), input->Height());
			for(unsigned x=0;x<weights->Width();++x)
			{
				num_t timeAvg = 0.0;
				for(unsigned y=0;y<weights->Height();++y)
				{
					if(std::isfinite(weights->Value(x, y)))
						timeAvg += weights->Value(x, y);
				}
				timeAvg /= (num_t) weights->Height();
				_horProfile[x] = timeAvg;
				for(unsigned y=0;y<input->Height();++y)
				{
					output->SetValue(x, y, input->Value(x, y) * timeAvg);
				}
			}
			data.SetImage(i, output);
		}
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onApplyVertProfile()
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		if(_vertProfile.size() != data.ImageHeight())
		{
			_vertProfile.clear();
			for(unsigned i=0;i<data.ImageHeight();++i)
				_vertProfile.push_back(1.0);
		}

		Image2DCPtr weights = data.GetSingleImage();
		for(unsigned i=0;i<data.ImageCount();++i)
		{
			Image2DCPtr input = data.GetImage(i);
			Image2DPtr output = Image2D::CreateEmptyImagePtr(input->Width(), input->Height());
			for(unsigned y=0;y<weights->Height();++y)
			{
				num_t vertAvg = 0.0;
				for(unsigned x=0;x<weights->Width();++x)
				{
					if(std::isfinite(weights->Value(x, y)))
						vertAvg += weights->Value(x, y);
				}
				vertAvg /= (num_t) weights->Width();
				_vertProfile[y] = vertAvg;
				for(unsigned x=0;x<input->Width();++x)
				{
					output->SetValue(x, y, input->Value(x, y) * vertAvg);
				}
			}
			data.SetImage(i, output);
		}
		_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onUseTimeProfile(bool inverse)
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		if(_horProfile.size()==data.ImageWidth())
		{
			for(unsigned i=0;i<data.ImageCount();++i)
			{
				Image2DCPtr input = data.GetImage(i);
				Image2DPtr output = Image2D::CreateEmptyImagePtr(input->Width(), input->Height());
				for(unsigned x=0;x<input->Width();++x)
				{
					for(unsigned y=0;y<input->Height();++y)
					{
						if(inverse)
						{
							if(_horProfile[x] != 0.0)
								output->SetValue(x, y, input->Value(x, y) / _horProfile[x]);
							else
								output->SetValue(x, y, 0.0);
						} else {
								output->SetValue(x, y, input->Value(x, y) * _horProfile[x]);
						}
					}
				}
				data.SetImage(i, output);
			}
			_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
			_timeFrequencyWidget.Update();
		}
	}
}

void MSWindow::onUseVertProfile(bool inverse)
{
	if(HasImage())
	{
		TimeFrequencyData data = GetActiveData();
		if(_vertProfile.size()==data.ImageHeight())
		{
			TimeFrequencyData data = GetActiveData();
			for(unsigned i=0;i<data.ImageCount();++i)
			{
				Image2DCPtr input = data.GetImage(i);
				Image2DPtr output = Image2D::CreateEmptyImagePtr(input->Width(), input->Height());
				for(unsigned x=0;x<input->Width();++x)
				{
					for(unsigned y=0;y<input->Height();++y)
					{
						if(inverse)
						{
							if(_vertProfile[y] != 0.0)
								output->SetValue(x, y, input->Value(x, y) / _vertProfile[y]);
							else
								output->SetValue(x, y, 0.0);
						} else {
								output->SetValue(x, y, input->Value(x, y) * _vertProfile[y]);
						}
					}
				}
				data.SetImage(i, output);
			}
			_timeFrequencyWidget.SetNewData(data, _timeFrequencyWidget.GetMetaData());
			_timeFrequencyWidget.Update();
		}
	}
}

void MSWindow::onStoreData()
{
	if(HasImage())
	{
		_storedData = _timeFrequencyWidget.GetActiveData();
	}
}

void MSWindow::onRecallData()
{
	_timeFrequencyWidget.SetNewData(_storedData, _timeFrequencyWidget.GetMetaData());
	_timeFrequencyWidget.Update();
}

void MSWindow::onSubtractDataFromMem()
{
	if(HasImage())
	{
		TimeFrequencyData activeData = _timeFrequencyWidget.GetActiveData();
		TimeFrequencyData *diffData = TimeFrequencyData::CreateTFDataFromDiff(_storedData, activeData);
		_timeFrequencyWidget.SetNewData(*diffData, _timeFrequencyWidget.GetMetaData());
		delete diffData;
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onTimeMergeUnsetValues()
{
	if(HasImage())
	{
		TimeFrequencyData activeData = _timeFrequencyWidget.GetActiveData();
		TimeFrequencyMetaDataPtr metaData(new class TimeFrequencyMetaData(*_timeFrequencyWidget.GetMetaData()));
		rfiStrategy::NoiseStatImageSet::MergeInTime(activeData, metaData);
		_timeFrequencyWidget.SetNewData(activeData, metaData);
		_timeFrequencyWidget.Update();
	}
}

void MSWindow::onExportImage()
{
	if(HasImage())
	{
		Gtk::FileChooserDialog dialog("Specify image filename", Gtk::FILE_CHOOSER_ACTION_SAVE);
		dialog.set_transient_for(*this);

		//Add response buttons the the dialog:
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button("Save", Gtk::RESPONSE_OK);

		Gtk::FileFilter pdfFilter;
		std::string pdfName = "Portable Document Format (*.pdf)";
		pdfFilter.set_name(pdfName);
		pdfFilter.add_pattern("*.pdf");
		pdfFilter.add_mime_type("application/pdf");
		dialog.add_filter(pdfFilter);

		Gtk::FileFilter svgFilter;
		std::string svgName = "Scalable Vector Graphics (*.svg)";
		svgFilter.set_name(svgName);
		svgFilter.add_pattern("*.svg");
		svgFilter.add_mime_type("image/svg+xml");
		dialog.add_filter(svgFilter);

		Gtk::FileFilter pngFilter;
		std::string pngName = "Portable Network Graphics (*.png)";
		pngFilter.set_name(pngName);
		pngFilter.add_pattern("*.png");
		pngFilter.add_mime_type("image/png");
		dialog.add_filter(pngFilter);

		int result = dialog.run();

		if(result == Gtk::RESPONSE_OK)
		{
			const Gtk::FileFilter *filter = dialog.get_filter();
			if(filter->get_name() == pdfName)
				_timeFrequencyWidget.SavePdf(dialog.get_filename());
			else if(filter->get_name() == svgName)
				_timeFrequencyWidget.SaveSvg(dialog.get_filename());
			else
				_timeFrequencyWidget.SavePng(dialog.get_filename());
		}
	}
}
