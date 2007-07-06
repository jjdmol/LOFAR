//#  PVSSresponse.h: 
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

#ifndef  PVSS_RESPONSE_H
#define  PVSS_RESPONSE_H

#include <GCF/GCF_PValue.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

// This is an abstract base class, which provides the interface the PVSSservice
// uses for handling the responses from the PVSS database.
// Be means of specilisation of this ABC the user can fill in its own way
// of handling the responses.
class PVSSresponse
{
public:
    PVSSresponse () {};
    virtual ~PVSSresponse () {};

protected:
	friend class PVSSservice;
    virtual void dpCreated 			 (const string& dpName)  = 0;
    virtual void dpDeleted	 		 (const string& dpName)  = 0;
    virtual void dpeSubscribed 		 (const string& dpeName) = 0;    
    virtual void dpeSubscriptionLost (const string& dpeName) = 0;
    virtual void dpeUnsubscribed	 (const string& dpeName) = 0;
    virtual void dpeValueGet		 (const string& dpeName, const Common::GCFPValue& value) = 0;
    virtual void dpeValueChanged	 (const string& dpeName, const Common::GCFPValue& value) = 0;        
    virtual void dpeValueSet		 (const string& dpeName) = 0;
    virtual void dpQuerySubscribed	 (uint32 queryId) 		 = 0;        

private: 
	// data members    
};                                 

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR

#endif
