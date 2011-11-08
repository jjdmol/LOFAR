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
#ifndef AOQPLOT_WINDOW_H
#define AOQPLOT_WINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/radiobutton.h>

#include <AOFlagger/gui/imagewidget.h>

#include <AOFlagger/quality/qualitytablesformatter.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class AOQPlotWindow : public Gtk::Window {
	public:
		AOQPlotWindow();
		
		void Open(const std::string &filename)
		{
			_filename = filename;
			_isOpen = true;
			updateImage();
		}
		
		QualityTablesFormatter::StatisticKind GetSelectedStatisticKind() const
		{
			return _selectStatisticKind;
		}
	private:
		void updateImage();
		void onSelectCount() { _selectStatisticKind = QualityTablesFormatter::CountStatistic; updateImage(); }
		void onSelectMean() { _selectStatisticKind = QualityTablesFormatter::MeanStatistic; updateImage(); }
		void onSelectVariance() { _selectStatisticKind = QualityTablesFormatter::VarianceStatistic; updateImage(); }
		void onSelectDCount() { _selectStatisticKind = QualityTablesFormatter::DCountStatistic; updateImage(); }
		void onSelectDMean() { _selectStatisticKind = QualityTablesFormatter::DMeanStatistic; updateImage(); }
		void onSelectDVariance() { _selectStatisticKind = QualityTablesFormatter::DVarianceStatistic; updateImage(); }
		void onSelectRFIRatio() { _selectStatisticKind = QualityTablesFormatter::RFIRatioStatistic; updateImage(); }
		
		void setToSelectedPolarization(TimeFrequencyData &data);
		
		Gtk::HBox _mainHBox;
		Gtk::VBox _sideBox;
		
		Gtk::Frame _statisticKindFrame;
		Gtk::VBox _statisticKindBox;
		
		Gtk::RadioButton _countButton, _meanButton, _varianceButton, _dCountButton, _dMeanButton, _dVarianceButton, _rfiRatioButton;
		
		Gtk::Frame _polarizationFrame;
		Gtk::VBox _polarizationBox;
		
		Gtk::RadioButton _polXXButton, _polXYButton, _polYXButton, _polYYButton, _polXXandYYButton, _polXYandYXButton;
		
		bool _isOpen;
		std::string _filename;
		QualityTablesFormatter::StatisticKind _selectStatisticKind;
		ImageWidget _imageWidget;
};

#endif
