//#  GCF_PVDynArr.cc: 
//#
//#  Copyright (C) 2002-2008
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

#include <GCF/PVSS/GCF_PVDynArr.h>
#include <Common/DataConvert.h>
#include <Common/LofarTypes.h>

using LOFAR::TYPES::uint16;

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

//
// GCFPVDynArr(type, value)
//
GCFPVDynArr::GCFPVDynArr(TMACValueType itemType, const GCFPValueArray& val) :
	GCFPValue((TMACValueType) (LPT_DYNARR | itemType))
{
	ASSERT(itemType != LPT_DYNARR);
	setValue(val);
}

//
// GCFPVDynArr(type)
//
GCFPVDynArr::GCFPVDynArr(TMACValueType itemType) :
	GCFPValue((TMACValueType) (LPT_DYNARR | itemType))
{
	ASSERT(itemType != LPT_DYNARR);
}

//
// ~GCFPVDynArr
//
GCFPVDynArr::~GCFPVDynArr()
{
	cleanup();
}

//
// unpackConcrete(buffer)
//
unsigned int GCFPVDynArr::unpackConcrete(const char* valBuf)
{
	unsigned int curUnpackedBytes(0);
	unsigned int unpackedBytes(0);
	uint16 		 arraySize(0);
	GCFPValue* 	 pNewValue(0);

	cleanup();	// delete any old values.

	// start buffer with size-field
	memcpy((void *) &arraySize, valBuf, sizeof(uint16));	
	unpackedBytes += sizeof(uint16);

	if (mustConvert()) {
		LOFAR::dataConvert(LOFAR::dataFormat(), &arraySize, 1);
	}

	for (unsigned int i = 0; i < arraySize; i++) {
		pNewValue = GCFPValue::createMACTypeObject(
								(TMACValueType) (getType() & ~LPT_DYNARR));
		pNewValue->setDataFormat(_dataFormat);

		curUnpackedBytes = pNewValue->unpackConcrete(valBuf + unpackedBytes);
		if (curUnpackedBytes == 0) {	// something went wrong
			return (0);
		}
		unpackedBytes += curUnpackedBytes;
		_values.push_back(pNewValue);
	}
	return (unpackedBytes);
}

//
// packConcrete(buf)
//
unsigned int GCFPVDynArr::packConcrete(char* valBuf) const
{
	unsigned int packedBytes(0);  
	unsigned int curPackedBytes(0);
	uint16 		 arraySize(_values.size());

	memcpy(valBuf, (void *) &arraySize, sizeof(uint16));
	packedBytes += sizeof(uint16);

	GCFPValueArray::const_iterator iter = _values.begin();
	GCFPValueArray::const_iterator end  = _values.end();
	while (iter != end) {
		curPackedBytes = (*iter)->packConcrete(valBuf + packedBytes);
		if (curPackedBytes == 0) {	// something went wrong
			return(0);
		}        
		packedBytes += curPackedBytes;
		++iter;
	}  
	return (packedBytes);
}

//
// getConcreteSize()
//
unsigned int GCFPVDynArr::getConcreteSize() const
{
	unsigned int totalSize(sizeof(uint16));

	GCFPValueArray::const_iterator iter = _values.begin();
	GCFPValueArray::const_iterator end  = _values.end();
	while (iter != end) {
		totalSize += (*iter)->getConcreteSize();
		++iter;
	}  

	return (totalSize);
}

//
// setvalue(string)
//
TGCFResult GCFPVDynArr::setValue(const string& value)
{
	LOG_ERROR("setValue(string) is not possible on a dynArray!");
	return (GCF_DIFFERENT_TYPES);
}

//
// setValue(array)
//
void GCFPVDynArr::setValue(const GCFPValueArray& newVal)
{
	cleanup();

	GCFPValueArray::const_iterator iter = newVal.begin();
	GCFPValueArray::const_iterator end  = newVal.end();

	while (iter != end) {
		if ((*iter)->getType() == (getType() & ~LPT_DYNARR)) {
			_values.push_back((*iter)->clone());
		}
		++iter;
	}
}

//
// getValueAsString(format)
//
string GCFPVDynArr::getValueAsString(const string& format) const
{
	return ("Not yet implemented!");
}

//
// clone()
//
GCFPValue* GCFPVDynArr::clone() const
{
	return (new GCFPVDynArr(getType(), _values));
}

//
// copy(value)
//
TGCFResult GCFPVDynArr::copy(const GCFPValue& newVal)
{
	if (newVal.getType() != getType()) {
		return(GCF_DIFFERENT_TYPES);
	}

	setValue(((GCFPVDynArr*)&newVal)->getValue());
	return (GCF_NO_ERROR);
}

//
// cleanup()
//
void GCFPVDynArr::cleanup()
{
	// first delete the values pointed at.
	for (GCFPValueArray::iterator iter = _values.begin();
		iter != _values.end(); ++iter) {
		delete *iter;  
	}
	_values.clear();	// clear pointer vector.
}
  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
