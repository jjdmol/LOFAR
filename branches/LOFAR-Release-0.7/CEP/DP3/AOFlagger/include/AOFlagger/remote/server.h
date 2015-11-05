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

#ifndef AOREMOTE__SERVER_H
#define AOREMOTE__SERVER_H

#include <AOFlagger/remote/format.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <sigc++/signal.h>

#include "serverconnection.h"

class StatisticsCollection;

namespace aoRemote {

class Server
{
	public:
		Server();
		
		void Run();
		void Stop();
		
		static unsigned PORT() { return 1892; }
		
		sigc::signal<void, ServerConnectionPtr, bool&> &SignalConnectionCreated()
		{
			return _onConnectionCreated;
		}
	private:
		void startAccept();
		void handleAccept(ServerConnectionPtr connection, const boost::system::error_code &error);
		
		boost::asio::io_service _ioService;
		boost::asio::ip::tcp::acceptor _acceptor;
		sigc::signal<void, ServerConnectionPtr, bool&> _onConnectionCreated;
};
	
}

#endif
