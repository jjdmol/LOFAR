//#  ACDaemon.cc: launches Application Controllers on demand.
//#
//#  Copyright (C) 2002-2004
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

//# Includes
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/SystemUtil.h>
#include <ALC/ACRequest.h>
#include "ACDaemon.h"
#include "ACDaemonComm.h"
#include "lofarDirs.h"
#include "forkexec.h"

namespace LOFAR {
  namespace ACC {

ACDaemon::ACDaemon(const string&	progName) :
	itsListener   (0),
	itsPingSocket (0),
	itsParamSet   (new ParameterSet),
	itsACPool     (0)
{
	// Read in the parameterfile with network parameters.
	ConfigLocator	aCL;
	string			configFile(progName + ".conf");
	LOG_DEBUG_STR("Using parameterfile: "<< configFile <<"-->"<< aCL.locate(configFile));
	itsParamSet->adoptFile(aCL.locate(configFile));		// May throw

	// Open listener for AC requests.
	itsListener = new Socket("ACdaemon",
							 itsParamSet->getString("ACDaemon.requestportnr"),
							 Socket::TCP);

	// Open listener for ping from AC's.
	itsPingSocket = new Socket("ACdaemon",
							 itsParamSet->getString("ACDaemon.pingportnr"),
							 Socket::UDP);

	// Try to read in an old administration if it is available.
	itsACPool = new ACRequestPool(itsParamSet->getInt32("ACDaemon.ACpoolport"),
								  itsParamSet->getInt32("ACDaemon.ACpoolsize"));
	itsAdminFile = "../etc/" + itsParamSet->getString("ACDaemon.adminfile");
	itsACPool->load(itsAdminFile);

}

ACDaemon::~ACDaemon()
{
	if (itsListener)   { delete itsListener; }
	if (itsPingSocket) { delete itsPingSocket; }
	if (itsParamSet)   { delete itsParamSet; }
	if (itsACPool)     { delete itsACPool; }
}

void ACDaemon::doWork() throw (Exception)
{
	// Setup some values we need in the main loop
	int32	cleanTime = itsParamSet->getTime("ACDaemon.alivetimeout");
	int32	warnTime  = cleanTime / 2;

	// Prepare a fd_set for select
	itsConnSet.add(itsListener->getSid());
	itsConnSet.add(itsPingSocket->getSid());

	LOG_INFO ("ACDaemon: entering main loop");

	while (true) {
		// wait for request or ping
		FdSet	readSet(itsConnSet);
		struct timeval	tv;					// prepare select-timer
		tv.tv_sec       = 60;				// this must be IN the while loop
		tv.tv_usec      = 0;				// because select will change tv
		int32 selResult = select(readSet.highest()+1, readSet.getSet(), 0, 0, &tv);

		// -1 may be an interrupt or a program-error.
		if ((selResult == -1) && (errno != EINTR)) {
			THROW(Exception, "ACDaemon: 'select' returned serious error: " << 
							  errno << ":" << strerror(errno));
		}

		if (selResult == -1) {		// EINTR: ignore
			continue;
		}

		// Ping from an AC?
		if (readSet.isSet(itsPingSocket->getSid())) {
			handlePingMessage();
		}

		// Request for new AC?
		if (readSet.isSet(itsListener->getSid())) {
			handleACRequest();
		}

		// cleanup AC's that did not respond for 5 minutes.
		if (itsACPool->cleanup(warnTime, cleanTime)) {
			itsACPool->save(itsAdminFile);
		}

	} // while
}

//
// handlePingMessage (on UDP socket)
//
void ACDaemon::handlePingMessage()
{
	LOG_TRACE_FLOW("Ping???");

	// Read name of AC from ping message
	char	buffer[ACREQUESTNAMESIZE+1];
	int32 btsRead = itsPingSocket->read(static_cast<void*>(&buffer), 
								  		ACREQUESTNAMESIZE+1);
	if (btsRead != ACREQUESTNAMESIZE+1) {
		LOG_TRACE_STAT_STR("Read on ping port retured: " << btsRead <<
					   	   " iso " << ACREQUESTNAMESIZE+1);
		return;
	}

	// search name in pool
	ACRequest*	ACRPtr = itsACPool->search(&buffer[1]);
	if (!ACRPtr) {
		LOG_TRACE_RTTI_STR ("Received ping from unknown applicationcontroller: " 
					  << buffer << " (ignoring)");
		return;
	}

	if (buffer[0] == AC_LEAVING_SIGN) {
		LOG_DEBUG_STR(&buffer[1] << " leaves ACDaemon pool");
		itsACPool->remove(&buffer[1]);
		itsACPool->save(itsAdminFile);
		return;
	}

	// assume first char is a AC_ALIVE_SIGN, remember ping time, update state.
	LOG_TRACE_COND_STR("AC " << ACRPtr->itsRequester << " is alive");
	ACRPtr->itsPingtime = time(0);
	ACRPtr->itsState    = ACRok;
	
}

//
// handleACRequest()
//
void ACDaemon::handleACRequest()
{
	// Accept the new connection
	Socket*	dataSocket = itsListener->accept(-1);
	ASSERTSTR(dataSocket,
			  "Serious problems on listener socket, exiting! : " <<
			  itsListener->errstr());

	// read request which should come very soon.
	ACRequest 	aRequest;
	uint16		reqSize    = sizeof (ACRequest);
	dataSocket->setBlocking(true);
	uint32 btsRead = dataSocket->read(static_cast<void*>(&aRequest), reqSize);
	if (btsRead != reqSize) {
		LOG_INFO_STR ("ILLEGAL REQUEST SIZE (" << btsRead << 
								" iso " << reqSize << "), IGNORING REQUEST");
		delete dataSocket;
		return;
	}
	aRequest.itsRequester[ACREQUESTNAMESIZE-1] = '\0'; // be save

	// is ACuser already known? Its a reconnect, don't start the AC
	ACRequest*	existingACR;
	bool		startAC = true;
	if ((existingACR = itsACPool->search(aRequest.itsRequester))) {
		LOG_DEBUG_STR("Reconnect of " << aRequest.itsRequester);
		aRequest = *existingACR;			// copy old info
		existingACR->itsState    = ACRnew;	// reset ping state and time
		existingACR->itsPingtime = time(0);	// to prevent early removal
	}
	else {
		// Its an unknown ACuser, add it to the pool
		// assignment of node and portnr based on request params
		if (!itsACPool->assignNewPort(&aRequest)) {
			aRequest.itsAddr = 0;			// return failure to user
			aRequest.itsPort = 0;
			startAC          = false;
			LOG_ERROR("No more ports available for application controllers");
		}
		else {
			// Register new AC in the pool and save the pool
			aRequest.itsPingtime = time(0);
			aRequest.itsState    = ACRnew;
			itsACPool->add(aRequest);
			itsACPool->save(itsAdminFile);
		}
	}

	if (startAC) {
		// Try to launch the AC, if it is already running the
		// new one will fail and the user will reconnect on the old one.
		constructACFile(&aRequest, string(LOFAR_SHARE_LOCATION) + "/" + aRequest.itsRequester+".param");
		string	sysCommand = formatString(itsParamSet->getString("ACDaemon.command").c_str(), 
										  aRequest.itsRequester);
		LOG_DEBUG_STR("Executing '" << sysCommand << "'");
		int32 result = forkexec (sysCommand.c_str());
		LOG_DEBUG_STR ("result = " << result << ", errno = " << errno << ":" << strerror(errno));
	}

	// return the answer to the ACuser
	uint32 btsWritten = dataSocket->write(static_cast<void*>(&aRequest), reqSize);
	if (btsWritten != reqSize) {
		LOG_WARN_STR ("REQUEST FOR " << aRequest.itsRequester << 
					  " COULD NOT BE WRITTEN (" << btsWritten << " iso "  << reqSize << ")");
	}

	delete dataSocket;
}

//
// constructACFile(filename)
//
void ACDaemon::constructACFile(const ACRequest*		anACR,
							   const string&		aFilename) {
	ParameterSet		ACPS;

	uint16	backlog = anACR->itsNrProcs / 10;
	backlog = MAX (5, MIN (backlog, 100));
	
	// TODO: Calculate processportnr

	ACPS.add(KVpair("AC.backlog", 		backlog));
	ACPS.add(KVpair("AC.node",	  		myHostname(false)));
	ACPS.add(KVpair("AC.userportnr",    ntohs(anACR->itsPort)));
	ACPS.add(KVpair("AC.processportnr", 100+ntohs(anACR->itsPort))); // TODO

	ACPS.add("AC.pinghost",     myHostname(false));
	ACPS.add("AC.pingportnr",   itsParamSet->getString("ACDaemon.pingportnr"));
	ACPS.add("AC.pinginterval", itsParamSet->getString("ACDaemon.aliveinterval"));
	ACPS.add("AC.pingID", 	    anACR->itsRequester);
	
	ACPS.writeFile (aFilename);
}

  } // namespace ACC
} // namespace LOFAR
