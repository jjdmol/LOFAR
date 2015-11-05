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

void ProcessCommander::Run()
{
	_errors.clear();
	
	if(!_observation.GetItems().empty() && !_tasks.empty())
	{
		const std::string thisHostName = GetHostName();
		
		//construct a process for each unique node name
		makeNodeMap(_observation);
		for(std::map<std::string, std::deque<ClusteredObservationItem> >::const_iterator i=_nodeMap.begin();i!=_nodeMap.end();++i)
		{
			RemoteProcess *process = new RemoteProcess(i->first, thisHostName);
			process->SignalFinished().connect(sigc::mem_fun(*this, &ProcessCommander::onProcessFinished));
			process->Start();
			_processes.push_back(process);
		}
		
		initializeNextTask();
		
		_server.Run();
	}
}

void ProcessCommander::continueReadQualityTablesTask(ServerConnectionPtr serverConnection)
{
	const std::string &hostname = serverConnection->Hostname();
	
	boost::mutex::scoped_lock lock(_mutex);
	NodeMap::iterator iter = _nodeMap.find(hostname);
	if(iter == _nodeMap.end())
	{
		serverConnection->StopClient();
	}
	else {
		std::deque<ClusteredObservationItem> &items = iter->second;
		if(items.empty())
		{
			serverConnection->StopClient();
			_nodeMap.erase(iter);
			if(_nodeMap.empty())
			{
				removeCurrentTask();
				initializeNextTask();
				lock.unlock();
				onConnectionAwaitingCommand(serverConnection);
			}
		}
		else
		{
			const std::string msFilename = items.front().LocalPath();
			items.pop_front();
			StatisticsCollection *statisticsCollection = new StatisticsCollection();
			HistogramCollection *histogramCollection = new HistogramCollection();
			serverConnection->ReadQualityTables(msFilename, *statisticsCollection, *histogramCollection);
		}
	}
}

void ProcessCommander::continueReadAntennaTablesTask(ServerConnectionPtr serverConnection)
{
	boost::mutex::scoped_lock lock(_mutex);
	removeCurrentTask();
	initializeNextTask();
	
	const std::string &hostname = serverConnection->Hostname();
	NodeMap::iterator iter = _nodeMap.find(hostname);
	const std::string msFilename = iter->second.front().LocalPath();
	std::vector<AntennaInfo> *antennas = new std::vector<AntennaInfo>();
	serverConnection->ReadAntennaTables(msFilename, *antennas);
}

void ProcessCommander::initializeNextTask()
{
	std::cout << "Initializing next task.\n";
	switch(currentTask())
	{
		case ReadQualityTablesTask:
			//makeNodeMap(_observation);
			break;
		case ReadAntennaTablesTask:
			break;
		case NoTask:
			_server.Stop();
			break;
	}
}

void ProcessCommander::makeNodeMap(const ClusteredObservation &observation)
{
	const std::vector<ClusteredObservationItem> &items = observation.GetItems();
	for(std::vector<ClusteredObservationItem>::const_iterator i=items.begin();i!=items.end();++i)
	{
		_nodeMap[i->HostName()].push_back(*i);
	}
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
	serverConnection->SignalError().connect(sigc::mem_fun(*this, &ProcessCommander::onError));
	acceptConnection = true;
}

void ProcessCommander::onConnectionAwaitingCommand(ServerConnectionPtr serverConnection)
{
	const std::string &hostname = serverConnection->Hostname();
	std::cout << "Connection " << hostname << " awaiting commands..." << std::endl;
	
	switch(currentTask())
	{
		case ReadQualityTablesTask:
			continueReadQualityTablesTask(serverConnection);
			break;
		case ReadAntennaTablesTask:
			continueReadAntennaTablesTask(serverConnection);
			break;
		case NoTask:
			serverConnection->StopClient();
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

void ProcessCommander::onError(ServerConnectionPtr connection, const std::string &error)
{
	std::stringstream s;
	s << "On connection with " << connection->Hostname() << ", reported error was: " << error;
	boost::mutex::scoped_lock lock(_mutex);
	_errors.push_back(s.str());
}

void ProcessCommander::onProcessFinished(RemoteProcess &process, bool error, int status)
{
	boost::mutex::scoped_lock lock(_mutex);
	NodeMap::iterator iter = _nodeMap.find(process.ClientHostname());
	if(iter == _nodeMap.end())
	{
		// There were no comments for this client, thus probably finished okay.
	}
	else {
		_nodeMap.erase(iter);
		if(_nodeMap.empty())
		{
			removeCurrentTask();
			initializeNextTask();
		}
	}
	
	if(error)
	{
		std::stringstream s;
		s << "Remote process to " << process.ClientHostname() << " reported an error";
		if(status != 0) s << " (status " << status << ")";
		_errors.push_back(s.str());
	}
}


}
