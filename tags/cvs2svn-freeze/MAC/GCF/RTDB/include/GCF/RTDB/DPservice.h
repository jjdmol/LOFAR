//#  DPservice.h: class for accessing a datapoinst directly (w/o a PropertySet).
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

// \file
// RealTime database service for accessing DataPoints without a PropertySet.
//
// Sometimes you need access to DataPointElements of several PropertySets. In stead
// of creating all the PropertySet objects it is far more easy to use the DPservice.
// The DPservice acts as a gateway to the database with almost no use of memory which
// is very different from PropertySets that make a local copy of the DataPoints in
// memory.
// The use of DPservice is limited to getting and setting values. It is not possible 
// to take a subscription on DataPoints which IS possible with PropertySets.

#ifndef RTDB_DPSERVICE_H
#define RTDB_DPSERVICE_H

#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_Defines.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/PVSS/PVSSresponse.h>

namespace LOFAR {
  namespace GCF {
	using Common::GCFPValue;
	using Common::TMACValueType;
	using PVSS::PVSSresponse;
	using PVSS::PVSSservice;
	using PVSS::PVSSresult;
	namespace RTDB {
	  class DPResponse;
	  class DPanswer;


class DPservice
{
public:
	// A DPservice object is bound to a GCFtask because it must be able to deliver the
	// DP protocol events to the task. One DPservice per task is normally enough. With the
	// reportBack parameter use can set wether or not you like to receive the DP events.
	explicit DPservice (TM::GCFTask*	clientTask, // must be a pointer
						bool			reportBack = true);
	~DPservice ();

	// There are two ways of setting a value, they differ in ease of use and performance.
	//
	// @param DPname with full scope.
	// @param value can be of type GCFPValue or string. The string parameter will be 
	//		  converted to a GCFPValue which takes time but is easier in use.
	// @param type of the GCFPValue object that must be created.
	// @param timestamp Optional parameter for attaching a timestamp to the value change.
	// @returns PVSS_xxx result codes.
	// <group>
	PVSSresult setValue (const string&		DPname, 
						 const GCFPValue&	value, 
						 double 			timestamp = 0.0);

	PVSSresult setValue	(const string& 		DPname,	// slower than previous function
						 const string&		value, 
						 TMACValueType		type,	// LPT_BOOL/CHAR/UNSIGNED/....
						 double 			timestamp = 0.0);
    // </group>

	// Place a request for getting the value from a DataPoint from the database.
	// The function will return directly and the result will be delivered in a
	// DPGetEvent sometime later.
	PVSSresult getValue(const string&		DPname);

protected:
	friend class DPresponse;
	void dpCreated 			 (const string& DPname, PVSSresult	result);
	void dpDeleted	 		 (const string& DPname, PVSSresult	result);
	void dpeSubscribed 		 (const string& DPname, PVSSresult	result);    
	void dpeSubscriptionLost (const string& DPname, PVSSresult	result);
	void dpeUnsubscribed	 (const string& DPname, PVSSresult	result);
	void dpeValueGet		 (const string& DPname, PVSSresult	result, const Common::GCFPValue& value);
	void dpeValueChanged	 (const string& DPname, PVSSresult	result, const Common::GCFPValue& value);
	void dpeValueSet		 (const string& DPname, PVSSresult	result);
	void dpQuerySubscribed	 (uint32 queryId, PVSSresult	result);        

private:
	DPservice();
	// Don't allow copying this object.
	// <group>
	DPservice (const DPservice&);
	DPservice& operator= (const DPservice&);
	// </group>

	// data members
	PVSSservice*          		itsService;			// connection to database
	PVSSresponse*          		itsOwnResponse;		// callback to myself
	DPanswer*	          		itsExtResponse;		// callback to client
	bool						itsPassResult;		// client wants result-events
};

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
#endif
