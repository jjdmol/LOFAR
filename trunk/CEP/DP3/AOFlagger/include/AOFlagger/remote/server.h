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

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace aoRemote {

class Server
{
	public:
		Server();
		
		static unsigned PORT() { return 1892; }
		
		enum BlockId { InitialId = 1, InitialResponseId =2, RequestId = 3, ReadQualityTablesHeaderId = 10 };
		enum ErrorCodes { NoError = 0, ProtocolNotUnderstoodError = 10 } ;
		enum RequestType { StopServer = 0, ReadQualityTables = 1 };
		struct InitialBlock
		{
			int16_t blockSize;
			int16_t blockIdentifier;
			int16_t protocolVersion;
			int16_t options;
		};
		struct InitialResponseBlock
		{
			int16_t blockSize;
			int16_t blockIdentifier;
			int16_t negotiatedProtocolVersion;
			int16_t errorCode;
		};
		struct RequestBlock
		{
			int16_t blockSize;
			int16_t blockIdentifier;
			int16_t request;
			int16_t dataSize;
		};
		struct ReadQualityTablesHeader
		{
			int16_t blockSize;
			int16_t blockIdentifier;
			int16_t errorCode;
		};
	private:
		boost::asio::io_service _ioService;
		
		void handleReadQualityTables(boost::asio::ip::tcp::socket &socket, unsigned dataSize);
};
	
}

#endif
