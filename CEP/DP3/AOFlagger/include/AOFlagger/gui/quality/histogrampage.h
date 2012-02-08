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
#include <gtkmm/frame.h>

#include <AOFlagger/quality/qualitytablesformatter.h>
#include <AOFlagger/quality/histogramtablesformatter.h>

#include <AOFlagger/gui/plot/plot2d.h>
#include <AOFlagger/gui/plot/plotwidget.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class HistogramPage : public Gtk::HBox {
	public:
		HistogramPage();
    virtual ~HistogramPage();

		void SetStatistics(const std::string &filename)
		{
			_statFilename = filename;
			updatePlot();
		}
		void CloseStatistics()
		{
			_statFilename = std::string();
		}
		bool HasStatistics() const
		{
			return !_statFilename.empty();
		}
	private:
		void addHistogramToPlot(const std::vector<HistogramTablesFormatter::HistogramItem> &histogram);
		void updatePlot();
		void onPlotPropertiesClicked() { }
		void onDataExportClicked() { }
		
		std::string _statFilename;
		Plot2D _plot;
		PlotWidget _plotWidget;
};

#endif
