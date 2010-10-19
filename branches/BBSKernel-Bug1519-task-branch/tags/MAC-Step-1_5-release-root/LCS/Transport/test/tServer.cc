//#  tServer.cc: Example program how a build a server with libtransport.
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#ifdef USE_NOSOCKETS
int main()
{
  return 3;
}

#else

//# Includes
#include <time.h>
#include <libgen.h>
#include <Common/LofarLogger.h>
#include <Transport/TH_Socket.h>
#include <Transport/ConnectionPool.h>
#include <DH_Socket.h>

using namespace LOFAR;

//
// This program demonstrates how an Application Process should communicate
// with the Application Controller. It shows the minimal implementation possible:
// [A] Connect to the AC.
// [B] Register at the AC so we receive messages.
// [C] See if the are new messages arrived.
// [D] Dispatch the message to the right routine in our APCmdImpl.
// [E] Unregister at the AC.
//
// The places where other program code should be added are marked with
// IMPLEMENT.
//
// Note: The structure of the program is conform the MAIN.cc template.
//
int main (int argc, char *argv[]) {

	// Always bring up the logger first
	string  progName = basename(argv[0]);
	INIT_LOGGER(progName.c_str());

	// Check invocation syntax
	if (argc < 2) {
		LOG_FATAL_STR("Invocation error, syntax: " << progName << 
					  " portnr");
		return (-1);
	}

	LOG_INFO_STR("Starting up: " << argv[0] << "(" << argv[1] << ")");

	try {
		// start listener
		Socket listenSocket("server", argv[1], Socket::TCP);
		ASSERTSTR(listenSocket.ok(),
							"Cannot start listener at port " << argv[1]);

		// prepare filedesc mask and add listener to it.
		fd_set	socketSet;
		FD_ZERO (&socketSet);
		FD_SET (listenSocket.getSid(), &socketSet);
		int32	highestID = listenSocket.getSid();
		int32	lowestID  = highestID;
		
		// Allocate two pools, one for reading one for writing
		ConnectionPool	ConnReadPool;
		ConnectionPool	ConnWritePool;

		// Enter main loop
		bool	alive = true;
		while (alive) {
			fd_set	scanSet = socketSet;
			LOG_DEBUG_STR("select:" << highestID+1);
			int32 selResult = select(highestID+1, &scanSet, 0, 0, 0);

			// -1 may be an interrupt or programming-error
			if (selResult == -1) {
				if  (errno == EINTR) {		// ignore interrupts
					continue;
				}
				// Oops, not an interrupt, bail out.
				THROW(Exception, "'select' returned serious error: " <<
						errno << ":" << strerror(errno));
			}

			// Event on listener socket? -> handle connection request
			if (FD_ISSET(listenSocket.getSid(), &scanSet)) {
				// Get the new datasocket
				Socket*	    dataSocket = listenSocket.accept(-1);
				ASSERTSTR(dataSocket,
						  "Serious problems on listener: " << 
						  listenSocket.errstr());
				dataSocket->setBlocking(true);

				LOG_DEBUG_STR("New connection, ID = " << dataSocket->getSid());

				// Note: We reuse 1 DH and 1 TH for read and write you may also
				//       use seperate DHs and THs for reading and writing.
				DH_Socket*  aDHS = new DH_Socket();
				TH_Socket*  aTHS = new TH_Socket(dataSocket);
//				Connection*	readConn  = new Connection("read", 0, aDHS, aTHS);
//				Connection* writeConn = new Connection("write", aDHS, 0, aTHS);

				aDHS->init();		// DONT FOGET THIS!!!!!!
				aDHS->setBufferSize(250);

				// Add read- and write-connection to the pools
				ConnReadPool.add (dataSocket->getSid(),
								  new Connection("read",  0,    aDHS, aTHS));
				ConnWritePool.add(dataSocket->getSid(),
								  new Connection("write", aDHS, 0,    aTHS));

				// Add datasocket to scan-mask
				FD_SET(dataSocket->getSid(), &socketSet);
				if (highestID) {
					highestID = dataSocket->getSid();
				}
			}
	
			// Now scan read of resultset and handle those events
			for (int32 id = lowestID+1; id <= highestID; id++) {
				if (!FD_ISSET(id, &scanSet)) {
					continue;
				}
				LOG_DEBUG_STR("Data on ID: " << id);

				// Read on dataSocket
				Connection*		readConn = ConnReadPool.getConn(id);
				readConn->read();
				readConn->waitForRead();

				// Did peer hang up?
				if (!readConn->isConnected()) {
					LOG_DEBUG_STR("Disconnect on ID: " << id);

					// Pitty, I need these pointers only for 'delete'
					DH_Socket*	theDH = dynamic_cast<DH_Socket*>(readConn->getDataHolder());
					TH_Socket*	theTH = dynamic_cast<TH_Socket*>(readConn->getTransportHolder());
					ConnReadPool.remove(id);
					ConnWritePool.remove(id);
					delete theDH;
					delete theTH;
//					delete theConn;
					FD_CLR(id, &socketSet);
					if (id == highestID) {
						while (highestID > lowestID && 
											!FD_ISSET(highestID, &socketSet)) {
							highestID--;
						}
					}
					continue;
				}

				// modify info
				LOG_DEBUG_STR("preparing answer for ID: " << id);
				DH_Socket*	aDHS = dynamic_cast<DH_Socket*>
												   (readConn->getDataHolder());
				int32 nrBytes = aDHS->getCounter();
				uchar*	buffer = static_cast<uchar*>(aDHS->getBuffer());
				for (int32 b = 0; b < nrBytes; b++) {
					buffer[b] ^= 0xFF;
				}
					
				// Write answer to dataSocket
				LOG_DEBUG_STR("Writing data to ID: " << id);
				Connection*		writeConn = ConnWritePool.getConn(id);
				writeConn->write();
				writeConn->waitForWrite();
			} // for all IDs
		}	// while alive

		LOG_INFO_STR("Shutting down: " << argv[0]);

		// IMPLEMENT: do all neccesary shutdown actions.
	}
	catch (Exception&	ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		LOG_FATAL_STR(argv[0] << " terminated by exception!");
		return(1);
	}


	LOG_INFO_STR(argv[0] << " terminated normally");
	return (0);
}

#endif
