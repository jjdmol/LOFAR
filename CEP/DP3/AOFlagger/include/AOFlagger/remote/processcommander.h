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

#ifndef AOREMOTE__PROCESS_COMMANDER_H
#define AOREMOTE__PROCESS_COMMANDER_H

#include <map>
#include <string>
#include <deque>
#include <vector>

#include "clusteredobservation.h"
#include "remoteprocess.h"
#include "server.h"

#include <AOFlagger/msio/antennainfo.h>

class StatisticsCollection;

namespace aoRemote {

class ProcessCommander
{
	public:
		ProcessCommander(const ClusteredObservation &observation);
		~ProcessCommander();
		
		void Run();
		
		static std::string GetHostName();
		const StatisticsCollection &Statistics() const { return *_collection; }
		const std::vector<AntennaInfo> &Antennas() const { return _antennas; }
		const std::vector<std::string> &Errors() const { return _errors; }
		
		void PushReadQualityTablesTask(StatisticsCollection *dest)
		{
			_tasks.push_back(ReadQualityTablesTask);
			_collection = dest;
		}
		void PushReadAntennaTablesTask() { _tasks.push_back(ReadAntennaTablesTask); }
	private:
		enum Task { NoTask, ReadQualityTablesTask, ReadAntennaTablesTask };
		
		void initializeNextTask();
		
		void continueReadQualityTablesTask(ServerConnectionPtr serverConnection);
		void continueReadAntennaTablesTask(ServerConnectionPtr serverConnection);
		
		void makeNodeMap(const ClusteredObservation &observation);
		void onConnectionCreated(ServerConnectionPtr serverConnection, bool &acceptConnection);
		void onConnectionAwaitingCommand(ServerConnectionPtr serverConnection);
		void onConnectionFinishReadQualityTables(ServerConnectionPtr serverConnection, StatisticsCollection &collection);
		void onConnectionFinishReadAntennaTables(ServerConnectionPtr serverConnection, std::vector<AntennaInfo> &antennas);
		void onError(ServerConnectionPtr connection, const std::string &error);
		
		Server _server;
		typedef std::map<std::string, std::deque<ClusteredObservationItem> > NodeMap;
		NodeMap _nodeMap;
		std::vector<RemoteProcess *> _processes;
		StatisticsCollection *_collection;
		std::vector<AntennaInfo> _antennas;
		const ClusteredObservation _observation;
		
		std::vector<std::string> _errors;
		std::deque<enum Task> _tasks;
		
		Task currentTask() const {
			if(!_tasks.empty()) return _tasks.front();
			else return NoTask;
		}
		void removeCurrentTask() {
			_tasks.pop_front();
		}
};

}

#endif
