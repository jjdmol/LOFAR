//#  SHMInfoServer.h: 
//#
//#  Copyright (C) 2002-2008
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

#ifndef SHMINFOSERVER_H
#define SHMINFOSERVER_H

#include <Common/lofar_list.h>
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
//#include <SHMPolicyHandler.h>

//MAXMOD add for antenna coords
#include <APL/CAL_Protocol/SubArray.h>

namespace LOFAR {
	namespace AMI {  

class SHMSession;

/**
*/

class SHMInfoServer : public GCF::TM::GCFTask
{
public:
    SHMInfoServer ();
    ~SHMInfoServer();
    
	// member functions
    GCF::TM::GCFTCPPort& 	getPortProvider();
//    SHMPolicyHandler& 		getPolicyHandler();
    void clientClosed(SHMSession& client);
  
    //MAXMOD
    CAL::AntennaArrays               m_arrays;       // antenna arrays (read from file)

private: 
	// state methods
    MACIO::GCFEvent::TResult initial   (MACIO::GCFEvent& e, GCF::TM::GCFPortInterface& p);
    MACIO::GCFEvent::TResult accepting (MACIO::GCFEvent& e, GCF::TM::GCFPortInterface& p);
        
	// data members        
    GCF::TM::GCFTCPPort*	itsListener;
//	RTDB::DPservice*		itsDPservice;
//    SHMPolicyHandler		_policyHandler;

	// admin members
    typedef list<SHMSession*> TSessions;
    TSessions				_sessionsGarbage;    

};

inline GCF::TM::GCFTCPPort& SHMInfoServer::getPortProvider()
{
	return (*itsListener);
}

//inline SHMPolicyHandler& SHMInfoServer::getPolicyHandler()
//{
//	return (_policyHandler);
//}

 } // namespace AMI
} // namespace LOFAR

#endif
