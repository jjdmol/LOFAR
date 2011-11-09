/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#include <limits>

#include <AOFlagger/gui/quality/aoqplotwindow.h>

#include <AOFlagger/msio/measurementset.h>

#include <AOFlagger/quality/statisticscollection.h>

AOQPlotWindow::AOQPlotWindow() :
	_isOpen(false)
{
	_notebook.append_page(_baselinePlotPage, "Baselines");
	_baselinePlotPage.show();
	
	_notebook.append_page(_bLengthPlotPage, "Baselines length");
	_bLengthPlotPage.show();
	
	_notebook.append_page(_timePlotPage, "Time");
	_timePlotPage.show();
	
	_notebook.append_page(_frequencyPlotPage, "Frequency");
	_frequencyPlotPage.show();
	
	add(_notebook);
	_notebook.show();
}

void AOQPlotWindow::Open(const std::string &filename)
{
	_filename = filename;
	readStatistics();
	_baselinePlotPage.SetStatistics(_statCollection);
	_bLengthPlotPage.SetStatistics(_statCollection, filename);
	_timePlotPage.SetStatistics(_statCollection, filename);
	_frequencyPlotPage.SetStatistics(_statCollection, filename);
}


void AOQPlotWindow::close()
{
	if(_isOpen)
	{
		_baselinePlotPage.CloseStatistics();
		_bLengthPlotPage.CloseStatistics();
		_timePlotPage.CloseStatistics();
		_frequencyPlotPage.CloseStatistics();
		delete _statCollection;
		_isOpen = false;
	}
}

void AOQPlotWindow::readStatistics()
{
	MeasurementSet *ms = new MeasurementSet(_filename);
	const unsigned polarizationCount = ms->GetPolarizationCount();
	delete ms;

	QualityTablesFormatter formatter(_filename);
	_statCollection = new StatisticsCollection(polarizationCount);
	_statCollection->Load(formatter);
	_isOpen = true;
}

