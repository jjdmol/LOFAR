//#  GCF_PVDateTime.cc: 
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


#include <lofar_config.h>

#include <GCF/PVSS/GCF_PVDateTime.h>
#include <Common/DataConvert.h>
#include <Common/StringUtil.h>

namespace LOFAR {
  namespace GCF {
	namespace PVSS {

//
// GCFPVDateTime(sec, milli)
//
GCFPVDateTime::GCFPVDateTime(time_t		gmtSec, uint16		milli) :
	GCFPValue (LPT_DATETIME),
	itsSeconds(gmtSec),
	itsMilli  (milli)
{}

//
// copy constructor
//
GCFPVDateTime::GCFPVDateTime(const GCFPVDateTime&	that) :
	GCFPValue (LPT_DATETIME),
	itsSeconds(that.itsSeconds),
	itsMilli  (that.itsMilli)
{ }

//
// unpackConcrete(buf)
//
unsigned int GCFPVDateTime::unpackConcrete(const char* valBuf)
{
	memcpy((void*) &itsSeconds, valBuf, sizeof(time_t));
	memcpy((void*) &itsMilli,   valBuf +sizeof(time_t), sizeof(uint16));
	if (mustConvert()) {
		LOFAR::dataConvert(LOFAR::dataFormat(), (uint32*)&itsSeconds, 1);
		LOFAR::dataConvert(LOFAR::dataFormat(), &itsMilli,   1);
	}
	return (getConcreteSize());
}

//
// packConcrete(buf)
//
unsigned int GCFPVDateTime::packConcrete(char* valBuf) const
{
	memcpy(valBuf, 				  (void*) &itsSeconds, sizeof(time_t));
	memcpy(valBuf+sizeof(time_t), (void*) &itsMilli,   sizeof(uint16));
	return (getConcreteSize());
}

//
// setValue(double)
//
TGCFResult GCFPVDateTime::setValue(double	newVal)
{
	if (newVal < 0.0) {
		return (GCF_VALUESTRING_NOT_VALID);
	}

	itsSeconds = (time_t)newVal;
	itsMilli   = (uint16) ((1.0 * itsSeconds - newVal) * 1000.0);
	return (GCF_NO_ERROR);
}


//
// setValue(string)
//
TGCFResult GCFPVDateTime::setValue(const string& valueString)
{
	if (valueString.empty()) {
		return (GCF_VALUESTRING_NOT_VALID);
	}

	// try to match to a date, at least y.m.d is needed.
	struct tm	T = { 0,0,0,0,0,0,0,0,0,0,0};
	itsMilli = 0;
	uint32		theMilli;
	if (sscanf(valueString.c_str(), "%4d[./-]%2d[./-]%2d %2d:%2d:%2d.%3d", 
		&T.tm_year, &T.tm_mon, &T.tm_mday, &T.tm_hour, &T.tm_min, &T.tm_sec, &theMilli) < 3) {
		return (GCF_VALUESTRING_NOT_VALID);
	}
	itsSeconds = timegm(&T);
	itsMilli   = theMilli % 1000;
	return(GCF_NO_ERROR);
}

//
// gtValueAsString(format)
//
string GCFPVDateTime::getValueAsString(const string& format) const
{
	string	theFormat(format);
	if (theFormat.empty()) {
		theFormat="%F %T";
	}
	return (timeString(itsSeconds, true, (char*)theFormat.c_str()) 
			+ formatString(".%03d", itsMilli));
}

//
// close()
//
GCFPValue* GCFPVDateTime::clone() const
{
	return (new GCFPVDateTime(itsSeconds, itsMilli));
}

//
// copy(newVal)
//
TGCFResult GCFPVDateTime::copy(const GCFPValue& newVal)
{
	if (newVal.getType() != getType()) {
		return (GCF_DIFFERENT_TYPES);
	}

	itsSeconds = ((GCFPVDateTime *)&newVal)->getSeconds();
	itsMilli   = ((GCFPVDateTime *)&newVal)->getMilli();
	return (GCF_NO_ERROR);
}

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR
