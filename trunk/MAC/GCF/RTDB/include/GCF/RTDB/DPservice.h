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
	// Constructor.
	explicit DPservice (TM::GCFTask*	clientTask, // must be a pointer
						bool			reportBack = true);
	~DPservice ();

	// @param propName with or without the scope
	// @param value can be of type GCFPValue or string (will be converted to 
	//              GCFPValue content)
	// @param wantAnswer setting this parameter to 'true' forces GCF to give an
	//                   answer on this setValue operation. This is very useful
	//                   in case the value must be set on a property of a remote system.
	// @returns GCF_PROP_NOT_IN_SET,  GCF_PROP_WRONG_TYPE, GCF_PROP_NOT_VALID
	// <group>
	PVSSresult setValue (const string&		propName, 
						 const GCFPValue&	value);

	PVSSresult setValue (const string& 		propName,	// slower than previous function
						 const string& 		value,
						 TMACValueType		type);	// LPT_BOOL/CHAR/UNSIGNED/....

	PVSSresult setValueTimed (const string&		propName, 
							  const GCFPValue&	value, 
							  double 			timestamp);

	PVSSresult setValueTimed (const string& 	propName,	// slower than previous function
							  const string&		value, 
							  TMACValueType		type,	// LPT_BOOL/CHAR/UNSIGNED/....
							  double 			timestamp);
    // </group>

	PVSSresult getValue(const string&		propName);

protected:
	friend class DPresponse;
	void dpCreated 			 (const string& propName, PVSSresult	result);
	void dpDeleted	 		 (const string& propName, PVSSresult	result);
	void dpeSubscribed 		 (const string& propName, PVSSresult	result);    
	void dpeSubscriptionLost (const string& propName, PVSSresult	result);
	void dpeUnsubscribed	 (const string& propName, PVSSresult	result);
	void dpeValueGet		 (const string& propName, PVSSresult	result, const Common::GCFPValue& value);
	void dpeValueChanged	 (const string& propName, PVSSresult	result, const Common::GCFPValue& value);
	void dpeValueSet		 (const string& propName, PVSSresult	result);
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

//# ----- inline functions -----

// setValue(propname, valueString immediately)
inline PVSSresult DPservice::setValue (const string&	propName, 
									   const string&	value,
									   TMACValueType	type)
{
  return (setValueTimed(propName, value, type, 0.0));
}

// setValue(propname, value&, immediately)
inline PVSSresult DPservice::setValue (const string&			propName, 
									   const Common::GCFPValue&	value)
{
  return (setValueTimed(propName, value, 0.0));
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
#endif
