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

#include <AOFlagger/remote/server.h>

#include <vector>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

#include <AOFlagger/remote/serverconnection.h>

namespace aoRemote
{

Server::Server()
	: _acceptor(_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT()))
{
}

void Server::Run()
{
	startAccept();
	_ioService.run();
}

void Server::startAccept()
{
	ServerConnection *connection = new ServerConnection(_ioService);
	
	_acceptor.async_accept(connection->Socket(), boost::bind(&Server::handleAccept, this, connection, boost::asio::placeholders::error));
}

void Server::handleAccept(ServerConnection *connection, const boost::system::error_code &error)
{
	if (_acceptor.is_open())
	{
		if (!error)
		{
			connection->Start();
		}

		startAccept();
	}
}

void Server::Stop()
{
	_acceptor.close();
}


}
