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

#include <AOFlagger/remote/client.h>

#include <typeinfo>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <ms/MeasurementSets/MSColumns.h>

#include <AOFlagger/quality/histogramcollection.h>
#include <AOFlagger/quality/histogramtablesformatter.h>
#include <AOFlagger/quality/qualitytablesformatter.h>
#include <AOFlagger/quality/statisticscollection.h>

#include <AOFlagger/remote/format.h>
#include <AOFlagger/remote/processcommander.h>

#include <AOFlagger/msio/antennainfo.h>
#include <AOFlagger/msio/measurementset.h>

namespace aoRemote
{

Client::Client()
	: _socket(_ioService)
{
}

void Client::Run(const std::string &serverHost)
{
	boost::asio::ip::tcp::resolver resolver(_ioService);
	std::stringstream s;
	s << PORT();
	boost::asio::ip::tcp::resolver::query query(serverHost, s.str());
	boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
	
	boost::asio::ip::tcp::endpoint endpoint = *iter;
	_socket.connect(endpoint);
	
	const std::string hostname = ProcessCommander::GetHostName();
	struct InitialBlock initialBlock;
	boost::asio::read(_socket, boost::asio::buffer(&initialBlock, sizeof(initialBlock)));
	
	struct InitialResponseBlock initialResponse;
	initialResponse.blockIdentifier = InitialResponseId;
	initialResponse.blockSize = sizeof(initialResponse);
	initialResponse.negotiatedProtocolVersion = AO_REMOTE_PROTOCOL_VERSION;
	initialResponse.hostNameSize = hostname.size();
	if(initialBlock.protocolVersion != AO_REMOTE_PROTOCOL_VERSION || initialBlock.blockSize != sizeof(initialBlock) || initialBlock.blockIdentifier != InitialId)
	{
		initialResponse.errorCode = ProtocolNotUnderstoodError;
		boost::asio::write(_socket, boost::asio::buffer(&initialResponse, sizeof(initialResponse)));
		boost::asio::write(_socket, boost::asio::buffer(hostname));
		return;
	}
	initialResponse.errorCode = NoError;
	boost::asio::write(_socket, boost::asio::buffer(&initialResponse, sizeof(initialResponse)));
	boost::asio::write(_socket, boost::asio::buffer(hostname));

	while(true)
	{
		struct RequestBlock requestBlock;
		boost::asio::read(_socket, boost::asio::buffer(&requestBlock, sizeof(requestBlock)));
		
		enum RequestType request = (enum RequestType) requestBlock.request;
		switch(request)
		{
			case StopClientRequest:
				return;
			case ReadQualityTablesRequest:
				handleReadQualityTables(requestBlock.dataSize);
				break;
			case ReadAntennaTablesRequest:
				handleReadAntennaTables(requestBlock.dataSize);
				break;
			case ReadBandTableRequest:
				handleReadBandTable(requestBlock.dataSize);
				break;
			case ReadDataRowsRequest:
				handleReadDataRows(requestBlock.dataSize);
				break;
			default:
				writeGenericReadException("Command not understood by client: server and client versions don't match?");
				break;
		}
	}
}

void Client::writeGenericReadException(const std::exception &e)
{
	std::stringstream s;
	s << "Exception type " << typeid(e).name() << ": " << e.what();
	writeGenericReadException(s.str());
}

void Client::writeGenericReadException(const std::string &s)
{
	GenericReadResponseHeader header;
	header.blockIdentifier = GenericReadResponseHeaderId;
	header.blockSize = sizeof(header);
	header.errorCode = UnexpectedExceptionOccured;
	header.dataSize = s.size();
	
	boost::asio::write(_socket, boost::asio::buffer(&header, sizeof(header)));
	boost::asio::write(_socket, boost::asio::buffer(s));
}

void Client::writeGenericReadError(enum ErrorCode error)
{
	GenericReadResponseHeader header;
	header.blockIdentifier = GenericReadResponseHeaderId;
	header.blockSize = sizeof(header);
	header.errorCode = CouldNotOpenTableError;
	header.dataSize = 0;
	boost::asio::write(_socket, boost::asio::buffer(&header, sizeof(header)));
}

void Client::writeDataResponse(std::ostringstream &buffer)
{
	try {
		GenericReadResponseHeader header;
		header.blockIdentifier = GenericReadResponseHeaderId;
		header.blockSize = sizeof(header);
		header.errorCode = NoError;
		const std::string str = buffer.str();
		header.dataSize = str.size();
		
		boost::asio::write(_socket, boost::asio::buffer(&header, sizeof(header)));
		boost::asio::write(_socket, boost::asio::buffer(str));
	} catch(std::exception &e) {
		writeGenericReadException(e);
	}
}

std::string Client::readStr(unsigned size)
{
	char data[size+1];
	boost::asio::read(_socket, boost::asio::buffer(data, size));
	data[size] = 0;
	return std::string(data);
}

void Client::handleReadQualityTables(unsigned dataSize)
{
	try {
		ReadQualityTablesRequestOptions options;
		boost::asio::read(_socket, boost::asio::buffer(&options.flags, sizeof(options.flags)));
		
		unsigned nameLength = dataSize - sizeof(options.flags);
		options.msFilename = readStr(nameLength);
		
		QualityTablesFormatter formatter(options.msFilename);
		if(!formatter.TableExists(QualityTablesFormatter::KindNameTable))
		{
			writeGenericReadError(CouldNotOpenTableError);
		} else {
			StatisticsCollection collection(formatter.GetPolarizationCount());
			collection.Load(formatter);
			// TODO: maybe we want to configure the following parameter at one point
			collection.LowerTimeResolution(1000);
			
			HistogramTablesFormatter histogramFormatter(options.msFilename);
			HistogramCollection histogramCollection(formatter.GetPolarizationCount());
			const bool histogramsExist = histogramFormatter.HistogramsExist();
			if(histogramsExist)
			{
				histogramCollection.Load(histogramFormatter);
			}
			
			std::ostringstream s;
			collection.Serialize(s);
			if(histogramsExist)
				histogramCollection.Serialize(s);
			writeDataResponse(s);
		}
	} catch(std::exception &e) {
		writeGenericReadException(e);
	}
}

void Client::handleReadAntennaTables(unsigned dataSize)
{
	try {
		ReadAntennaTablesRequestOptions options;
		boost::asio::read(_socket, boost::asio::buffer(&options.flags, sizeof(options.flags)));
		
		unsigned nameLength = dataSize - sizeof(options.flags);
		options.msFilename = readStr(nameLength);
		
		std::ostringstream buffer;
		
		// Serialize the antennae info
		MeasurementSet ms(options.msFilename);
		size_t antennas = ms.AntennaCount();
		Serializable::SerializeToUInt32(buffer, antennas);
		for(unsigned aIndex = 0; aIndex<antennas; ++aIndex)
		{
			AntennaInfo antennaInfo = ms.GetAntennaInfo(aIndex);
			antennaInfo.Serialize(buffer);
		}
		
		writeDataResponse(buffer);
	} catch(std::exception &e) {
		writeGenericReadException(e);
	}
}

void Client::handleReadBandTable(unsigned dataSize)
{
	try {
		ReadBandTableRequestOptions options;
		boost::asio::read(_socket, boost::asio::buffer(&options.flags, sizeof(options.flags)));
		
		unsigned nameLength = dataSize - sizeof(options.flags);
		options.msFilename = readStr(nameLength);
		
		std::ostringstream buffer;
		
		// Serialize the band info
		MeasurementSet ms(options.msFilename);
		if(ms.BandCount() != 1)
			throw std::runtime_error("The number of bands in the measurement set was not 1");
		BandInfo band = ms.GetBandInfo(0);
		band.Serialize(buffer);
		
		writeDataResponse(buffer);
	} catch(std::exception &e) {
		writeGenericReadException(e);
	}
}

void Client::handleReadDataRows(unsigned dataSize)
{
	try {
		ReadDataRowsRequestOptions options;
		boost::asio::read(_socket, boost::asio::buffer(&options.flags, sizeof(options.flags)));
		
		unsigned nameLength = dataSize - sizeof(options.flags);
		options.msFilename = readStr(nameLength);
		
		boost::asio::read(_socket, boost::asio::buffer(&options.startRow, sizeof(options.startRow)));
		boost::asio::read(_socket, boost::asio::buffer(&options.rowCount, sizeof(options.rowCount)));
		
		casa::Table table(options.msFilename);
		casa::ROTableColumn dataCol(table, "DATA");
		
		std::ostringstream buffer;
		
		// Serialize the band info
		MeasurementSet ms(options.msFilename);
		if(ms.BandCount() != 1)
			throw std::runtime_error("The number of bands in the measurement set was not 1");
		BandInfo band = ms.GetBandInfo(0);
		band.Serialize(buffer);
		
		writeDataResponse(buffer);
	} catch(std::exception &e) {
		writeGenericReadException(e);
	}
}

} // namespace

