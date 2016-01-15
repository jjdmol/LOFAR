//#  ClaimMgrTask.h: singleton class; bridge between controller application 
//#                    and Property Agent
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef GTM_SERVICEBROKER_H
#define GTM_SERVICEBROKER_H

#include <queue>
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>

namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFTimerPort;
  using GCF::TM::GCFPortInterface;
  using GCF::TM::GCFHandler;
  using GCF::RTDB::RTDBPropertySet;
  namespace APL {
    namespace RTDBCommon {

/**
*/

class CMHandler;

class ClaimMgrTask : public GCFTask
{
public:
    ~ClaimMgrTask ();
    static ClaimMgrTask* instance(bool temporary = false);
    static void release();

	// Ask the claimManager to claim an object. An ClaimReply event is send to the given port.
    void claimObject(const string&		objectType,
					 const string&		nameInAppl,
					 GCFPortInterface&	replyPort);
  
	// Ask the claimManager to flee an object. 
    void freeObject (const string&		objectType,
					 const string&		nameInAppl);
private:
	enum {
		RO_UNDEFINED = 0,
		RO_CREATING,
		RO_CREATED,
		RO_ASKED,
		RO_READY
	};

    friend class CMHandler;
    ClaimMgrTask ();		// only allowed by CMHandler

	// state methods
    GCFEvent::TResult operational (GCFEvent& e, GCFPortInterface& p);

	typedef struct cmRequest {
		string				objectType;
		string				objectName;
		GCFPortInterface*	replyPort;
		cmRequest(const string& oType, const string& oName, GCFPortInterface* rPort) : 
					objectType(oType), objectName(oName), replyPort(rPort) {};
	} cmRequest_t;
        
	// data members        
	std::queue<cmRequest_t>	itsRequestPool;		// pool with waiting requests
	GCFPortInterface*		itsReplyPort;		// Port to send the result to
	GCFTimerPort*			itsTimerPort;		// for reconnecting to brokers
	RTDBPropertySet*		itsClaimMgrPS;		// for accessing the ClaimManager
	string					itsObjectType;		// Objecttype of object in claim
	string					itsNameInAppl;		// Name user likes to use
	uint32					itsResolveState;	// Where we are in claiming the object.
	// result fields
	uint32					itsFieldsReceived;
	string					itsResultDPname;
};

class CMHandler : public GCFHandler
{
public:
    ~CMHandler() { itsInstance = 0; }
    void workProc() {}
    void stop () {}
    
private:
    friend class ClaimMgrTask;
    CMHandler();

    static CMHandler*	itsInstance;
    ClaimMgrTask 		itsCMTask;
};
  } // namespace SB
 } // namespace GCF
} // namespace LOFAR
#endif
