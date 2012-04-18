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

#include <AOFlagger/remote/serverconnection.h>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>

#include <boost/bind.hpp>

#include <AOFlagger/remote/format.h>

#include <AOFlagger/util/autoarray.h>

#include <AOFlagger/quality/statisticscollection.h>
#include <AOFlagger/quality/histogramcollection.h>

namespace aoRemote
{

ServerConnection::ServerConnection(boost::asio::io_service &ioService) :
	_socket(ioService), _buffer(0)
{
}

ServerConnection::~ServerConnection()
{
	if(_buffer != 0)
		delete[] _buffer;
}

void ServerConnection::Start()
{
	InitialBlock initialBlock;
	initialBlock.blockIdentifier = InitialId;
	initialBlock.blockSize = sizeof(initialBlock);
	initialBlock.options = 0;
	initialBlock.protocolVersion = AO_REMOTE_PROTOCOL_VERSION;
	
	boost::asio::write(_socket, boost::asio::buffer(&initialBlock, sizeof(initialBlock)));
	
	prepareBuffer(sizeof(InitialResponseBlock));
	boost::asio::async_read(_socket, boost::asio::buffer(_buffer, sizeof(InitialResponseBlock)), boost::bind(&ServerConnection::onReceiveInitialResponse, shared_from_this()));
}

void ServerConnection::onReceiveInitialResponse()
{
	InitialResponseBlock initialResponse = *reinterpret_cast<InitialResponseBlock*>(_buffer);
	enum ErrorCode errorCode = (enum ErrorCode) initialResponse.errorCode;
	if(initialResponse.blockIdentifier != InitialResponseId || initialResponse.blockSize != sizeof(initialResponse))
		throw std::runtime_error("Bad response from client during initial response");
	if(errorCode != NoError)
		throw std::runtime_error(std::string("Error reported by client during initial response: ") + ErrorStr::GetStr(errorCode));
	if(initialResponse.negotiatedProtocolVersion != AO_REMOTE_PROTOCOL_VERSION)
		throw std::runtime_error("Client seems to run different protocol version");
	if(initialResponse.hostNameSize == 0 || initialResponse.hostNameSize > 65536)
		throw std::runtime_error("Client did not send proper hostname");
	
	char hostname[initialResponse.hostNameSize + 1];
	boost::asio::read(_socket, boost::asio::buffer(hostname, initialResponse.hostNameSize));
	hostname[initialResponse.hostNameSize] = 0;
	_hostname = hostname;
	
	_onAwaitingCommand(shared_from_this());
}

void ServerConnection::StopClient()
{
	RequestBlock requestBlock;
	requestBlock.blockIdentifier = RequestId;
	requestBlock.blockSize = sizeof(requestBlock);
	requestBlock.dataSize = 0;
	requestBlock.request = StopClientRequest;
	boost::asio::write(_socket, boost::asio::buffer(&requestBlock, sizeof(requestBlock)));
}

void ServerConnection::ReadQualityTables(const std::string &msFilename, StatisticsCollection &collection, HistogramCollection &histogramCollection)
{
	_collection = &collection;
	_histogramCollection = &histogramCollection;
	
	std::stringstream reqBuffer;
	
	RequestBlock requestBlock;
	ReadQualityTablesRequestOptions options;
	
	requestBlock.blockIdentifier = RequestId;
	requestBlock.blockSize = sizeof(requestBlock);
	requestBlock.dataSize = sizeof(options.flags) + msFilename.size();
	requestBlock.request = ReadQualityTablesRequest;
	reqBuffer.write(reinterpret_cast<char *>(&requestBlock), sizeof(requestBlock));
	
	options.flags = 0;
	options.msFilename = msFilename;
	reqBuffer.write(reinterpret_cast<char *>(&options.flags), sizeof(options.flags));
	reqBuffer.write(reinterpret_cast<const char *>(options.msFilename.c_str()), options.msFilename.size());
	
	boost::asio::write(_socket, boost::asio::buffer(reqBuffer.str()));
	
	prepareBuffer(sizeof(GenericReadResponseHeader));
	boost::asio::async_read(_socket, boost::asio::buffer(_buffer, sizeof(GenericReadResponseHeader)),
		boost::bind(&ServerConnection::onReceiveQualityTablesResponseHeader, shared_from_this()));
}

void ServerConnection::ReadAntennaTables(const std::string &msFilename, std::vector<AntennaInfo> &antennas)
{
	_antennas = &antennas;
	
	std::stringstream reqBuffer;
	
	RequestBlock requestBlock;
	ReadAntennaTablesRequestOptions options;
	
	requestBlock.blockIdentifier = RequestId;
	requestBlock.blockSize = sizeof(requestBlock);
	requestBlock.dataSize = sizeof(options.flags) + msFilename.size();
	requestBlock.request = ReadAntennaTablesRequest;
	reqBuffer.write(reinterpret_cast<char *>(&requestBlock), sizeof(requestBlock));
	
	options.flags = 0;
	options.msFilename = msFilename;
	reqBuffer.write(reinterpret_cast<char *>(&options.flags), sizeof(options.flags));
	reqBuffer.write(reinterpret_cast<const char *>(options.msFilename.c_str()), options.msFilename.size());
	
	boost::asio::write(_socket, boost::asio::buffer(reqBuffer.str()));
	
	std::cout << "Requesting antenna tables from " << Hostname() << "...\n";
	
	prepareBuffer(sizeof(GenericReadResponseHeader));
	boost::asio::async_read(_socket, boost::asio::buffer(_buffer, sizeof(GenericReadResponseHeader)),
		boost::bind(&ServerConnection::onReceiveAntennaTablesResponseHeader, shared_from_this()));
}

void ServerConnection::handleError(const GenericReadResponseHeader &header)
{
	std::stringstream s;
	s << "Client reported \"" << ErrorStr::GetStr(header.errorCode) << '\"';
	if(header.dataSize > 0)
	{
		char message[header.dataSize+1];
		boost::asio::read(_socket, boost::asio::buffer(message, header.dataSize));
		message[header.dataSize] = 0;
		s << " (detailed info: " << message << ')';
	}
	_onError(shared_from_this(), s.str());
}

void ServerConnection::onReceiveQualityTablesResponseHeader()
{
	GenericReadResponseHeader responseHeader = *reinterpret_cast<GenericReadResponseHeader*>(_buffer);
	if(responseHeader.blockIdentifier != GenericReadResponseHeaderId || responseHeader.blockSize != sizeof(responseHeader))
	{
		_onError(shared_from_this(), "Bad response from client upon read quality tables request");
		StopClient();
	}
	else if(responseHeader.errorCode != NoError)
	{
		handleError(responseHeader);
		_onAwaitingCommand(shared_from_this());
	}
	else {
		prepareBuffer(responseHeader.dataSize);
		boost::asio::async_read(_socket, boost::asio::buffer(_buffer, responseHeader.dataSize),
			boost::bind(&ServerConnection::onReceiveQualityTablesResponseData, shared_from_this(), responseHeader.dataSize));
	}
}

void ServerConnection::onReceiveQualityTablesResponseData(size_t dataSize)
{
	std::istringstream stream;
	if(stream.rdbuf()->pubsetbuf(_buffer, dataSize) == 0)
		throw std::runtime_error("Could not set string buffer");
	
	std::cout << "Received quality table of size " << dataSize << "." << std::endl;
	_collection->Unserialize(stream);
	if(stream.tellg() != (std::streampos) dataSize)
	{
		size_t histogramTablesSize = dataSize - stream.tellg();
		std::cout << "Processing histogram tables of size " << histogramTablesSize << "." << std::endl;
		_histogramCollection->Unserialize(stream);
	}

	_onFinishReadQualityTables(shared_from_this(), *_collection, *_histogramCollection);
	_onAwaitingCommand(shared_from_this());
}

void ServerConnection::onReceiveAntennaTablesResponseHeader()
{
	std::cout << "Receiving antenna tables...\n";
	GenericReadResponseHeader responseHeader = *reinterpret_cast<GenericReadResponseHeader*>(_buffer);
	if(responseHeader.blockIdentifier != GenericReadResponseHeaderId || responseHeader.blockSize != sizeof(responseHeader))
	{
		_onError(shared_from_this(), "Bad response from client upon read antenna tables request");
		StopClient();
	}
	else if(responseHeader.errorCode != NoError)
	{
		handleError(responseHeader);
		_onAwaitingCommand(shared_from_this());
	}
	else {
		prepareBuffer(responseHeader.dataSize);
		boost::asio::async_read(_socket, boost::asio::buffer(_buffer, responseHeader.dataSize),
			boost::bind(&ServerConnection::onReceiveAntennaTablesResponseData, shared_from_this(), responseHeader.dataSize));
	}
}

void ServerConnection::onReceiveAntennaTablesResponseData(size_t dataSize)
{
	std::istringstream stream;
	if(stream.rdbuf()->pubsetbuf(_buffer, dataSize) == 0)
		throw std::runtime_error("Could not set string buffer");
	
	std::cout << "Received antenna table of size " << dataSize << "." << std::endl;
	size_t count = Serializable::UnserializeUInt32(stream);
	for(size_t i=0;i<count;++i)
	{
		_antennas->push_back(AntennaInfo());
		_antennas->rbegin()->Unserialize(stream);
	}

	_onFinishReadAntennaTables(shared_from_this(), *_antennas);
	_onAwaitingCommand(shared_from_this());
}

}

