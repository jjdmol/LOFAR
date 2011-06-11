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
#ifndef MSWINDOW_H
#define MSWINDOW_H

#include <set>

#include <boost/thread/mutex.hpp>

#include <gtkmm/actiongroup.h>
#include <gtkmm/box.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/paned.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/toggleaction.h>
#include <gtkmm/window.h>

#include <AOFlagger/msio/timefrequencydata.h>
#include <AOFlagger/msio/timefrequencymetadata.h>

#include <AOFlagger/strategy/control/types.h>

#include <AOFlagger/gui/plot/plotwidget.h>

#include <AOFlagger/gui/plotframe.h>
#include <AOFlagger/gui/timefrequencywidget.h>

#include <AOFlagger/imaging/defaultmodels.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class MSWindow : public Gtk::Window {
	public:
		MSWindow();
		~MSWindow();

		void SetImageSet(rfiStrategy::ImageSet *newImageSet);
		void SetImageSetIndex(rfiStrategy::ImageSetIndex *newImageSetIndex);
		rfiStrategy::ImageSet &GetImageSet() const { return *_imageSet; }
		rfiStrategy::ImageSetIndex &GetImageSetIndex() const { return *_imageSetIndex; }
		void AddAlternativeFlagging(Mask2DCPtr mask)
		{
			_timeFrequencyWidget.AddAlternativeFlagging(mask);
		}
		void SetRevisedData(const TimeFrequencyData &data)
		{
			_timeFrequencyWidget.SetRevisedData(data);
		}
		void Update()
		{
			_timeFrequencyWidget.Update();
		}
 		bool HasImageSet() const { return _imageSet != 0; }
		bool HasImage() const { return _timeFrequencyWidget.HasImage(); }
		Mask2DCPtr Mask() const { return GetOriginalData().GetSingleMask(); }
		Mask2DCPtr AltMask() const { return GetContaminatedData().GetSingleMask(); }
		
		TimeFrequencyData GetActiveData() const
		{
			return _timeFrequencyWidget.GetActiveData();
		}
		const TimeFrequencyData &GetOriginalData() const
		{
			return _timeFrequencyWidget.OriginalData();
		}
		const TimeFrequencyData &GetContaminatedData() const
		{
			return _timeFrequencyWidget.ContaminatedData();
		}

		class TimeFrequencyWidget &GetTimeFrequencyWidget()
		{
			return _timeFrequencyWidget;
		}
		
		class ThresholdConfig &HighlightConfig()
		{
			return _timeFrequencyWidget.HighlightConfig();
		}
		void SetHighlighting(bool newValue)
		{
			_timeFrequencyWidget.SetHighlighting(newValue);
		}
		TimeFrequencyMetaDataCPtr TimeFrequencyMetaData()
		{
			return _timeFrequencyWidget.GetMetaData();
		}
		rfiStrategy::Strategy &Strategy() const { return *_strategy; }
		void SetStrategy(rfiStrategy::Strategy *newStrategy) { _strategy = newStrategy; }

		void onExecuteStrategyFinished();
	private:
		void createToolbar();
		void loadCurrentTFData();
		void openPath(const std::string &path);
		
		void onLoadPrevious();
		void onLoadNext();
		void onLoadLargeStepPrevious();
		void onLoadLargeStepNext();
		void onToggleFlags();
		void onToggleMap();
		void onToggleImage();
		void onRangeChanged();
		void onCompress();
		void onQuit() { hide(); }
		void onActionFileOpen();
		void onActionDirectoryOpen();
		void onActionDirectoryOpenForSpatial();
		void onOpenBandCombined();
		void onShowImagePlane();
		void onSetAndShowImagePlane();
		void onAddToImagePlane();
		void onClearAltFlagsPressed();
		void onDifferenceToOriginalPressed();
		void onBackgroundToOriginalPressed();
		void onHightlightPressed();
		void showPhasePart(enum TimeFrequencyData::PhaseRepresentation phaseRepresentation);
		void onShowRealPressed() { showPhasePart(TimeFrequencyData::RealPart); }
		void onShowImaginaryPressed() { showPhasePart(TimeFrequencyData::ImaginaryPart); }
		void onShowPhasePressed() { showPhasePart(TimeFrequencyData::PhasePart); }
		void showPolarisation(enum PolarisationType polarisation);
		void onShowStokesIPressed() { showPolarisation(StokesIPolarisation); }
		void onShowStokesQPressed() { showPolarisation(StokesQPolarisation); }
		void onShowStokesUPressed() { showPolarisation(StokesUPolarisation); }
		void onShowStokesVPressed() { showPolarisation(StokesVPolarisation); }
		void onShowAutoDipolePressed() { showPolarisation(AutoDipolePolarisation); }
		void onShowCrossDipolePressed() { showPolarisation(CrossDipolePolarisation); }
		void onShowXXPressed() { showPolarisation(XXPolarisation); }
		void onShowXYPressed() { showPolarisation(XYPolarisation); }
		void onShowYXPressed() { showPolarisation(YXPolarisation); }
		void onShowYYPressed() { showPolarisation(YYPolarisation); }
		void onZoomPressed();
		void onOpenTestSetNoise() { openTestSet(2); }
		void onOpenTestSetA() { openTestSet(3); }
		void onOpenTestSetB() { openTestSet(4); }
		void onOpenTestSetC() { openTestSet(5); }
		void onOpenTestSetD() { openTestSet(18); }
		void onOpenTestSetE() { openTestSet(14); }
		void onOpenTestSetF() { openTestSet(16); }
		void onOpenTestSetG() { openTestSet(17); }
		void onOpenTestSetH() { openTestSet(7); }
		void onOpenTestSetNoise3Model() { openTestSet(19); }
		void onOpenTestSetNoise5Model() { openTestSet(20); }
		void onOpenTestSet3Model() { openTestSet(21); }
		void onOpenTestSet5Model() { openTestSet(22); }
		void onOpenTestSetBStrong() { openTestSet(24); }
		void onOpenTestSetBWeak() { openTestSet(23); }
		void onOpenTestSetBAligned() { openTestSet(25); }
		void onGaussianTestSets() { _gaussianTestSets = 1; }
		void onRayleighTestSets() { _gaussianTestSets = 0; }
		void onZeroTestSets() { _gaussianTestSets = 2; }
		void onAddStaticFringe();
		void onAdd1SigmaFringe();
		void onSetToOne();
		void onSetToI();
		void onSetToOnePlusI();
		void onShowStats();
		void onPlotDistPressed();
		void onPlotComplexPlanePressed();
		void onPlotPowerSpectrumPressed();
		void onPlotPowerRMSPressed();
		void onPlotPowerSNRPressed();
		void onPlotPowerTimePressed();
		void onPlotScatterPressed();
		void onPlotSingularValuesPressed();
		void onPlotSNRToFitVariance();
		void onPlotQuality25Pressed();
		void onPlotQualityAllPressed();
		void onEditStrategyPressed();
		void onExecuteStrategyPressed();
		void onGoToPressed();
		void onTFWidgetMouseMoved(size_t x, size_t y);
		void onTFWidgetButtonReleased(size_t x, size_t y);
		void onMultiplyData();
		void onSegment();
		void onCluster();
		void onClassify();
		void onRemoveSmallSegments();
		void onTimeGraphButtonPressed();
		void onFrequencyGraphButtonPressed();
		void onUnrollPhaseButtonPressed();
		void onVertEVD();
		void onApplyTimeProfile();
		void onApplyVertProfile();
		void onRestoreTimeProfile();
		void onRestoreVertProfile();
		
		void showError(const std::string &description);
		
		DefaultModels::SetLocation getSetLocation(bool empty = false);
		void loadDefaultModel(DefaultModels::Distortion distortion, bool withNoise, bool empty = false, unsigned channelCount = 64);
		void onSimulateCorrelation() { loadDefaultModel(DefaultModels::ConstantDistortion, false); }
		void onSimulateSourceSetA() { loadDefaultModel(DefaultModels::ConstantDistortion, true); }
		void onSimulateSourceSetB() { loadDefaultModel(DefaultModels::VariableDistortion, true); }
		void onSimulateSourceSetC() { loadDefaultModel(DefaultModels::FaintDistortion, true); }
		void onSimulateSourceSetD() { loadDefaultModel(DefaultModels::MislocatedDistortion, true); }
		void onSimulateSourceSetALarge() { loadDefaultModel(DefaultModels::ConstantDistortion, true, false, 256); }
		void onSimulateOffAxisSource() { loadDefaultModel(DefaultModels::ConstantDistortion, false, true); }
		void onSimulateOnAxisSource() { loadDefaultModel(DefaultModels::OnAxisSource, false, true); }
		
		void onShowAntennaMapWindow();
		void openTestSet(unsigned index);
		
		Gtk::VBox _mainVBox;
		Gtk::VPaned _panedArea;
		TimeFrequencyWidget _timeFrequencyWidget;
		Glib::RefPtr<Gtk::ActionGroup> _actionGroup;
		Gtk::Statusbar _statusbar;
		PlotFrame _plotFrame;

		Glib::RefPtr<Gtk::ToggleAction>
			_originalFlagsButton, _altFlagsButton,
			_originalImageButton, _backgroundImageButton, _diffImageButton,
			_timeGraphButton;
		Glib::RefPtr<Gtk::RadioAction>
			_mapLogButton, _mapBWButton, _mapColorButton,
			_rangeFullButton, _rangeWinsorizedButton, _rangeSpecifiedButton,
			_gaussianTestSetsButton, _rayleighTestSetsButton, _zeroTestSetsButton,
			_ncpSetButton, _b1834SetButton, _emptySetButton;
		//std::vector<Gtk::Window*> _subWindows;
		class ImagePlaneWindow *_imagePlaneWindow;
		Gtk::Window
			*_optionWindow, *_editStrategyWindow,
			*_gotoWindow,
			*_progressWindow, *_highlightWindow,
			*_plotComplexPlaneWindow, *_zoomWindow,
			*_antennaMapWindow;

		class RFIStatistics *_statistics;
		
		rfiStrategy::ImageSet *_imageSet;
		rfiStrategy::ImageSetIndex *_imageSetIndex;
		rfiStrategy::Strategy *_strategy;
		int _gaussianTestSets;
		boost::mutex _ioMutex;
		SegmentedImagePtr _segmentedImage;
		class SpatialMatrixMetaData *_spatialMetaData;
		std::vector<double> _horProfile, _vertProfile;
};

#endif
