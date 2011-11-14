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

#ifndef AOREMOTE__SERVER_CONNECTION_H
#define AOREMOTE__SERVER_CONNECTION_H

#include <string>

#include <boost/asio/ip/tcp.hpp>

#include <sigc++/signal.h>

#include <AOFlagger/remote/format.h>

class StatisticsCollection;

namespace aoRemote {

class ServerConnection
{
	public:
		ServerConnection(boost::asio::io_service &ioService);
		
		void StopClient();
		void ReadQualityTables(const std::string &msFilename, class StatisticsCollection &collection);
		void Start();
		
		boost::asio::ip::tcp::socket &Socket() { return _socket; }
		
		sigc::signal<void, ServerConnection&> &SignalAwaitingCommand() { return _onAwaitingCommand; }
		sigc::signal<void, ServerConnection&, StatisticsCollection&> &SignalFinishReadQualityTables() { return _onFinishReadQualityTables; }
		
		const std::string &Hostname() const { return _hostname; }
		
		static std::string GetErrorStr(enum ErrorCode errorCode);
	private:
		boost::asio::ip::tcp::socket _socket;
		std::string _hostname;
		
		sigc::signal<void, ServerConnection&> _onAwaitingCommand;
		sigc::signal<void, ServerConnection&, StatisticsCollection&> _onFinishReadQualityTables;
};
	
}

#endif
