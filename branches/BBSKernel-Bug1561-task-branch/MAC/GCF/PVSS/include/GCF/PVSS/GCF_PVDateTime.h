//#  GCF_PVDateTime.h: MAC unsigned integer property type
//#
//#  Copyright (C) 2008
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

#ifndef GCF_PVDATETIME_H
#define GCF_PVDATETIME_H

#include <time.h>
#include <GCF/PVSS/GCF_PValue.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace GCF {
	namespace PVSS {


/**
 * By means of this property type a unsigned integer (0 to 4,294,967,295) value 
 * can be used.
 */
class GCFPVDateTime : public GCFPValue
{
public:
	GCFPVDateTime (time_t gmtSec = 0, uint16	milliSec = 0);
	GCFPVDateTime (const GCFPVDateTime& val); 
	virtual ~GCFPVDateTime() {}

	/** Changes the value of this object */
	virtual TGCFResult setValue(double newVal);

	/** 
	* Changes the value of this object by means of a stringbuffer, 
	* which will be translated.
	* @see GCFPValue::setValue(const string value)
	*/
	virtual TGCFResult setValue(const string& value);

	// Returns the value of this object in a string
	virtual string getValueAsString(const string& format = "") const;

	/** Returns the value of this object*/
	virtual double getValue()   const { return (1.0 * itsSeconds + (itsMilli / 1000.0)); }
	time_t		   getSeconds() const { return (itsSeconds); }
	uint16		   getMilli()   const { return (itsMilli); }

	/** @see GCFPValue::clone() */
	virtual GCFPValue* clone() const;

	/** @see GCFPValue::copy() */
	virtual TGCFResult copy(const GCFPValue& value);

	/** @see GCFPValue::operator==() */
	virtual bool operator==(const GCFPValue& that) const;
	virtual bool operator!=(const GCFPValue& that) const { return (!(*this == that)); }

private:
	/// @see GCFPValue::unpack()
	unsigned int unpackConcrete(const char* valBuf);

	/// @see GCFPValue::pack()
	unsigned int packConcrete(char* valBuf) const;

	/// @see GCFPValue::getSize()
	unsigned int getConcreteSize() const { return (sizeof(time_t) + sizeof(uint16)); }

	//# --- DataMembers ---
	time_t	itsSeconds;
	uint16	itsMilli;
};

//# ---------- inline functions ----------
inline bool GCFPVDateTime::operator==(const GCFPValue&	that) const {
	return ((that.getType() == getType()) && 
			(getValue() == ((GCFPVDateTime *) &that)->getValue()));
}

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
#endif
