//#  PortResponse.h:  PVSS response class for catching PVSS events.
//#
//#  Copyright (C) 2007-2011
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
//#  $Id: PortResponse.h 11189 2008-04-29 14:29:41Z overeem $
//#

//	Note: This class simply forwards all PVSS-callback routines to the DPservice.
//		  We still need this function because a DPservice IS NOT A PVSSresponse.

#ifndef RTDB_PORTRESPONSE_H
#define RTDB_PORTRESPONSE_H

#include <GCF/PVSS/PVSSresponse.h>
#include <GCF/PVSS/PVSSresult.h>
#include <GCF/RTDB/GCF_RTDBPort.h>

namespace LOFAR {
  namespace GCF {
    using PVSS::GCFPValue;
    using PVSS::GCFPVDynArr;
	using PVSS::PVSSresult;
	using PVSS::PVSSresponse;
    namespace RTDB {
	  class GCFRTDBPort;

class PortResponse: public PVSSresponse
{
public:
	PortResponse (GCFRTDBPort*		DPsPtr) : itsRTDBPort(DPsPtr) {};
	virtual ~PortResponse () {};

	virtual void dpCreated	 		 (const string& /*DPname*/, PVSSresult /*result*/) {};
	virtual void dpDeleted	 		 (const string& /*DPname*/, PVSSresult /*result*/) {};
	virtual void dpeValueGet		 (const string& /*DPname*/, PVSSresult /*result*/, const GCFPValue& /*value*/) {};
	virtual void dpeValueSet		 (const string& /*DPname*/, PVSSresult /*result*/) {};
	virtual void dpQuerySubscribed	 (uint32 /*queryId*/, PVSSresult /*result*/) {};        
	virtual void dpQueryUnsubscribed (uint32 /*queryId*/, PVSSresult /*result*/) {};        
	virtual void dpQueryChanged		 (uint32 /*queryId*/, PVSSresult /*result*/,
									  const GCFPVDynArr&	/*DPnames*/,
									  const GCFPVDynArr&	/*DPvalues*/,
									  const GCFPVDynArr&	/*DPtimes*/) {};

	// only those are used.
	virtual void dpeSubscribed 		 (const string& DPname, PVSSresult result);
	virtual void dpeSubscriptionLost (const string& DPname, PVSSresult result);
	virtual void dpeUnsubscribed	 (const string& DPname, PVSSresult result);
	virtual void dpeValueChanged	 (const string& DPname, PVSSresult result, const GCFPValue& value);

private:
	GCFRTDBPort*	itsRTDBPort;
};

//# ----- inline functions -----

inline void PortResponse::dpeSubscribed(const string& DPname, PVSSresult	result)        
{
	itsRTDBPort->dpeSubscribed(DPname, result);
}

inline void PortResponse::dpeSubscriptionLost(const string& DPname, PVSSresult	result)        
{
	itsRTDBPort->dpeSubscriptionLost(DPname, result);
}

inline void PortResponse::dpeUnsubscribed(const string& DPname, PVSSresult	result)        
{
	itsRTDBPort->dpeUnsubscribed(DPname, result);
}

inline void PortResponse::dpeValueChanged(const string& DPname, PVSSresult result, const GCFPValue& value)
{
	itsRTDBPort->dpeValueChanged(DPname, result, value);
}

    } // namespace RTDB
  } // namespace GCF
} // namespace LOFAR

#endif
