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

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <AOFlagger/quality/qualitytablesformatter.h>
#include <AOFlagger/quality/statisticscollection.h>

namespace aoRemote
{

Server::Server()
{
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), PORT());
	boost::asio::ip::tcp::acceptor acceptor(_ioService, endpoint);
	boost::asio::ip::tcp::socket socket(_ioService);
	acceptor.accept(socket);
	
	struct InitialBlock initialBlock;
	boost::asio::read(socket, boost::asio::buffer(&initialBlock, sizeof(initialBlock)));
	
	struct InitialResponseBlock initialResponse;
	initialResponse.blockIdentifier = InitialResponseId;
	initialResponse.blockSize = sizeof(initialResponse);
	initialResponse.negotiatedProtocolVersion = 1;
	if(initialBlock.protocolVersion != 1 || initialBlock.blockSize != sizeof(initialResponse) || initialBlock.blockIdentifier != InitialId)
	{
		initialResponse.errorCode = ProtocolNotUnderstoodError;
		boost::asio::write(socket, boost::asio::buffer(&initialResponse, sizeof(initialResponse)));
		return;
	}
	initialResponse.errorCode = NoError;
	boost::asio::write(socket, boost::asio::buffer(&initialResponse, sizeof(initialResponse)));

	while(true)
	{
		struct RequestBlock requestBlock;
		boost::asio::read(socket, boost::asio::buffer(&requestBlock, sizeof(requestBlock)));
		
		enum RequestType request = (enum RequestType) requestBlock.request;
		switch(request)
		{
			case StopServer:
				return;
			case ReadQualityTables:
				handleReadQualityTables(socket, requestBlock.dataSize);
		}
	}
}

void Server::handleReadQualityTables(boost::asio::ip::tcp::socket &socket, unsigned dataSize)
{
	try {
		char data[dataSize+1];
		boost::asio::read(socket, boost::asio::buffer(data, dataSize));
		data[dataSize] = 0;
		
		const std::string filename(data);
		QualityTablesFormatter formatter(filename);
		StatisticsCollection collection(formatter.GetPolarizationCount());
		collection.Load(formatter);
		
		ReadQualityTablesHeader header;
		header.blockIdentifier = ReadQualityTablesHeaderId;
		header.blockSize = sizeof(header);
		header.errorCode = NoError;
		std::stringstream s;
		collection.Serialize(s);
		
		boost::asio::write(socket, boost::asio::buffer(&header, sizeof(header)));
		boost::asio::write(socket, boost::asio::buffer(s.str()));
	} catch(std::exception &e) {
		ReadQualityTablesHeader header;
		header.blockIdentifier = ReadQualityTablesHeaderId;
		header.blockSize = sizeof(header);
		header.errorCode = UnexpectedExceptionOccured;
		boost::asio::write(socket, boost::asio::buffer(&header, sizeof(header)));
	}
}

} // namespace

