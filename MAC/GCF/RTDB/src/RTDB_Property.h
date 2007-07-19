//#  RTDB_Property.h:  
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

#ifndef RTDB_PROPERTY_H
#define RTDB_PROPERTY_H

#include <GCF/GCF_Defines.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/PVSSresponse.h>

namespace LOFAR {
  namespace GCF {
    using Common::GCFPValue;
	using Common::TPropertyInfo;
	using PVSS::PVSSresult;
	using PVSS::PVSSservice;
	using PVSS::PVSSresponse;
    namespace RTDB {
	  class RTDBPropertySet;

class RTDBProperty
{
public:
	// Constructor.
    RTDBProperty (const string&				scope,
				  const TPropertyInfo&		propInfo,
				  PVSSresponse*				responsePtr);
    virtual ~RTDBProperty ();

	const string getName () const 
		{ return (itsPropInfo.propName); }
      
    // Asynchronous action
    // Performs a get operation on the SCADA DB
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    GCFPValue& getValue();
      
    // (A)Synchronous (!) action
    // Performs a set operation on the SCADA DB
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    PVSSresult setValue(const GCFPValue& value, bool wantAnswer = false);
      
    // (A)Synchronous (!) action
    // Performs a set operation on the SCADA DB
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    PVSSresult setValue(const string& value, bool wantAnswer = false);

    // (A)Synchronous (!) action
    // Performs a set operation on the SCADA DB with a timestamp
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    PVSSresult setValueTimed(const GCFPValue& value, 
							 double timestamp, 
							 bool wantAnswer = false);
      
    // (A)Synchronous (!) action
    // Performs a set operation on the SCADA DB with a timestamp
    // @return can be GCF_NO_ERROR or GCF_PML_ERROR
    PVSSresult setValueTimed(const string& value, 
    						 double timestamp, 
							 bool wantAnswer = false);
protected:
	friend class PropResponse;
	void setSubscription(bool	on) {};
	void valueSetAck	(PVSSresult	result);
	void valueGetAck	(PVSSresult	result, const GCFPValue&	value);
	void valueChangedAck(PVSSresult	result, const GCFPValue&	value);

private:
	RTDBProperty();
	// Don't allow copying this object.
	// <group>
	RTDBProperty (const RTDBProperty&);
	RTDBProperty& operator= (const RTDBProperty&);
	// </group>

	// data members
	TPropertyInfo		itsPropInfo;
	GCFPValue*			itsCurValue;	// ???
	GCFPValue*			itsOldValue;	// ???
	PVSSservice*		itsService;			// connection to database.
	PVSSresponse*		itsOwnResponse;		// catching internal events
	PVSSresponse*		itsExtResponse;		// dispatching to client
};

inline PVSSresult RTDBProperty::setValue (const string& value, bool wantAnswer)
{
  return setValueTimed(value, 0.0, wantAnswer);
}

inline PVSSresult RTDBProperty::setValue (const GCFPValue& value, bool wantAnswer)
{
  return setValueTimed(value, 0.0, wantAnswer);
}
  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR
#endif
