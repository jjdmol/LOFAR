//#  PropSetResponse.h:  PVSS response class for catching PVSS events.
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

//	Note: This class simply forwards all PVSS-callback routines to the PropertySet.
//		  We still need this function because a PropertySet IS NOT A PVSSresponse.

#ifndef RTDB_PROPSETRESPONSE_H
#define RTDB_PROPSETRESPONSE_H

#include <GCF/PVSS/PVSSresponse.h>
#include "DPanswer.h"

namespace LOFAR {
  namespace GCF {
    class PVSS::GCFPValue;
	using PVSS::GCFPVDynArr;
	using PVSS::PVSSresult;
	using PVSS::PVSSresponse;
    namespace RTDB {
	  class RTDBPropertySet;

class PropSetResponse: public PVSSresponse
{
public:
	PropSetResponse (RTDBPropertySet*		propSetPtr) : itsPropertySet(propSetPtr) {};
	virtual ~PropSetResponse () {};

	virtual void dpCreated 			 (const string& propName, PVSSresult	result);
	virtual void dpDeleted	 		 (const string& propName, PVSSresult	result);
	virtual void dpeSubscribed 		 (const string& propName, PVSSresult	result);    
	virtual void dpeSubscriptionLost (const string& propName, PVSSresult	result);
	virtual void dpeUnsubscribed	 (const string& propName, PVSSresult	result);
	virtual void dpeValueGet		 (const string& propName, PVSSresult	result, const GCFPValue& value);
	virtual void dpeValueChanged	 (const string& propName, PVSSresult	result, const GCFPValue& value);
	virtual void dpeValueSet		 (const string& propName, PVSSresult	result);

	// Next functions are not used.
	virtual void dpQuerySubscribed	 (uint32 /*queryId*/, PVSSresult	/*result*/) {};
	virtual void dpQueryUnsubscribed (uint32 /*queryId*/, PVSSresult	/*result*/) {};
	virtual void dpQueryChanged		 (uint32 /*queryId*/, PVSSresult 	/*result*/,
									  const GCFPVDynArr&	/*DPnames*/,
									  const GCFPVDynArr&	/*DPvalues*/,
									  const GCFPVDynArr&	/*DPtimes*/) {};

private:
	RTDBPropertySet*	itsPropertySet;
};

//# ----- inline functions -----
inline void PropSetResponse::dpCreated(const string& propName, PVSSresult	result)
{
	itsPropertySet->dpCreated(propName, result);
}

inline void PropSetResponse::dpDeleted(const string& propName, PVSSresult	result)
{
	itsPropertySet->dpDeleted(propName, result);
}
inline void PropSetResponse::dpeSubscribed(const string& propName, PVSSresult	result)
{
	itsPropertySet->dpeSubscribed(propName.empty() ? "value" : propName, result);
}

inline void PropSetResponse::dpeUnsubscribed(const string& propName, PVSSresult	result)
{
	itsPropertySet->dpeUnsubscribed(propName.empty() ? "value" : propName, result);
}

inline void PropSetResponse::dpeValueGet(const string& propName, PVSSresult	result, const GCFPValue& value)
{
	itsPropertySet->dpeValueGet(propName.empty() ? "value" : propName, result, value);
}

inline void PropSetResponse::dpeValueChanged(const string& propName, PVSSresult	result, const GCFPValue& value)
{
	itsPropertySet->dpeValueChanged(propName.empty() ? "value" : propName, result, value);
}

inline void PropSetResponse::dpeValueSet(const string& propName, PVSSresult	result)
{
	itsPropertySet->dpeValueSet(propName.empty() ? "value" : propName, result);
}

inline void PropSetResponse::dpeSubscriptionLost(const string& propName, PVSSresult	result)
{
	itsPropertySet->dpeSubscriptionLost(propName.empty() ? "value" : propName, result);
}

    } // namespace RTDB
  } // namespace GCF
} // namespace LOFAR

#endif
