//#  GCF_RTDBPort.h: Port interface via RTDB database
//#
//#  Copyright (C) 2011
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
//#  $Id: GCF_RTDBPort.h 15644 2010-05-10 11:14:01Z loose $

#ifndef GCF_RTDBPORT_H
#define GCF_RTDBPORT_H

#include <Common/lofar_vector.h>
#include <Common/SystemUtil.h>
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/PVSS/GCF_PValue.h>
#include <GCF/PVSS/PVSSresult.h>

namespace LOFAR {
  namespace MACIO {
    class GCFEvent;
  }
  namespace GCF {
    namespace TM {
      class GCFTask;
    }
	namespace PVSS {
	  class PVSSservice;
	  class PVSSresponse;
	}
    namespace RTDB {

// forward declarations

/**
 * This is the class, which implements the special port with the RTDB message 
 * transport protocol. It uses socket pattern to do this. Is can act as MSPP 
 * (port provider), SPP (server) and SAP (client).
 */
class GCFRTDBPort : public TM::GCFRawPort
{
public:
	// consturctors && destructors
    /// params see constructor of GCFPortInterface    
    GCFRTDBPort (TM::GCFTask& 	task,
				 const string&	name,
				 const string&	DPname);
  
    /// destructor
    virtual ~GCFRTDBPort ();
  
    // open/close methods
    virtual bool open ();
    virtual bool close ();
      
    // send/recv functions
    virtual ssize_t send (GCFEvent& event);
    virtual ssize_t recv (void* buf,
                          size_t count);

	// GCFRTDBPort specific methods    
	void dpCreated 			 (const string& DPname, PVSS::PVSSresult result);
	void dpeSubscribed 		 (const string& DPname, PVSS::PVSSresult result);
	void dpeSubscriptionLost (const string& DPname, PVSS::PVSSresult result);
	void dpeUnsubscribed	 (const string& DPname, PVSS::PVSSresult result);
	void dpeValueChanged	 (const string& DPname, PVSS::PVSSresult result, const PVSS::GCFPValue& value);

private:  
    /// copying is not allowed.
    GCFRTDBPort ();
    GCFRTDBPort (const GCFRTDBPort&);
    GCFRTDBPort& operator= (const GCFRTDBPort&);

	// ----- Data Members -----
	PVSS::PVSSservice*	itsService;
	PVSS::PVSSresponse*	itsResponse;
	string				itsDPname;
	bool				itsIsOpened;
	long				itsOwnID;
};

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
#endif
