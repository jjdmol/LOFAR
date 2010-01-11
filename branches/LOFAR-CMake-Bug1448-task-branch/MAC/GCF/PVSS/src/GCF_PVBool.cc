//#  GCF_PVBool.cc: 
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


#include <lofar_config.h>

#include <GCF/PVSS/GCF_PVBool.h>
#include <Common/StringUtil.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

//
// unpackConcrete(buffer)
//
unsigned int GCFPVBool::unpackConcrete(const char* valBuf)
{
	_value = (valBuf[0] == 1 ? true : false);
	return 1;
}

//
// packConcrete(buffer)
//
unsigned int GCFPVBool::packConcrete(char* valBuf) const
{
	valBuf[0] = (_value ? 1 : 0);
	return 1;
}

//
// setValue(valueString)
//
TGCFResult GCFPVBool::setValue(const string& valueData)
{
	// string must contain data.
	if (valueData.empty()) {
		return (GCF_VALUESTRING_NOT_VALID);
	}

	// does string contain a number?
	char* 		validPos(0);
	long int 	value(strtol(valueData.c_str(), &validPos, 10));
	if (*validPos == '\0') {	// reached EOstring?
		_value = (value != 0);
		return (GCF_NO_ERROR);
	}

	if ((strncasecmp(valueData.c_str(), "false", 5) == 0) || 
		(strncasecmp(valueData.c_str(), "no", 2) == 0) ||
		(strncasecmp(valueData.c_str(), "off", 3) == 0)) {
		_value = false; 
		return (GCF_NO_ERROR);
	}

	if ((strncasecmp(valueData.c_str(), "true", 5) == 0) || 
		(strncasecmp(valueData.c_str(), "yes", 2) == 0) ||
		(strncasecmp(valueData.c_str(), "on", 3) == 0)) {
		_value = true;
		return (GCF_NO_ERROR);
	}

	return (GCF_VALUESTRING_NOT_VALID);
}

//
// getValueAsString(format)
//
string GCFPVBool::getValueAsString(const string& format) const
{
	string retVal;

	if (format.length() == 0) {
		retVal = (_value ? "1" : "0");
	}
	else if (format == "true/false") {
		retVal = (_value ? "true" : "false");
	}
	else if (format == "on/off") {
		retVal = (_value ? "on" : "off");
	}
	else if (format == "yes/no") {
		retVal = (_value ? "yes" : "no");
	}
	else {
		retVal = formatString(format.c_str(), _value);
	}

	return (retVal);
}

//
// clone()
//
GCFPValue* GCFPVBool::clone() const
{
	GCFPValue* pNewValue = new GCFPVBool(_value);

	return (pNewValue);
}

//
// copy(other)
//
TGCFResult GCFPVBool::copy(const GCFPValue& newVal)
{
	if (newVal.getType() != getType()) {
		return (GCF_DIFFERENT_TYPES);
	}

	_value = ((GCFPVBool *)&newVal)->getValue();
	return (GCF_NO_ERROR);
}

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
