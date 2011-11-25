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
#ifndef GUI_QUALITY__GRAYSCALEPLOTPAGE_H
#define GUI_QUALITY__GRAYSCALEPLOTPAGE_H

#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>

#include <AOFlagger/gui/imagewidget.h>

#include <AOFlagger/quality/qualitytablesformatter.h>

#include <AOFlagger/msio/timefrequencydata.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class GrayScalePlotPage : public Gtk::HBox {
	public:
		GrayScalePlotPage();
    virtual ~GrayScalePlotPage();
		
	protected:
		virtual TimeFrequencyData ConstructImage() = 0;
		
		QualityTablesFormatter::StatisticKind GetSelectedStatisticKind() const
		{
			return _selectStatisticKind;
		}
		
		void UpdateImage();
		
		ImageWidget &GrayScaleWidget() { return _imageWidget; }
	private:
		void initStatisticKinds();
		void initPolarizations();
		void initPhaseButtons();
		void initRanges();
		
		void onSelectCount() { _selectStatisticKind = QualityTablesFormatter::CountStatistic; UpdateImage(); }
		void onSelectMean() { _selectStatisticKind = QualityTablesFormatter::MeanStatistic; UpdateImage(); }
		void onSelectVariance() { _selectStatisticKind = QualityTablesFormatter::VarianceStatistic; UpdateImage(); }
		void onSelectDCount() { _selectStatisticKind = QualityTablesFormatter::DCountStatistic; UpdateImage(); }
		void onSelectDMean() { _selectStatisticKind = QualityTablesFormatter::DMeanStatistic; UpdateImage(); }
		void onSelectDVariance() { _selectStatisticKind = QualityTablesFormatter::DVarianceStatistic; UpdateImage(); }
		void onSelectRFIRatio() { _selectStatisticKind = QualityTablesFormatter::RFIRatioStatistic; UpdateImage(); }
		void onSelectSNR() { _selectStatisticKind = QualityTablesFormatter::SignalToNoiseStatistic; UpdateImage(); }
		
		void onSelectMinMaxRange() { _imageWidget.SetRange(ImageWidget::MinMax); _imageWidget.Update(); }
		void onSelectWinsorizedRange() { _imageWidget.SetRange(ImageWidget::Winsorized); _imageWidget.Update(); }
		void onSelectSpecifiedRange() { _imageWidget.SetRange(ImageWidget::Specified); _imageWidget.Update(); }
		void onLogarithmicScaleClicked()
		{
			if(_logarithmicScaleButton.get_active())
				_imageWidget.SetScaleOption(ImageWidget::LogScale);
			else
				_imageWidget.SetScaleOption(ImageWidget::NormalScale);
			 _imageWidget.Update();
		}
		
		void setToSelectedPolarization(TimeFrequencyData &data);
		void setToSelectedPhase(TimeFrequencyData &data);
		
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
		
		bool _ready;
};

#endif
