//#  GCF_PValue.cc: 
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
#include <Common/LofarLogger.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <Common/StringUtil.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

//
// createMACTypeObject(type)
//
GCFPValue* GCFPValue::createMACTypeObject(TMACValueType type)
{
	GCFPValue* pResult(0);
	switch (type) {
		case LPT_BOOL: 		pResult = new GCFPVBool(); 		break;
		case LPT_CHAR: 		pResult = new GCFPVChar(); 		break;
		case LPT_UNSIGNED:	pResult = new GCFPVUnsigned();	break;
		case LPT_INTEGER:	pResult = new GCFPVInteger();	break;
		case LPT_DOUBLE:	pResult = new GCFPVDouble();	break;
		case LPT_STRING:	pResult = new GCFPVString();	break;
		case LPT_BLOB:		pResult = new GCFPVBlob();		break;      
		case LPT_DATETIME:	pResult = new GCFPVDateTime();	break;
		/* 
		case LPT_REF: 		pResult = new GCFPVRef(); 		break;
		case LPT_BIT32:		pResult = new GCFPVBit32(); 	break;    
		*/
		default:
			bool	isDynArr (type & LPT_DYNARR);
			int		basicType(type & ~LPT_DYNARR);
			if (isDynArr && basicType < END_LPT && basicType != 0) {
				pResult = new GCFPVDynArr((TMACValueType)basicType);
			}
			else {
				LOG_ERROR(LOFAR::formatString ("MACValueType '%d' is unknown or not supported yet.", type));
			}
		break;
	}  

	return (pResult);
}

//
// unpackValue(buffer, &offset)
//
GCFPValue* GCFPValue::unpackValue(const char* valBuf, unsigned int*	offset)
{
	ASSERTSTR(valBuf, "unpackValue called with nullpointer");

	GCFPValue* pValue = createMACTypeObject((TMACValueType) (uchar)(*(valBuf + *offset)));
	if (pValue) {
		unsigned int readLength = pValue->unpack(valBuf + *offset);
		if (readLength != pValue->getSize()) {
			LOG_FATAL_STR("Unpacking object of type " << pValue->getTypeName() << " used " << readLength << " bytes iso " << pValue->getSize());
			delete pValue;
			pValue = 0;
		}
		else {
			(*offset) += readLength;
		}
	}

	return (pValue);
}

//
// unpack(buffer)
//
unsigned int GCFPValue::unpack(const char* valBuf)
{
	ASSERTSTR(_type == (TMACValueType) (uchar) *valBuf, "Buffer contains type " << (TMACValueType) (uchar) *valBuf << " iso " << _type);
	_dataFormat = (LOFAR::DataFormat) valBuf[1];
	// the type was already set, because it was set on construction of this class
	// later maybe also a timestamp can be assigned to a value.
	return (2 + unpackConcrete(valBuf + 2));
}

//
// pack (buffer)
//
unsigned int GCFPValue::pack(char* valBuf) const
{
	valBuf[0] = _type;
	valBuf[1] = _dataFormat;

	// at this moment only the type will be packed, later maybe a timestamp can 
	// be assigned to a value.
	return (2 + packConcrete(valBuf + 2));
}

//
// getTypeName()
//
string GCFPValue::getTypeName() const
{
	string retVal;
	static const char* typeNames[] = {
		"NO_TYPE", 
		"bool", 
		"char", 
		"unsigned", 
		"integer",
		"bit32", 
		"blob",
		"ref", 
		"float", 
		"datetime", 
		"string", 
		"array",    
	};

	if (_type >= LPT_DYNARR) {
		retVal = "dyn_";
	}
	retVal += typeNames[_type & ~LPT_DYNARR];

	return (retVal);
}

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
