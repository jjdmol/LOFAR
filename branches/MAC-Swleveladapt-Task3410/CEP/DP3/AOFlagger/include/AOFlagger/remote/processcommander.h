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
#include "nodecommandmap.h"
#include "remoteprocess.h"
#include "server.h"

#include <AOFlagger/msio/antennainfo.h>

class StatisticsCollection;

namespace aoRemote {
	
class ObservationTimerange;

class ProcessCommander
{
	public:
		ProcessCommander(const ClusteredObservation &observation);
		~ProcessCommander();
		
		void Run(bool finishConnections = true);
		
		static std::string GetHostName();
		const StatisticsCollection &Statistics() const { return *_statisticsCollection; }
		const HistogramCollection &Histograms() const { return *_histogramCollection; }
		const std::vector<AntennaInfo> &Antennas() const { return _antennas; }
		const ObservationTimerange &ObsTimerange() const { return *_observationTimerange; }
		const std::vector<std::string> &Errors() const { return _errors; }
		
		void PushReadQualityTablesTask(StatisticsCollection *dest, HistogramCollection *destHistogram, bool correctHistograms = false)
		{
			_correctHistograms = correctHistograms;
			_tasks.push_back(ReadQualityTablesTask);
			_statisticsCollection = dest;
			_histogramCollection = destHistogram;
		}
		void PushReadDataRowsTask(class ObservationTimerange &timerange)
		{
			_tasks.push_back(ReadDataRowsTask);
			_observationTimerange = &timerange;
		}
		void PushReadAntennaTablesTask() { _tasks.push_back(ReadAntennaTablesTask); }
	private:
		enum Task { NoTask, ReadQualityTablesTask, ReadAntennaTablesTask, ReadDataRowsTask };
		
		void continueReadQualityTablesTask(ServerConnectionPtr serverConnection);
		void continueReadAntennaTablesTask(ServerConnectionPtr serverConnection);
		void continueReadDataRowsTask(ServerConnectionPtr serverConnection);
		
		void onConnectionCreated(ServerConnectionPtr serverConnection, bool &acceptConnection);
		void onConnectionAwaitingCommand(ServerConnectionPtr serverConnection);
		void onConnectionFinishReadQualityTables(ServerConnectionPtr serverConnection, StatisticsCollection &statisticsCollection, HistogramCollection &histogramCollection);
		void onConnectionFinishReadAntennaTables(ServerConnectionPtr serverConnection, std::vector<AntennaInfo> &antennas);
		void onConnectionFinishReadDataRows(ServerConnectionPtr serverConnection, MSRowDataExt *rowData);
		void onError(ServerConnectionPtr connection, const std::string &error);
		void onProcessFinished(RemoteProcess &process, bool error, int status);
		
		Server _server;
		typedef std::vector<ServerConnectionPtr> ConnectionVector;
		ConnectionVector _idleConnections;
		std::vector<RemoteProcess *> _processes;
		
		StatisticsCollection *_statisticsCollection;
		HistogramCollection *_histogramCollection;
		bool _correctHistograms;
		std::vector<AntennaInfo> _antennas;
		class ObservationTimerange *_observationTimerange;
		
		const ClusteredObservation _observation;
		NodeCommandMap _nodeCommands;
		bool _finishConnections;
		
		std::vector<std::string> _errors;
		std::deque<enum Task> _tasks;
		
		/** 
		 * Because the processes have separate threads that can send signals from
		 * their thread, locking is required for accessing data that might be
		 * accessed by the processes.
		 */
		boost::mutex _mutex;
		
		Task currentTask() const {
			if(!_tasks.empty()) return _tasks.front();
			else return NoTask;
		}
		void onCurrentTaskFinished() {
			_tasks.pop_front();
			if(currentTask() == NoTask)
				_server.Stop();
		}
};

}

#endif
