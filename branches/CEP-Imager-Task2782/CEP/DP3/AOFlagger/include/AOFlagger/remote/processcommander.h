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
		const std::vector<std::string> &Errors() const { return _errors; }
	private:
		void makeNodeMap(const ClusteredObservation &observation);
		void onConnectionCreated(class ServerConnection &serverConnection, bool &acceptConnection);
		void onConnectionAwaitingCommand(class ServerConnection &serverConnection);
		void onConnectionFinishReadQualityTables(class ServerConnection &serverConnection, StatisticsCollection &collection);
		void onError(ServerConnection &connection, const std::string &error);
		
		Server _server;
		typedef std::map<std::string, std::deque<ClusteredObservationItem> > NodeMap;
		NodeMap _nodeMap;
		std::vector<RemoteProcess *> _processes;
		StatisticsCollection *_collection;
		const ClusteredObservation _observation;
		
		std::vector<std::string> _errors;
};

}

#endif
