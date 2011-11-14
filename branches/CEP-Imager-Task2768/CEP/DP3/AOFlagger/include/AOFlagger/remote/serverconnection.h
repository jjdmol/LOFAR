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

#include <boost/asio/ip/tcp.hpp>

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
	private:
		boost::asio::ip::tcp::socket _socket;
};
	
}

#endif
