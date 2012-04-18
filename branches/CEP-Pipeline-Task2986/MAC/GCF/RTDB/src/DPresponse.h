//#  DPresponse.h:  PVSS response class for catching PVSS events.
//#
//#  Copyright (C) 2007
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
//#

//	Note: This class simply forwards all PVSS-callback routines to the DPservice.
//		  We still need this function because a DPservice IS NOT A PVSSresponse.

#ifndef RTDB_DPRESPONSE_H
#define RTDB_DPRESPONSE_H

#include <GCF/PVSS/PVSSresponse.h>
#include <GCF/RTDB/DPservice.h>

namespace LOFAR {
  namespace GCF {
    class PVSS::GCFPValue;
    using PVSS::GCFPVDynArr;
	using PVSS::PVSSresult;
	using PVSS::PVSSresponse;
    namespace RTDB {
	  class DPservice;

class DPresponse: public PVSSresponse
{
public:
	DPresponse (DPservice*		DPsPtr) : itsDPservice(DPsPtr) {};
	virtual ~DPresponse () {};

	virtual void dpCreated 			 (const string& /*propName*/, PVSSresult	/*result*/) {};
	virtual void dpDeleted	 		 (const string& /*propName*/, PVSSresult	/*result*/) {};
	virtual void dpeSubscribed 		 (const string& /*propName*/, PVSSresult	/*result*/) {};    
	virtual void dpeSubscriptionLost (const string& /*propName*/, PVSSresult	/*result*/) {};
	virtual void dpeUnsubscribed	 (const string& /*propName*/, PVSSresult	/*result*/) {};
	virtual void dpeValueChanged	 (const string& /*propName*/, PVSSresult	/*result*/, 
									  const GCFPValue& /*value*/) {};

	// only those are used.
	virtual void dpeValueGet		 (const string& propName, PVSSresult	result, const GCFPValue& value);
	virtual void dpeValueSet		 (const string& propName, PVSSresult	result);
	virtual void dpQuerySubscribed	 (uint32 queryId, PVSSresult	result);        
	virtual void dpQueryUnsubscribed (uint32 queryId, PVSSresult	result);        
	virtual void dpQueryChanged		 (uint32 queryId, 		 PVSSresult result,
									  const GCFPVDynArr&	DPnames,
									  const GCFPVDynArr&	DPvalues,
									  const GCFPVDynArr&	DPtimes);
private:
	DPservice*	itsDPservice;
};

//# ----- inline functions -----

inline void DPresponse::dpeValueGet(const string& propName, PVSSresult	result, const GCFPValue& value)
{
	itsDPservice->dpeValueGet(propName, result, value);
}

inline void DPresponse::dpeValueSet(const string& propName, PVSSresult	result)
{
	itsDPservice->dpeValueSet(propName, result);
}

inline void DPresponse::dpQuerySubscribed  (uint32 queryID, PVSSresult	result)        
{
	itsDPservice->dpQuerySubscribed(queryID, result);
}

inline void DPresponse::dpQueryUnsubscribed(uint32 queryID, PVSSresult	result)        
{
	itsDPservice->dpQueryUnsubscribed(queryID, result);
}
inline void DPresponse::dpQueryChanged(uint32 queryID, 		 PVSSresult result,
									  const GCFPVDynArr&	DPnames,
									  const GCFPVDynArr&	DPvalues,
									  const GCFPVDynArr&	DPtimes)
{
	itsDPservice->dpQueryChanged(queryID, result, DPnames, DPvalues, DPtimes);
}

    } // namespace RTDB
  } // namespace GCF
} // namespace LOFAR

#endif
