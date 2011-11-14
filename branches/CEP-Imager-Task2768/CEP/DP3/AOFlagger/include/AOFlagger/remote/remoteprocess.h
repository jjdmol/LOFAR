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

#ifndef AOREMOTE__REMOTE_PROCESS_H
#define AOREMOTE__REMOTE_PROCESS_H

#include <sstream>

#include <boost/thread/thread.hpp>

#include "clusteredobservation.h"

namespace aoRemote
{

class RemoteProcess
{
	public:
		RemoteProcess(const std::string &clientHostName, const std::string &serverHostName)
		: _clientHostName(clientHostName), _serverHostName(serverHostName), _thread(ThreadFunctor(*this)), _running(true)
		{
		}
		
		~RemoteProcess()
		{
			Join();
		}
		
		void Join()
		{
			if(_running)
			{
				_thread.join();
				_running = false;
			}
		}
	private:
		RemoteProcess(const RemoteProcess &source) { }
		void operator=(const RemoteProcess &source) { }
		
		struct ThreadFunctor
		{
			ThreadFunctor(RemoteProcess &process) : _remoteProcess(process) { }
			RemoteProcess &_remoteProcess;
			void operator()()
			{
				std::ostringstream commandLine;
				commandLine
					<< "ssh " << _remoteProcess._clientHostName << " -C \"aoremoteserver connect "
					<< _remoteProcess._serverHostName << "\"";
				system(commandLine.str().c_str());
			}
		};
		
		const ClusteredObservationItem _item;
		const std::string _clientHostName, _serverHostName;
		boost::thread _thread;
		bool _running;
};

}

#endif
