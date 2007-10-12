//#  ACRequestPool.cc: one line description
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
//#  Note: this source is read best with tabstop 4.
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <arpa/inet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <Common/SystemUtil.h>
#include "ACRequestPool.h"

namespace LOFAR {
  namespace ACC {

ACRequestPool::ACRequestPool(uint16			firstPortNr, 
							 uint16			nrOfPorts) :
	itsFirstPort(firstPortNr),
	itsLastPort (firstPortNr+nrOfPorts-1),
	itsNextPort (firstPortNr)
{}

ACRequestPool::~ACRequestPool()
{}

//
// add(anACR)
//
// Add (a copy of) the given ACRequest to the pool
//
void	ACRequestPool::add    (const ACRequest&		anACR)
{
	LOG_TRACE_OBJ_STR("ACRPool: add " << anACR.itsRequester);

	itsPool.push_back(new ACRequest(anACR));		// save ptr to copy
}

//
// remove(anACRName)
//
// Remove the ACRequest with the given name from the pool
//
void	ACRequestPool::remove (const string&		anACRName)
{
	LOG_TRACE_OBJ_STR("ACRPool: remove " << anACRName);

	ACRequest*	ACRPtr = search(anACRName);
	if (ACRPtr) {
		itsPool.remove(ACRPtr);			// remove ptr from pool
		delete ACRPtr;					// remove object itself
	}
}

//
// search(anACRName)
//
// Searches for an ACRequest with the given name. Returns 0 if not found.
//
ACRequest*	ACRequestPool::search (const string&		anACRName)
{
	iterator	iter = itsPool.begin();

	while (iter != itsPool.end()) {
		// Does name match?
		if (!strcmp(anACRName.c_str(), (*iter)->itsRequester)) {
			return (*iter);
		}
		iter++;
	}

	return (0);		// No match found
}

//
// save(filename)
//
// Save the pool to survive power failures.
//
bool	ACRequestPool::save (const string&		aFilename)
{
	static bool		showedWarning = false;

	ofstream	oFile(aFilename.c_str(), ofstream::out | ofstream::trunc
													   | ofstream::binary);

	// If the file can not be opened warn the operator once.
	if (!oFile) {
		if (!showedWarning) {
			LOG_WARN("ACDaemon is not powerfailure save!");
			showedWarning = true;
		}
		return (false);
	}

	LOG_TRACE_RTTI_STR("Saving " << itsPool.size() << " ACrequests to file "
					   << aFilename);

	uint16	writeVersion = ACREQUEST_VERSION;
	uint16  count = itsPool.size();
	oFile.write((char*)(&writeVersion), sizeof(writeVersion));
	oFile.write((char*)&count, sizeof(count));

	iterator	iter = itsPool.begin();
	while (iter != itsPool.end()) {
		oFile.write((char*)(*iter), sizeof(ACRequest));
		iter++;
	}

	oFile.close();
	return (true);
}

//
// load (filename)
//
// readin file with old admin info
//
bool	ACRequestPool::load (const string&		aFilename)
{
	ifstream	iFile(aFilename.c_str(), ifstream::in | ifstream::binary);
	if (!iFile) {					// No file is OK
		return (false);				// tell we did not load anything
	}

	uint16		count;
	uint16		readVersion;

	iFile.read((char*)&readVersion, sizeof(readVersion));// for future V. control
	iFile.read((char*)&count, sizeof(count));			 // nr elements in file

	LOG_INFO_STR("Loading " << count << " ACrequests from file "
					   << aFilename);

	while (count) {
		ACRequest		ACR;

		iFile.read((char*)&ACR, sizeof (ACRequest));
		ACR.itsPingtime = time(0);				// reset pingtime
		ACR.itsState = ACRloaded;
	
		// report what we loaded.
		in_addr		IPaddr;
		IPaddr.s_addr = ACR.itsAddr;
		LOG_INFO_STR("Application " << ACR.itsRequester << " was at " << 
							inet_ntoa(IPaddr) << "," << ntohs(ACR.itsPort));
		add (ACR);
		--count;
	}

	iFile.close();
	return (true);
}

bool ACRequestPool::assignNewPort(ACRequest*	anACR) 
{
	// Controller always run on my own machine
	anACR->itsAddr = myIPV4Address();	// network byte order

	uint16	startingPort = itsNextPort;
	uint16	freePort;
	bool	found		 = false;
	do {
		// scan pool to see if number is (still) in use.
		iterator	iter = itsPool.begin();
		bool		inUse = false;
		while (!inUse && iter != itsPool.end()) {
			if ((*iter)->itsPort == itsNextPort) {
				inUse = true;
			}
			++iter;
		}
		if ((found = !inUse)) {
			freePort = itsNextPort;
		}
		itsNextPort++;				// increment nextPort for next round.
		if (itsNextPort > itsLastPort) {
			itsNextPort = itsFirstPort;
		}
	} while (!found && itsNextPort != startingPort);

	if (!found) {
		LOG_ERROR ("Pool of TCP portnumber of ACDaemon is full. "
				   "Can not start a new Application Controller");
		return (false);
	}

	// fill in portnumber
	anACR->itsPort = htons(freePort);

	// log assignment
	in_addr		IPaddr;
	IPaddr.s_addr = anACR->itsAddr;
	LOG_INFO_STR (inet_ntoa(IPaddr) << ", " << freePort <<
				   " assigned to " << anACR->itsRequester);

	return (true);
}

//
// cleanupACPool()
//
// Remove AC's that did not repsond for more than 5 minutes.
// Returns true if one or more elements were removed from the pool.
//
bool ACRequestPool::cleanup(int32	warnTime, int32	cleanTime)
{
	LOG_TRACE_STAT("Checking sign of life of AC's");

	time_t		curTime  = time(0);
	bool    	modified = false;

	iterator	iter     = itsPool.begin();
	while (iter != itsPool.end()) {
		// No sign of life for a long time anymore?? Remove from pool.
		if ((curTime - (*iter)->itsPingtime) > cleanTime) {
			// Don't complain about loaded (old) stuff
			if ((*iter)->itsState != ACRloaded) {
				LOG_INFO_STR ("No more pings from " << (*iter)->itsRequester <<
						  ". Removing it from pool");
			}
			
			// iter is destroyed in remove, prepare new iter.
			iterator	tmp_iter = iter;
			++tmp_iter;

			// remove the AC from the pool
			remove((*iter)->itsRequester);
			modified = true;
			iter = tmp_iter;
		} 
		else {	// When pings stay out a while inform operator
			if ((curTime - (*iter)->itsPingtime) > warnTime) {
				// Don't complain about loaded (old) stuff
				if ((*iter)->itsState != ACRloaded) {
					LOG_INFO_STR ("No sign of life from " <<
								  (*iter)->itsRequester);
					(*iter)->itsState = ACRlosing;
				}
			}
			++iter;
		}
	}

	return (modified);
}

  } // namespace ACC
} // namespace LOFAR
