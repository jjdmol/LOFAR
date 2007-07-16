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
#include <GCF/GCF_PValue.h>
#include <GCF/PVSS/PVSSresponse.h>

namespace LOFAR {
  namespace GCF {
  class Common::GCFPValue;
  using PVSS::PVSSresponse;
  using PVSS::PVSSservice;
  using PVSS::PVSSresult;
  namespace RTDB {
  class RTDBProperty;

typedef enum PSAccessType {
	PS_AT_EXTERNAL,			// owned by other program
	PS_AT_OWNED_PERM,		// my own PS, permanent in database
	PS_AT_OWNED_TEMP		// my own PS, temporarely in database
};

class RTDBPropertySet
{
public:
	// Constructor.
	RTDBPropertySet (const string&	name,
					const string&	type, 
					PSAccessType	accessType,
					PVSSresponse*	pAnswerObj);
	~RTDBPropertySet ();

	string getScope		() const ;
	string getFullScope () const;
	string getType 		() const ;
    bool   exists 		(const string& propName) const;
    

	// Searches the property specified by the propName param
	// @param propName with or without the scope
	// @returns 0 if not in this property set
	RTDBProperty* getProperty (const string& propName) const;

	// Searches the property specified by the propName param
	// @param propName with or without the scope
	// @returns a dummy port if property could not be found
//	GCFProperty& operator[] (const string& propName);

	// Changes the property value, if isMonitoringOn is true and property is 
	// readable the value will be forwared to the DP element in DB
	// @param propName with or without the scope
	// @param value can be of type GCFPValue or string (will be converted to 
	//              GCFPValue content)
	// @param wantAnswer setting this parameter to 'true' forces GCF to give an
	//                   answer on this setValue operation. This is very useful
	//                   in case the value must be set on a property of a remote system.
	// @returns GCF_PROP_NOT_IN_SET,  GCF_PROP_WRONG_TYPE, GCF_PROP_NOT_VALID
	// <group>
	PVSSresult setValue (const string& 				propName, 
						 const Common::GCFPValue&	value, 
						 bool  						wantAnswer = false);

	PVSSresult setValue (const string& 	propName,
						 const string& 	value, 
						 bool  			wantAnswer = false);

	PVSSresult setValueTimed (const string& 			propName, 
							  const Common::GCFPValue&	value, 
							  double 					timestamp,
							  bool 						wantAnswer = false);

	PVSSresult setValueTimed (const string& 	propName,
							  const string&		value, 
							  double 			timestamp,
							  bool 				wantAnswer = false);
    // </group>

private:
	RTDBPropertySet();
	// Don't allow copying this object.
	// <group>
	RTDBPropertySet (const RTDBPropertySet&);
	RTDBPropertySet& operator= (const RTDBPropertySet&);
	// </group>

	// helper methods
    void _createAllProperties();
    void _deleteAllProperties();
    void _cutScope	 (string& propName) const;
    void _addProperty(const string& propName, RTDBProperty& prop);

	// data members
    string              		itsScope;	// with or without DBname:
    string              		itsType;
	PSAccessType				itsAccessType;
	PVSSservice*          		itsService;
	PVSSresponse*          		itsResponse;
	bool						itsIsTemp;
	// map with pointers to Property objects
    typedef map<string /*propName*/, RTDBProperty*> PropertyMap_t;
    PropertyMap_t       		itsPropMap;
	// list of property info
    typedef list<Common::TPropertyInfo> PropInfoList_t;
    PropInfoList_t       		itsPropInfoList;
};

//# ----- inline functions -----

inline PVSSresult RTDBPropertySet::setValue (const string& propName, const string& value, bool wantAnswer)
{
  return (setValueTimed(propName, value, 0.0, wantAnswer));
}

inline PVSSresult RTDBPropertySet::setValue (const string& propName, const Common::GCFPValue& value, bool wantAnswer)
{
  return (setValueTimed(propName, value, 0.0, wantAnswer));
}
  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
#endif
