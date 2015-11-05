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

#include <boost/enable_shared_from_this.hpp>

#include <sigc++/signal.h>

#include <AOFlagger/remote/format.h>

#include <AOFlagger/msio/antennainfo.h>

class StatisticsCollection;

namespace aoRemote {

typedef boost::shared_ptr<class ServerConnection> ServerConnectionPtr;

class ServerConnection : public boost::enable_shared_from_this<ServerConnection>

{
	public:
		static ServerConnectionPtr Create(boost::asio::io_service &ioService)
		{
			return ServerConnectionPtr(new ServerConnection(ioService));
		}
		~ServerConnection();
		
		void StopClient();
		void ReadQualityTables(const std::string &msFilename, class StatisticsCollection &collection);
		void ReadAntennaTables(const std::string &msFilename, std::vector<AntennaInfo> &antennas);
		void Start();
		
		boost::asio::ip::tcp::socket &Socket() { return _socket; }
		
		sigc::signal<void, ServerConnectionPtr> &SignalAwaitingCommand() { return _onAwaitingCommand; }
		sigc::signal<void, ServerConnectionPtr, StatisticsCollection&> &SignalFinishReadQualityTables() { return _onFinishReadQualityTables; }
		sigc::signal<void, ServerConnectionPtr, std::vector<AntennaInfo>&> &SignalFinishReadAntennaTables() { return _onFinishReadAntennaTables; }
		sigc::signal<void, ServerConnectionPtr, const std::string&> &SignalError() { return _onError; }
		
		const std::string &Hostname() const { return _hostname; }
	private:
		ServerConnection(boost::asio::io_service &ioService);
		boost::asio::ip::tcp::socket _socket;
		std::string _hostname;
		
		sigc::signal<void, ServerConnectionPtr> _onAwaitingCommand;
		sigc::signal<void, ServerConnectionPtr, StatisticsCollection&> _onFinishReadQualityTables;
		sigc::signal<void, ServerConnectionPtr, std::vector<AntennaInfo>&> _onFinishReadAntennaTables;
		sigc::signal<void, ServerConnectionPtr, const std::string&> _onError;
		
		char *_buffer;
		
		void onReceiveInitialResponse();
		
		void onReceiveQualityTablesResponseHeader();
		void onReceiveQualityTablesResponseData(size_t dataSize);
		
		void onReceiveAntennaTablesResponseHeader();
		void onReceiveAntennaTablesResponseData(size_t dataSize);
		
		void prepareBuffer(size_t size)
		{
			if(_buffer != 0) delete[] _buffer;
			_buffer = new char[size];
		}
		
		void handleError(const GenericReadResponseHeader &header);
		
		StatisticsCollection *_collection;
		std::vector<AntennaInfo> *_antennas;
};
	
}

#endif
