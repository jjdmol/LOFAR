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

#include <AOFlagger/remote/processcommander.h>

#include <unistd.h> //gethostname

#include <AOFlagger/remote/serverconnection.h>

#include <AOFlagger/quality/statisticscollection.h>
#include <AOFlagger/quality/histogramcollection.h>
#include <AOFlagger/quality/statisticsderivator.h>

namespace aoRemote {

ProcessCommander::ProcessCommander(const ClusteredObservation &observation)
: _server(), _observation(observation)
{
	_server.SignalConnectionCreated().connect(sigc::mem_fun(*this, &ProcessCommander::onConnectionCreated));
}

ProcessCommander::~ProcessCommander()
{
	for(std::vector<RemoteProcess*>::iterator i=_processes.begin();i!=_processes.end();++i)
	{
		delete *i;
	}
}

void ProcessCommander::Run(bool finishConnections)
{
	_errors.clear();
	_finishConnections = finishConnections;
	
	if(!_observation.GetItems().empty() && !_tasks.empty())
	{
		const std::string thisHostName = GetHostName();
		
		// make a list of the involved nodes
		_nodeCommands.Initialize(_observation);
		
		if(_processes.empty())
		{
			//construct a process for each unique node name
			std::vector<std::string> list;
			_nodeCommands.NodeList(list);
			for(std::vector<std::string>::const_iterator i=list.begin();i!=list.end();++i)
			{
				RemoteProcess *process = new RemoteProcess(*i, thisHostName);
				process->SignalFinished().connect(sigc::mem_fun(*this, &ProcessCommander::onProcessFinished));
				process->Start();
				_processes.push_back(process);
			}
		}
		
		// We will now start accepting connections. The Run() method will not return until the server
		// stops listening and there are no more io operations pending. With asynchroneous
		// handles, the server and its connections will call onEvent...(). These handles
		// will push new tasks until all tasks in the ProcessCommander are finished.
		_server.Run();
	}
}

void ProcessCommander::continueReadQualityTablesTask(ServerConnectionPtr serverConnection)
{
	const std::string &hostname = serverConnection->Hostname();
	
	boost::mutex::scoped_lock lock(_mutex);
	ClusteredObservationItem item;
	if(_nodeCommands.Pop(hostname, item))
	{
		const std::string msFilename = item.LocalPath();
		StatisticsCollection *statisticsCollection = new StatisticsCollection();
		HistogramCollection *histogramCollection = new HistogramCollection();
		serverConnection->ReadQualityTables(msFilename, *statisticsCollection, *histogramCollection);
	} else {
		serverConnection->StopClient();
		
		if(_nodeCommands.Empty())
		{
			onCurrentTaskFinished();
			lock.unlock();
			onConnectionAwaitingCommand(serverConnection);
		}
	}
}

void ProcessCommander::continueReadAntennaTablesTask(ServerConnectionPtr serverConnection)
{
	boost::mutex::scoped_lock lock(_mutex);
	onCurrentTaskFinished();
	
	const std::string &hostname = serverConnection->Hostname();
	std::vector<AntennaInfo> *antennas = new std::vector<AntennaInfo>();
	serverConnection->ReadAntennaTables(_nodeCommands.Top(hostname).LocalPath(), *antennas);
}

void ProcessCommander::continueReadDataRowsTask(ServerConnectionPtr serverConnection)
{
}

std::string ProcessCommander::GetHostName()
{
	char name[HOST_NAME_MAX];
	if(gethostname(name, HOST_NAME_MAX) == 0)
	{
		return std::string(name);
	} else {
		throw std::runtime_error("Error retrieving hostname");
	}
}

void ProcessCommander::onConnectionCreated(ServerConnectionPtr serverConnection, bool &acceptConnection)
{
	serverConnection->SignalAwaitingCommand().connect(sigc::mem_fun(*this, &ProcessCommander::onConnectionAwaitingCommand));
	serverConnection->SignalFinishReadQualityTables().connect(sigc::mem_fun(*this, &ProcessCommander::onConnectionFinishReadQualityTables));
	serverConnection->SignalFinishReadAntennaTables().connect(sigc::mem_fun(*this, &ProcessCommander::onConnectionFinishReadAntennaTables));
	serverConnection->SignalFinishReadDataRows().connect(sigc::mem_fun(*this, &ProcessCommander::onConnectionFinishReadDataRows));
	serverConnection->SignalError().connect(sigc::mem_fun(*this, &ProcessCommander::onError));
	acceptConnection = true;
}

void ProcessCommander::onConnectionAwaitingCommand(ServerConnectionPtr serverConnection)
{
	switch(currentTask())
	{
		case ReadQualityTablesTask:
			continueReadQualityTablesTask(serverConnection);
			break;
		case ReadAntennaTablesTask:
			continueReadAntennaTablesTask(serverConnection);
			break;
		case ReadDataRowsTask:
			continueReadDataRowsTask(serverConnection);
			break;
		case NoTask:
			if(_finishConnections)
				serverConnection->StopClient();
			else
				_idleConnections.push_back(serverConnection);
			break;
	}
}

void ProcessCommander::onConnectionFinishReadQualityTables(ServerConnectionPtr serverConnection, StatisticsCollection &statisticsCollection, HistogramCollection &histogramCollection)
{
	boost::mutex::scoped_lock lock(_mutex);
	if(statisticsCollection.PolarizationCount() == 0)
		throw std::runtime_error("Client sent StatisticsCollection with 0 polarizations.");
	
	// If the collection is still empty, we need to set its polarization count
	if(_statisticsCollection->PolarizationCount() == 0)
		_statisticsCollection->SetPolarizationCount(statisticsCollection.PolarizationCount());
	
	_statisticsCollection->Add(statisticsCollection);
	
	if(!histogramCollection.Empty())
	{
		if(_correctHistograms)
		{
			DefaultStatistics thisStat(statisticsCollection.PolarizationCount());
			statisticsCollection.GetGlobalCrossBaselineStatistics(thisStat);
			DefaultStatistics singlePol = thisStat.ToSinglePolarization();
			double stddev = StatisticsDerivator::GetStatisticAmplitude(QualityTablesFormatter::DStandardDeviationStatistic, singlePol, 0);
			
			std::cout << "Scaling with " << 1.0 / stddev << ".\n";
			histogramCollection.Rescale(1.0 / stddev);
		}
		
		if(_histogramCollection->PolarizationCount() == 0)
			_histogramCollection->SetPolarizationCount(histogramCollection.PolarizationCount());
		
		_histogramCollection->Add(histogramCollection);
	}
	
	delete &statisticsCollection;
	delete &histogramCollection;
}

void ProcessCommander::onConnectionFinishReadAntennaTables(ServerConnectionPtr serverConnection, std::vector<AntennaInfo> &antennas)
{
	boost::mutex::scoped_lock lock(_mutex);
	_antennas = antennas;
	delete &antennas;
}

void ProcessCommander::onConnectionFinishReadDataRows(ServerConnectionPtr serverConnection, MSRowDataExt *rowData)
{
}

void ProcessCommander::onError(ServerConnectionPtr connection, const std::string &error)
{
	std::stringstream s;
	
	const std::string &hostname = connection->Hostname();
	ClusteredObservationItem item;
	bool knowFile = _nodeCommands.Current(hostname, item);
	s << "On connection with " << hostname;
	if(knowFile)
		s << " to process local file '" << item.LocalPath() << "'";
	s << ", reported error was: " << error;
	boost::mutex::scoped_lock lock(_mutex);
	_errors.push_back(s.str());
}

void ProcessCommander::onProcessFinished(RemoteProcess &process, bool error, int status)
{
	boost::mutex::scoped_lock lock(_mutex);
	
	if(!_nodeCommands.RemoveNode(process.ClientHostname()))
		onCurrentTaskFinished();
	
	if(error)
	{
		std::stringstream s;
		s << "Remote process to " << process.ClientHostname() << " reported an error";
		if(status != 0) s << " (status " << status << ")";
		_errors.push_back(s.str());
	}
}


}
