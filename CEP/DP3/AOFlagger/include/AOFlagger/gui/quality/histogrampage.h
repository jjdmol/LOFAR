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
#ifndef GUI_QUALITY__HISTOGRAMPAGE_H
#define GUI_QUALITY__HISTOGRAMPAGE_H

#include <string>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/frame.h>

#include <AOFlagger/quality/qualitytablesformatter.h>

#include <AOFlagger/gui/plot/plot2d.h>
#include <AOFlagger/gui/plot/plotwidget.h>
#include <gtkmm/textview.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class HistogramPage : public Gtk::HBox {
	public:
		HistogramPage();
    ~HistogramPage();

		void SetStatistics(const std::string &filename)
		{
			_statFilename = filename;
			readFromFile();
			updatePlot();
		}
		void SetStatistics(class HistogramCollection &collection);
		void CloseStatistics()
		{
			_statFilename = std::string();
		}
		bool HasStatistics() const
		{
			return _histograms != 0;
		}
	private:
		void addHistogramToPlot(class LogHistogram &histogram);
		void addRayleighToPlot(class LogHistogram &histogram, double sigma, double n);
		void addRayleighDifferenceToPlot(LogHistogram &histogram, double sigma, double n);
		void updatePlot();
		void plotPolarization(class HistogramCollection &histograms, unsigned p);
		void plotFit(class LogHistogram &histogram, const std::string &title);
		void plotSlope(class LogHistogram &histogram, const std::string &title);
		void onPlotPropertiesClicked();
		void onDataExportClicked();
		void readFromFile();
		void updateSlopeFrame();
		void updateDataWindow();
		
		void onAutoRangeClicked()
		{
			bool autoRange = _fitAutoRangeButton.get_active();
			_fitStartEntry.set_sensitive(!autoRange);
			_fitEndEntry.set_sensitive(!autoRange);
			if(autoRange)
				updatePlot();
		}
		
		Gtk::VBox _sideBox;
		
		Gtk::Frame _histogramTypeFrame;
		Gtk::VBox _histogramTypeBox;
		Gtk::CheckButton _totalHistogramButton, _rfiHistogramButton, _notRFIHistogramButton;
		
		Gtk::Frame _polarizationFrame;
		Gtk::VBox _polarizationBox;
		Gtk::CheckButton _xxPolarizationButton, _xyPolarizationButton, _yxPolarizationButton, _yyPolarizationButton;
		
		Gtk::Frame _fitFrame;
		Gtk::VBox _fitBox;
		Gtk::CheckButton _fitButton, _subtractFitButton, _fitAutoRangeButton;
		Gtk::Entry _fitStartEntry, _fitEndEntry;
		
		Gtk::Frame _functionFrame;
		Gtk::VBox _functionBox;
		Gtk::RadioButton _nsButton, _dndsButton;
		
		Gtk::Button _plotPropertiesButton, _dataExportButton;
		
		Gtk::Frame _slopeFrame;
		Gtk::VBox _slopeBox;
		Gtk::TextView _slopeTextView;
		Gtk::CheckButton _drawSlope;
		
		std::string _statFilename;
		Plot2D _plot;
		PlotWidget _plotWidget;
		class PlotPropertiesWindow *_plotPropertiesWindow;
		class DataWindow *_dataWindow;
		class HistogramCollection *_histograms;
};

#endif
