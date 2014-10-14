//#  ServiceBroker.h: main class of the Property Agent
//#
//#  Copyright (C) 2002-2003
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

#ifndef GSB_CONTROLLER_H
#define GSB_CONTROLLER_H

#include <Common/lofar_vector.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_TCPPort.h>

namespace LOFAR {
	using MACIO::GCFEvent;
	using GCF::TM::GCFTask;
	using GCF::TM::GCFTCPPort;
	using GCF::TM::GCFPortInterface; 
	namespace CUDaemons {

/**
   This is the main class of the Property Agent. It uses a number of helper 
   classes to manage PML requests, registered scopes and use counts of created 
   properties. The assigned port provider supports the possibility to accept 
   more than one connect request from different clients (PML).
*/

class ServiceBroker : public GCFTask
{
public:
	ServiceBroker();
	virtual ~ServiceBroker();
  
private: 
	// state methods
	GCFEvent::TResult initial    (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult operational(GCFEvent& e, GCFPortInterface& p);

	#define	SB_ADMIN_VERSION		0x0100

    typedef struct {
		uint16					portNumber;
		string					serviceName;
		GCFPortInterface*	ownerPort;
    } TServiceInfo;

    void	acceptConnectRequest();
    void	readRanges			();
    uint16	claimPortNumber		(const string& aServiceName, 
								 GCFPortInterface* aPort);
	uint16	reRegisterService	(const string& aServicename, uint16	portnr, 
								 GCFPortInterface*	aPort);
	void	releaseService		(const string& aServiceName);
	void	releasePort			(GCFPortInterface*	aPort);
	uint16	findService			(const string& aServiceName, bool	usedOnly);
	void	saveAdministration	(const string&	aFileName);
	void	loadAdministration	(const string&	aFileName);
	void	cleanupOldRegistrations();

	// define conversions between portnumber and index.
	inline uint16	portNr2Index(uint16		portNumber)
		{	return (portNumber - itsLowerLimit);	}
	inline uint16	index2PortNr(uint16		index)
		{	return (index + itsLowerLimit);	}
    
	//# --- data members ---
	vector<TServiceInfo>		itsServiceList;		// the administration
	GCFTCPPort					itsListener;		// for all SB protocol messages

	string						itsAdminFile;		// to survive crashes

	uint16						itsLowerLimit;		// lowest portnr to assign
	uint16						itsUpperLimit;		// assign till this number
	uint16						itsNrPorts;			// number of ports to manage
	uint16						itsNrFreePorts;		// nr of not-assigned ports

};

  } // namespace CUDaemons
} // namespace LOFAR
#endif
