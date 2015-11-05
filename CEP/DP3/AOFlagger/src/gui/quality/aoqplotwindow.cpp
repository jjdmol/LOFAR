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

#include <gtkmm/messagedialog.h>

#include <AOFlagger/gui/quality/aoqplotwindow.h>

#include <AOFlagger/msio/measurementset.h>

#include <AOFlagger/quality/statisticscollection.h>

#include <AOFlagger/remote/clusteredobservation.h>
#include <AOFlagger/remote/processcommander.h>

AOQPlotWindow::AOQPlotWindow() :
	_isOpen(false)
{
	_notebook.append_page(_baselinePlotPage, "Baselines");
	_baselinePlotPage.show();
	_baselinePlotPage.SignalStatusChange().connect(sigc::mem_fun(*this, &AOQPlotWindow::onStatusChange));
	
	_notebook.append_page(_antennaePlotPage, "Antennae");
	_antennaePlotPage.show();
	
	_notebook.append_page(_bLengthPlotPage, "Baselines length");
	_bLengthPlotPage.show();
	
	_notebook.append_page(_timePlotPage, "Time");
	_timePlotPage.show();
	
	_notebook.append_page(_frequencyPlotPage, "Frequency");
	_frequencyPlotPage.show();
	
	_notebook.append_page(_timeFrequencyPlotPage, "Time-frequency");
	_timeFrequencyPlotPage.show();
	_timeFrequencyPlotPage.SignalStatusChange().connect(sigc::mem_fun(*this, &AOQPlotWindow::onStatusChange));
	//_timeFrequencyPlotPage.set_sensitive(false);
	
	_notebook.append_page(_summaryPage, "Summary");
	_summaryPage.show();
	
	_vBox.pack_start(_notebook);
	_notebook.show();
	
	_vBox.pack_end(_statusBar, Gtk::PACK_SHRINK);
	_statusBar.push("Quality plot util is ready. Author: André Offringa (offringa@astro.rug.nl)");
	_statusBar.show();
	
	add(_vBox);
	_vBox.show();
}

void AOQPlotWindow::Open(const std::string &filename)
{
	_filename = filename;
	readStatistics();
	_baselinePlotPage.SetStatistics(_statCollection, _antennas);
	_antennaePlotPage.SetStatistics(_statCollection, _antennas);
	_bLengthPlotPage.SetStatistics(_statCollection, _antennas);
	_timePlotPage.SetStatistics(_statCollection, _antennas);
	_frequencyPlotPage.SetStatistics(_statCollection, _antennas);
	//if(_fullStats->AllTimeStatistics().size() > 1)
	//{
		_timeFrequencyPlotPage.SetStatistics(_fullStats);
	//	_timeFrequencyPlotPage.set_sensitive(true);
	//} else {
	//	_timeFrequencyPlotPage.set_sensitive(false);
	//}
	_summaryPage.SetStatistics(_statCollection);
}


void AOQPlotWindow::close()
{
	if(_isOpen)
	{
		_baselinePlotPage.CloseStatistics();
		_antennaePlotPage.CloseStatistics();
		_bLengthPlotPage.CloseStatistics();
		_timePlotPage.CloseStatistics();
		_frequencyPlotPage.CloseStatistics();
		_timeFrequencyPlotPage.CloseStatistics();
		_summaryPage.CloseStatistics();
		delete _statCollection;
		delete _fullStats;
		_isOpen = false;
		
	}
}

void AOQPlotWindow::readStatistics()
{
	close();
	
	if(aoRemote::ClusteredObservation::IsClusteredFilename(_filename))
	{
		aoRemote::ClusteredObservation *observation = aoRemote::ClusteredObservation::Load(_filename);
		_statCollection = new StatisticsCollection();
		aoRemote::ProcessCommander commander(*observation);
		commander.PushReadAntennaTablesTask();
		commander.PushReadQualityTablesTask(_statCollection);
		commander.Run();
		if(!commander.Errors().empty())
		{
			std::stringstream s;
			s << commander.Errors().size() << " error(s) occured while querying the nodes or measurement sets in the given observation. This might be caused by a failing node, an unreadable measurement set, or maybe the quality tables are not available. The errors reported are:\n\n";
			for(std::vector<std::string>::const_iterator i=commander.Errors().begin();i!=commander.Errors().end();++i)
			{
				s << "- " << *i << '\n';
			}
			s << "\nThe program will continue, but this might mean that the statistics are incomplete. If this is the case, fix the issues and reopen the observation.";
			Gtk::MessageDialog dialog(s.str(), false, Gtk::MESSAGE_ERROR);
			dialog.run();
		}
		delete observation;
		
		_antennas = commander.Antennas();
	}
	else {
		MeasurementSet *ms = new MeasurementSet(_filename);
		const unsigned polarizationCount = ms->GetPolarizationCount();
		unsigned antennaCount = ms->AntennaCount();
		_antennas.clear();
		for(unsigned a=0;a<antennaCount;++a)
			_antennas.push_back(ms->GetAntennaInfo(a));
		delete ms;

		QualityTablesFormatter formatter(_filename);
		_statCollection = new StatisticsCollection(polarizationCount);
		_statCollection->Load(formatter);
	}
	std::cout << "Lowering time resolution..." << std::endl;
	_statCollection->LowerTimeResolution(1000);

	std::cout << "Lowering frequency resolution..." << std::endl;
	_statCollection->LowerFrequencyResolution(1000);

	std::cout << "Integrating baseline statistics to one channel..." << std::endl;
	_statCollection->IntegrateBaselinesToOneChannel();
	
	std::cout << "Regridding time statistics..." << std::endl;
	_statCollection->RegridTime();
	
	std::cout << "Copying statistics..." << std::endl;
	_fullStats = new StatisticsCollection(*_statCollection);
	
	std::cout << "Integrating time statistics to one channel..." << std::endl;
	_statCollection->IntegrateTimeToOneChannel();
	
	std::cout << "Opening statistics panel..." << std::endl;
	_isOpen = true;
}

void AOQPlotWindow::onStatusChange(const std::string &newStatus)
{
	_statusBar.pop();
	_statusBar.push(newStatus);
}

