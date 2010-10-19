//#  DPanswer.h:  PVSS response class that delivers DP_protocol messages.
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

#ifndef RTDB_DP_ANSWER_H
#define RTDB_DP_ANSWER_H

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/PVSS/PVSSresponse.h>
#include <GCF/PVSS/GCF_PVDynArr.h>

namespace LOFAR {
  namespace GCF {
    using PVSS::GCFPValue;
    using PVSS::GCFPVDynArr;
	using PVSS::PVSSresponse;
	using PVSS::PVSSresult;
    namespace RTDB {

class DPanswer: public PVSSresponse
{
public:
	DPanswer (TM::GCFTask*	aTask) : itsTask(aTask) {};
	virtual ~DPanswer () {};

	virtual void dpCreated 			 (const string& dpName,  PVSSresult result);
	virtual void dpDeleted	 		 (const string& dpName,  PVSSresult result);
	virtual void dpeSubscribed 		 (const string& dpeName, PVSSresult result);
	virtual void dpeSubscriptionLost (const string& dpeName, PVSSresult result);
	virtual void dpeUnsubscribed	 (const string& dpeName, PVSSresult result);
	virtual void dpeValueGet		 (const string& dpeName, PVSSresult result, 
									  const GCFPValue& value);
	virtual void dpeValueChanged	 (const string& dpeName, PVSSresult result, 
									  const GCFPValue& value);
	virtual void dpeValueSet		 (const string& dpeName, PVSSresult result);
	virtual void dpQuerySubscribed	 (uint32 queryId, 		 PVSSresult result);
	virtual void dpQueryUnsubscribed (uint32 queryId, 		 PVSSresult result);
	virtual void dpQueryChanged		 (uint32 queryId, 		 PVSSresult result,
									  const GCFPVDynArr&	DPnames,
									  const GCFPVDynArr&	DPvalues,
									  const GCFPVDynArr&	DPtimes);
    
private:
	// Don't allow copying this object.
	// <group>
	DPanswer (const DPanswer&);
	DPanswer& operator= (const DPanswer&);  
	// </group>

	void _dispatchEvent(MACIO::GCFEvent&	event);

	// ----- datamembers -----
	TM::GCFTask*				itsTask;
	static	TM::GCFDummyPort	gDummyPort;
};

    } // namespace RTDB
  } // namespace GCF
} // namespace LOFAR

#endif
