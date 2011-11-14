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
#include <vector>

#include "clusteredobservation.h"
#include "remoteprocess.h"
#include "server.h"

namespace aoRemote {

class ProcessCommander
{
	public:
		ProcessCommander(const ClusteredObservation &observation);
		~ProcessCommander();
		
		static std::string GetHostName();
	private:
		void makeNodeMap(const ClusteredObservation &observation);
		
		Server _server;
		std::map<std::string, std::vector<ClusteredObservationItem> > _nodeMap;
		std::vector<RemoteProcess *> _processes;
};

}

#endif
