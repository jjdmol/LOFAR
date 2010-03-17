//#  RTDB_PropertySet.h:  
//#
//#  Copyright (C) 2002-2007
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

#ifndef RTDB_PROPERTYSET_H
#define RTDB_PROPERTYSET_H

#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <Common/lofar_string.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/PVSS/GCF_PValue.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/PVSSresponse.h>

namespace LOFAR {
  namespace GCF {
	using PVSS::GCFPValue;
	using PVSS::GCFPVDynArr;
	using PVSS::PVSSresponse;
	using PVSS::PVSSservice;
	using PVSS::PVSSresult;
	namespace RTDB {
	  class PropSetResponse;
	  class DPanswer;

#define PSAT_RD_MASK	0x0001
#define PSAT_WR_MASK	0x0002
#define PSAT_TMP_MASK	0x0004
#define PSAT_CW_MASK	0x0008

enum {
	PSAT_RO = 1,		// read-only
	PSAT_WO,			// write-only
	PSAT_RW,			// read-write
	PSAT_TMP,			// PS wil be created and delete on the fly.
	PSAT_CW = 8			// values are only written when they are changed.
};

class RTDBPropertySet
{
public:
	// Constructor.
	RTDBPropertySet (const string&	name,
					 const string&	type, 
					 uint32			accessType,
					 TM::GCFTask*	clientTask);	// must be pointer!
	~RTDBPropertySet ();

	string getScope		() const;
	string getFullScope () const;
	string getType 		() const;
    
	// @param propName with or without the scope
	// @param value can be of type GCFPValue or string (will be converted to 
	//              GCFPValue content)
	// @param wantAnswer setting this parameter to 'true' forces GCF to give an
	//                   answer on this setValue operation. This is very useful
	//                   in case the value must be set on a property of a remote system.
	// @returns GCF_PROP_NOT_IN_SET,  GCF_PROP_WRONG_TYPE, GCF_PROP_NOT_VALID
	// <group>
	PVSSresult setValue (const string&		propName, 
						 const GCFPValue&	value, 
						 double 			timestamp = 0.0,
						 bool				immediately = true);

	PVSSresult setValue (const string& 		propName,
						 const string&		value, 
						 double 			timestamp = 0.0,
						 bool				immediately = true);
    // </group>

	PVSSresult getValue(const string&		propName,
						GCFPValue&			returnVar);

	PVSSresult flush();

	void setSubscription(bool	on);
	void setConfirmation(bool	on);

protected:
	friend class PropSetResponse;
	void dpCreated 			 (const string& propName, PVSSresult	result);
	void dpDeleted	 		 (const string& propName, PVSSresult	result);
	void dpeSubscribed 		 (const string& propName, PVSSresult	result);    
	void dpeSubscriptionLost (const string& propName, PVSSresult	result);
	void dpeUnsubscribed	 (const string& propName, PVSSresult	result);
	void dpeValueGet		 (const string& propName, PVSSresult	result, const GCFPValue& value);
	void dpeValueChanged	 (const string& propName, PVSSresult	result, const GCFPValue& value);
	void dpeValueSet		 (const string& propName, PVSSresult	result);
	void dpQuerySubscribed	 (uint32 queryId, PVSSresult	result);        
	void dpQueryChanged		 (uint32 queryId, 		 PVSSresult result,
							  const GCFPVDynArr&	DPnames,
							  const GCFPVDynArr&	DPvalues,
							  const GCFPVDynArr&	DPtypes);

private:
	RTDBPropertySet();
	// Don't allow copying this object.
	// <group>
	RTDBPropertySet (const RTDBPropertySet&);
	RTDBPropertySet& operator= (const RTDBPropertySet&);
	// </group>

	// define a struct for handling the DPE's.
	typedef struct Property_t {
		GCFPValue*		value;			// container for the actual value.
		bool			dirty;			// nedd to be updated to the database.
		bool			initialized;	// initial value is get from data.
		bool			isBasicType;	// PS is not a structure but a basictype

		Property_t(GCFPValue*	aValPtr, bool	basicType = false) : 
			value(aValPtr), dirty(false), initialized(false), isBasicType(basicType) {};
		~Property_t() { delete value; };
	} Property;

	// helper methods
    void _createAllProperties();
    void _deleteAllProperties();
    void _cutScope	 (string& propName) const;
    void _addProperty(const string& propName, Property* prop);
	Property* _getProperty (const string& propName) const;


	// data members
    string              		itsScope;			// with or without DBname:
    string              		itsType;			// PVSS typename
	uint32						itsAccessType;		// READ/WRITE/TEMP/CondWrite
	PVSSservice*          		itsService;			// connection to database
	PVSSresponse*          		itsOwnResponse;		// callback to myself
	DPanswer*	          		itsExtResponse;		// callback to client
	bool						itsExtSubscription;	// client has subscription
	bool						itsExtConfirmation;	// client want confirmation

	// map with pointers to Property objects
    typedef map<string /*propName*/, Property*> PropertyMap_t;
    PropertyMap_t       		itsPropMap;
};

//# ----- inline functions -----

// setSubscription(on)
inline	void RTDBPropertySet::setSubscription(bool	on)
{
	itsExtSubscription = on;
}

// setConfirmation(on)
inline	void RTDBPropertySet::setConfirmation(bool	on)
{
	itsExtConfirmation = on;
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
#endif
