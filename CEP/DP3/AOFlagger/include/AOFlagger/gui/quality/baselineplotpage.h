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
#ifndef GUI_QUALITY__BASELINEPLOTPAGE_H
#define GUI_QUALITY__BASELINEPLOTPAGE_H

#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>

#include <AOFlagger/gui/imagewidget.h>

#include <AOFlagger/quality/qualitytablesformatter.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class BaselinePlotPage : public Gtk::HBox {
	public:
		BaselinePlotPage();
    virtual ~BaselinePlotPage();
		
		QualityTablesFormatter::StatisticKind GetSelectedStatisticKind() const
		{
			return _selectStatisticKind;
		}
		
		void SetStatistics(class StatisticsCollection *statCollection, const std::vector<class AntennaInfo> &antennas)
		{
			_statCollection = statCollection;
			_antennas = &antennas;
			updateImage();
		}
		void CloseStatistics()
		{
			_statCollection = 0;
			_antennas = 0;
		}
		bool HasStatistics() const
		{
			return _statCollection != 0;
		}
		
		sigc::signal<void, const std::string &> SignalStatusChange() { return _signalStatusChange; }
	private:
		void initStatisticKinds();
		void initPolarizations();
		void initPhaseButtons();
		void initRanges();
		
		void updateImage();
		void onSelectCount() { _selectStatisticKind = QualityTablesFormatter::CountStatistic; updateImage(); }
		void onSelectMean() { _selectStatisticKind = QualityTablesFormatter::MeanStatistic; updateImage(); }
		void onSelectVariance() { _selectStatisticKind = QualityTablesFormatter::VarianceStatistic; updateImage(); }
		void onSelectDCount() { _selectStatisticKind = QualityTablesFormatter::DCountStatistic; updateImage(); }
		void onSelectDMean() { _selectStatisticKind = QualityTablesFormatter::DMeanStatistic; updateImage(); }
		void onSelectDVariance() { _selectStatisticKind = QualityTablesFormatter::DVarianceStatistic; updateImage(); }
		void onSelectRFIRatio() { _selectStatisticKind = QualityTablesFormatter::RFIRatioStatistic; updateImage(); }
		void onSelectSNR() { _selectStatisticKind = QualityTablesFormatter::SignalToNoiseStatistic; updateImage(); }
		
		void onSelectMinMaxRange() { _imageWidget.SetRange(ImageWidget::MinMax); _imageWidget.Update(); }
		void onSelectWinsorizedRange() { _imageWidget.SetRange(ImageWidget::Winsorized); _imageWidget.Update(); }
		void onSelectSpecifiedRange() { _imageWidget.SetRange(ImageWidget::Specified); _imageWidget.Update(); }
		void onLogarithmicScaleClicked() {
			if(_logarithmicScaleButton.get_active())
				_imageWidget.SetScaleOption(ImageWidget::LogScale);
			else
				_imageWidget.SetScaleOption(ImageWidget::NormalScale);
			 _imageWidget.Update();
		}
		
		void setToSelectedPolarization(TimeFrequencyData &data);
		void setToSelectedPhase(TimeFrequencyData &data);
		
		void onMouseMoved(size_t x, size_t y);
		
		Gtk::VBox _sideBox;
		
		Gtk::Frame _statisticKindFrame;
		Gtk::VBox _statisticKindBox;
		
		Gtk::RadioButton _countButton, _meanButton, _varianceButton, _dCountButton, _dMeanButton, _dVarianceButton, _rfiRatioButton, _snrButton;
		
		Gtk::Frame _polarizationFrame;
		Gtk::VBox _polarizationBox;
		
		Gtk::RadioButton _polXXButton, _polXYButton, _polYXButton, _polYYButton, _polXXandYYButton, _polXYandYXButton;
		
		Gtk::Frame _phaseFrame;
		Gtk::VBox _phaseBox;
		
		Gtk::RadioButton _amplitudePhaseButton, _phasePhaseButton, _realPhaseButton, _imaginaryPhaseButton;
		
		Gtk::Frame _rangeFrame;
		Gtk::VBox _rangeBox;
		
		Gtk::RadioButton _rangeMinMaxButton, _rangeWinsorizedButton, _rangeSpecified;
		Gtk::CheckButton _logarithmicScaleButton;
		
		QualityTablesFormatter::StatisticKind _selectStatisticKind;
		ImageWidget _imageWidget;
		class StatisticsCollection *_statCollection;
		const std::vector<class AntennaInfo> *_antennas;
		
		sigc::signal<void, const std::string &> _signalStatusChange;
};

#endif
