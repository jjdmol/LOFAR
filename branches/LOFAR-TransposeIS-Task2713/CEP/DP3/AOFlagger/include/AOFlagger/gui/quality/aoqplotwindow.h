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
#include <gtkmm/notebook.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/window.h>

#include <AOFlagger/gui/imagewidget.h>

#include <AOFlagger/quality/qualitytablesformatter.h>

#include "antennaeplotpage.h"
#include "baselineplotpage.h"
#include "blengthplotpage.h"
#include "frequencyplotpage.h"
#include "summarypage.h"
#include "timeplotpage.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class AOQPlotWindow : public Gtk::Window {
	public:
		AOQPlotWindow();
    virtual ~AOQPlotWindow()
    { 
			close();
		}
    
		void Open(const std::string &filename);
		
	private:
		void close();
		void readStatistics();
		void onStatusChange(const std::string &newStatus);
		
		Gtk::VBox _vBox;
		Gtk::Notebook _notebook;
		Gtk::Statusbar _statusBar;
		
		BaselinePlotPage _baselinePlotPage;
		AntennaePlotPage _antennaePlotPage;
		BLengthPlotPage  _bLengthPlotPage;
		TimePlotPage _timePlotPage;
		FrequencyPlotPage _frequencyPlotPage;
		SummaryPage _summaryPage;

		bool _isOpen;
		std::string _filename;
		class StatisticsCollection *_statCollection;
		class AntennaInfo *_antennas;
};

#endif
