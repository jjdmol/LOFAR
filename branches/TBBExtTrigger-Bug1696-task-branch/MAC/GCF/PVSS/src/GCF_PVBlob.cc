//#  GCF_PVBlob.cc: 
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

#include <GCF/PVSS/GCF_PVBlob.h>
#include <Common/DataConvert.h>

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

//
// GCFPVBlob(valbuf, length, clone)
//
GCFPVBlob::GCFPVBlob(unsigned char* val, uint16 length, bool clone) 
  : GCFPValue(LPT_BLOB), _value(0), _length(length), _isDataHolder(clone)
{
	if (clone) {
		itsBuffer.resize(length);
		memcpy(itsBuffer.data(), val, length);
	}
	else {
		_value = val;
	}
}

//
// unpackConcrete(buffer)
//
unsigned int GCFPVBlob::unpackConcrete(const char* valBuf)
{
	unsigned int unpackedBytes(0);
	// first read length field
	memcpy((void *) &_length, valBuf, sizeof(_length));
	unpackedBytes += sizeof(_length);

	if (mustConvert()) {
		LOFAR::dataConvert(LOFAR::dataFormat(), &_length, 1);
	}

	// create new blob buffer space (now it becames a data holder)
	_isDataHolder = true;
	_value = 0;
	itsBuffer.resize(_length);

	// copy the data
	memcpy(itsBuffer.data(), (unsigned char*) valBuf + unpackedBytes, _length);   
	unpackedBytes += _length;

	return (unpackedBytes);
}

//
// packConcrete(buf)
//
unsigned int GCFPVBlob::packConcrete(char* valBuf) const
{
	unsigned int packedBytes(0);

	memcpy(valBuf, (void *) &_length, sizeof(_length)); // packs the length field
	if (_isDataHolder) {
		memcpy(valBuf + sizeof(_length), (void *) itsBuffer.data(), _length); // packs the blob data
	}
	else {
		memcpy(valBuf + sizeof(_length), (void *) _value, _length); // packs the blob data
	}
	packedBytes += sizeof(_length) + _length;

	return packedBytes;
}

//
// addValue(buffer, length)
//
TGCFResult GCFPVBlob::addValue(unsigned char* val2Add, uint16 lenNewVal)
{ 
	if (!val2Add || !lenNewVal) {
		return (GCF_NO_ERROR);
	}

	itsBuffer.resize(_length+lenNewVal);
	if (!_isDataHolder) {		// no dataHolder yet? copy not cloned data first
		memcpy(itsBuffer.data(), (void *) _value, _length); // packs the blob data
		_isDataHolder = true;
		_value = 0;
	}
	memcpy(itsBuffer.data()+_length, val2Add, lenNewVal);   
	_length += lenNewVal; 

	return (GCF_NO_ERROR);
}
 
//
// setValue(buffer, length, clone)
//
TGCFResult GCFPVBlob::setValue(unsigned char* value, uint16 length, bool clone)
{ 
	if (!value || !length) {
		return (GCF_NO_ERROR);
	}

	_isDataHolder = clone;
	_length = length; 
	if (_isDataHolder) {
		itsBuffer.resize(length);
		memcpy(itsBuffer.data(), value, length);
		_value = 0;
	}
	else {
		_value = value;
		itsBuffer.clear();
	}

	return (GCF_NO_ERROR);
}
 
//
// setValue(valueString)
//
TGCFResult GCFPVBlob::setValue(const string& value)
{
	return (setValue((unsigned char*)(value.data()), value.length(), true));
}

//
// getValueAsString
//
string GCFPVBlob::getValueAsString(const string& format) const
{
	return ("<<blobcontent>>");
}

//
// clone()
//
GCFPValue* GCFPVBlob::clone() const
{
	GCFPValue* pNewValue = new GCFPVBlob(_value, _length, true);
	return (pNewValue);
}

//
// copy(other)
//
TGCFResult GCFPVBlob::copy(const GCFPValue& newVal)
{
	if (newVal.getType() != getType()) {
		return (GCF_DIFFERENT_TYPES);
	}

	_isDataHolder = true;
	_length = ((const GCFPVBlob *)&newVal)->getLen();
	itsBuffer.resize(_length);
	memcpy(itsBuffer.data(), ((const GCFPVBlob *)&newVal)->getValue(), _length);
	_value = 0;

	return (GCF_NO_ERROR);
}

//
// operator==(other)
//
bool GCFPVBlob::operator==(const GCFPValue&	that) const
{
	// types must match
	if (that.getType() != getType()) {
		return (false);
	}

	// must have same size.
	if (((GCFPVBlob*)&that)->getLen() != _length) {
		return (false);
	}

	return (memcmp(getValue(), ((GCFPVBlob*)&that)->getValue(), _length) == 0);
}

  } // namespace Common
 } // namespace GCF
} // namespace LOFAR
