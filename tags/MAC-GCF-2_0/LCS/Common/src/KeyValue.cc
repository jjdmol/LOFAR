// KeyValue.cc: Class to hold a general value
//
//  Copyright (C) 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <Common/KeyValue.h>
#include <Common/KeyValueMap.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <stdexcept>

namespace LOFAR {

KeyValue::KeyValue()
: itsDataType (DTValueVector),
  itsExtDT    (DTValueVector),
  itsValuePtr (new vector<KeyValue>())
{}

KeyValue::KeyValue (bool value)
: itsDataType (DTBool),
  itsExtDT    (DTBool),
  itsValuePtr (new bool(value))
{}

KeyValue::KeyValue (int32 value)
: itsDataType (DTInt),
  itsExtDT    (DTInt),
  itsValuePtr (new int32(value))
{}

KeyValue::KeyValue (float value)
: itsDataType (DTFloat),
  itsExtDT    (DTFloat),
  itsValuePtr (new float(value))
{}

KeyValue::KeyValue (double value)
: itsDataType (DTDouble),
  itsExtDT    (DTDouble),
  itsValuePtr (new double(value))
{}

KeyValue::KeyValue (const fcomplex& value)
: itsDataType (DTFComplex),
  itsExtDT    (DTFComplex),
  itsValuePtr (new fcomplex(value))
{}

KeyValue::KeyValue (const dcomplex& value)
: itsDataType (DTDComplex),
  itsExtDT    (DTDComplex),
  itsValuePtr (new dcomplex(value))
{}

KeyValue::KeyValue (const char* value)
: itsDataType (DTString),
  itsExtDT    (DTString),
  itsValuePtr (new string(value))
{}

KeyValue::KeyValue (const string& value)
: itsDataType (DTString),
  itsExtDT    (DTString),
  itsValuePtr (new string(value))
{}

KeyValue::KeyValue (const vector<bool>& value)
: itsDataType (DTVecBool),
  itsExtDT    (DTBool),
  itsValuePtr (new vector<bool>(value))
{}

KeyValue::KeyValue (const vector<int32>& value)
: itsDataType (DTVecInt),
  itsExtDT    (DTInt),
  itsValuePtr (new vector<int32>(value))
{}

KeyValue::KeyValue (const vector<float>& value)
: itsDataType (DTVecFloat),
  itsExtDT    (DTFloat),
  itsValuePtr (new vector<float>(value))
{}

KeyValue::KeyValue (const vector<double>& value)
: itsDataType (DTVecDouble),
  itsExtDT    (DTDouble),
  itsValuePtr (new vector<double>(value))
{}

KeyValue::KeyValue (const vector<fcomplex>& value)
: itsDataType (DTVecFComplex),
  itsExtDT    (DTFComplex),
  itsValuePtr (new vector<fcomplex>(value))
{}

KeyValue::KeyValue (const vector<dcomplex>& value)
: itsDataType (DTVecDComplex),
  itsExtDT    (DTDComplex),
  itsValuePtr (new vector<dcomplex>(value))
{}

KeyValue::KeyValue (const vector<string>& value)
: itsDataType (DTVecString),
  itsExtDT    (DTString),
  itsValuePtr (new vector<string>(value))
{}

KeyValue::KeyValue (const vector<KeyValue>& value)
: itsDataType (DTValueVector),
  itsExtDT    (DTValueVector),
  itsValuePtr (new vector<KeyValue>(value))
{}

KeyValue::KeyValue (const KeyValueMap& value)
: itsDataType (DTValueMap),
  itsExtDT    (DTValueMap),
  itsValuePtr (new KeyValueMap(value))
{}

KeyValue::KeyValue (const KeyValue& that)
: itsValuePtr (0)
{
  copyValue (that);
}
 
KeyValue& KeyValue::operator= (const KeyValue& that)
{
  if (this != &that) {
    clear();
    copyValue (that);
  }
  return *this;
}

KeyValue::~KeyValue()
{
  clear();
}

void KeyValue::clear()
{
  switch (itsDataType) {
  case DTBool:
    delete (bool*)itsValuePtr;
    break;
  case DTInt:
    delete (int32*)itsValuePtr;
    break;
  case DTFloat:
    delete (float*)itsValuePtr;
    break;
  case DTDouble:
    delete (double*)itsValuePtr;
    break;
  case DTFComplex:
    delete (fcomplex*)itsValuePtr;
    break;
  case DTDComplex:
    delete (dcomplex*)itsValuePtr;
    break;
  case DTString:
    delete (string*)itsValuePtr;
    break;
  case DTVecBool:
    delete (vector<bool>*)itsValuePtr;
    break;
  case DTVecInt:
    delete (vector<int32>*)itsValuePtr;
    break;
  case DTVecFloat:
    delete (vector<float>*)itsValuePtr;
    break;
  case DTVecDouble:
    delete (vector<double>*)itsValuePtr;
    break;
  case DTVecFComplex:
    delete (vector<fcomplex>*)itsValuePtr;
    break;
  case DTVecDComplex:
    delete (vector<dcomplex>*)itsValuePtr;
    break;
  case DTVecString:
    delete (vector<string>*)itsValuePtr;
    break;
  case DTValueVector:
    delete (vector<KeyValue>*)itsValuePtr;
    break;
  case DTValueMap:
    delete (KeyValueMap*)itsValuePtr;
    break;
  }
  itsValuePtr = 0;
}

void KeyValue::copyValue (const KeyValue& that)
{
  itsDataType = that.itsDataType;
  itsExtDT    = that.itsExtDT;
  switch (itsDataType) {
  case DTBool:
    itsValuePtr = new bool (that.getBool());
    break;
  case DTInt:
    itsValuePtr = new int32 (that.getInt());
    break;
  case DTFloat:
    itsValuePtr = new float (that.getFloat());
    break;
  case DTDouble:
    itsValuePtr = new double (that.getDouble());
    break;
  case DTFComplex:
    itsValuePtr = new fcomplex (that.getFComplex());
    break;
  case DTDComplex:
    itsValuePtr = new dcomplex (that.getDComplex());
    break;
  case DTString:
    itsValuePtr = new string (that.getString());
    break;
  case DTVecBool:
    itsValuePtr = new vector<bool> (that.getVecBool());
    break;
  case DTVecInt:
    itsValuePtr = new vector<int32> (that.getVecInt());
    break;
  case DTVecFloat:
    itsValuePtr = new vector<float> (that.getVecFloat());
    break;
  case DTVecDouble:
    itsValuePtr = new vector<double> (that.getVecDouble());
    break;
  case DTVecFComplex:
    itsValuePtr = new vector<fcomplex> (that.getVecFComplex());
    break;
  case DTVecDComplex:
    itsValuePtr = new vector<dcomplex> (that.getVecDComplex());
    break;
  case DTVecString:
    itsValuePtr = new vector<string> (that.getVecString());
    break;
  case DTValueVector:
    itsValuePtr = new vector<KeyValue> (that.getVector());
    break;
  case DTValueMap:
    itsValuePtr = new KeyValueMap (that.getValueMap());
    break;
  }
}

uint KeyValue::size() const
{
  switch (itsDataType) {
  case DTVecBool:
    return ((vector<bool>*)itsValuePtr)->size();
    break;
  case DTVecInt:
    return ((vector<int32>*)itsValuePtr)->size();
    break;
  case DTVecFloat:
    return ((vector<float>*)itsValuePtr)->size();
    break;
  case DTVecDouble:
    return ((vector<double>*)itsValuePtr)->size();
    break;
  case DTVecFComplex:
    return ((vector<fcomplex>*)itsValuePtr)->size();
    break;
  case DTVecDComplex:
    return ((vector<dcomplex>*)itsValuePtr)->size();
    break;
  case DTVecString:
    return ((vector<string>*)itsValuePtr)->size();
    break;
  case DTValueVector:
    return ((vector<KeyValue>*)itsValuePtr)->size();
    break;
  default:
    return 1;
  }
}

bool KeyValue::getBool() const
{
  switch (itsDataType) {
  case DTBool:
    return *(bool*)itsValuePtr;
  default:
    throw std::runtime_error("KeyValue::getBool - invalid data type");
  }
  return false;                  // to satisfy compiler
}

int32 KeyValue::getInt() const
{
  switch (itsDataType) {
  case DTInt:
    return *(int32*)itsValuePtr;
  default:
    throw std::runtime_error("KeyValue::getInt - invalid data type");
  }
  return 0;                  // to satisfy compiler
}

float KeyValue::getFloat() const
{
  switch (itsDataType) {
  case DTInt:
    return *(int32*)itsValuePtr;
  case DTFloat:
    return *(float*)itsValuePtr;
  default:
    throw std::runtime_error("KeyValue::getFloat - invalid data type");
  }
  return 0;                  // to satisfy compiler
}

double KeyValue::getDouble() const
{
  switch (itsDataType) {
  case DTInt:
    return *(int32*)itsValuePtr;
  case DTFloat:
    return *(float*)itsValuePtr;
  case DTDouble:
    return *(double*)itsValuePtr;
  default:
    throw std::runtime_error("KeyValue::getDouble - invalid data type");
  }
  return 0;                  // to satisfy compiler
}

fcomplex KeyValue::getFComplex() const
{
  switch (itsDataType) {
  case DTInt:
    return float(*(int32*)itsValuePtr);
  case DTFloat:
    return *(float*)itsValuePtr;
  case DTFComplex:
    return *(fcomplex*)itsValuePtr;
  default:
    throw std::runtime_error("KeyValue::getFComplex - invalid data type");
  }
  return getFComplex();               // to satisfy compiler
}

dcomplex KeyValue::getDComplex() const
{
  switch (itsDataType) {
  case DTInt:
    return double(*(int32*)itsValuePtr);
  case DTFloat:
    return double(*(float*)itsValuePtr);
  case DTDouble:
    return *(double*)itsValuePtr;
  case DTFComplex:
    return dcomplex (((fcomplex*)itsValuePtr)->real(),
		     ((fcomplex*)itsValuePtr)->imag());
  case DTDComplex:
    return *(dcomplex*)itsValuePtr;
  default:
    throw std::runtime_error("KeyValue::getDComplex - invalid data type");
  }
  return getDComplex();               // to satisfy compiler
}

const string& KeyValue::getString() const
{
  switch (itsDataType) {
  case DTString:
    return *(string*)itsValuePtr;
  default:
    throw std::runtime_error("KeyValue::getDComplex - invalid data type");
  }
  return getString();               // to satisfy compiler
}

vector<bool> KeyValue::getVecBool() const
{
  switch (itsDataType) {
  case DTVecBool:
    return *(vector<bool>*)itsValuePtr;
  default:
    {
      vector<bool> vec(1);
      vec[0] = getBool();
      return vec;
    }
  }
}

vector<int32> KeyValue::getVecInt() const
{
  switch (itsDataType) {
  case DTVecInt:
    return *(vector<int32>*)itsValuePtr;
  default:
    {
      vector<int32> vec(1);
      vec[0] = getInt();
      return vec;
    }
  }
}

vector<float> KeyValue::getVecFloat() const
{
  switch (itsDataType) {
  case DTVecInt:
    {
      const vector<int32>& vec1 = *(vector<int32>*)itsValuePtr;
      vector<float> vec2(vec1.size());
      for (uint i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecFloat:
    return *(vector<float>*)itsValuePtr;
  default:
    {
      vector<float> vec(1);
      vec[0] = getFloat();
      return vec;
    }
  }
}

vector<double> KeyValue::getVecDouble() const
{
  switch (itsDataType) {
  case DTVecInt:
    {
      const vector<int32>& vec1 = *(vector<int32>*)itsValuePtr;
      vector<double> vec2(vec1.size());
      for (uint i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecFloat:
    {
      const vector<float>& vec1 = *(vector<float>*)itsValuePtr;
      vector<double> vec2(vec1.size());
      for (uint i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecDouble:
    return *(vector<double>*)itsValuePtr;
  default:
    {
      vector<double> vec(1);
      vec[0] = getDouble();
      return vec;
    }
  }
}

vector<fcomplex> KeyValue::getVecFComplex() const
{
  switch (itsDataType) {
  case DTVecInt:
    {
      const vector<int32>& vec1 = *(vector<int32>*)itsValuePtr;
      vector<fcomplex> vec2(vec1.size());
      for (uint i=0; i<vec2.size(); i++) {
	vec2[i] = float(vec1[i]);
      }
      return vec2;
    }
  case DTVecFloat:
    {
      const vector<float>& vec1 = *(vector<float>*)itsValuePtr;
      vector<fcomplex> vec2(vec1.size());
      for (uint i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecFComplex:
    return *(vector<fcomplex>*)itsValuePtr;
  default:
    {
      vector<fcomplex> vec(1);
      vec[0] = getFComplex();
      return vec;
    }
  }
}

vector<dcomplex> KeyValue::getVecDComplex() const
{
  switch (itsDataType) {
  case DTVecInt:
    {
      const vector<int32>& vec1 = *(vector<int32>*)itsValuePtr;
      vector<dcomplex> vec2(vec1.size());
      for (uint i=0; i<vec2.size(); i++) {
	vec2[i] = double(vec1[i]);
      }
      return vec2;
    }
  case DTVecFloat:
    {
      const vector<float>& vec1 = *(vector<float>*)itsValuePtr;
      vector<dcomplex> vec2(vec1.size());
      for (uint i=0; i<vec2.size(); i++) {
	vec2[i] = double(vec1[i]);
      }
      return vec2;
    }
  case DTVecDouble:
    {
      const vector<double>& vec1 = *(vector<double>*)itsValuePtr;
      vector<dcomplex> vec2(vec1.size());
      for (uint i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecFComplex:
    {
      const vector<fcomplex>& vec1 = *(vector<fcomplex>*)itsValuePtr;
      vector<dcomplex> vec2(vec1.size());
      for (uint i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecDComplex:
    return *(vector<dcomplex>*)itsValuePtr;
  default:
    {
      vector<dcomplex> vec(1);
      vec[0] = getDComplex();
      return vec;
    }
  }
}

vector<string> KeyValue::getVecString() const
{
  switch (itsDataType) {
  case DTVecString:
    return *(vector<string>*)itsValuePtr;
  default:
    {
      vector<string> vec(1);
      vec[0] = getString();
      return vec;
    }
  }
}

const vector<KeyValue>& KeyValue::getVector() const
{
  switch (itsDataType) {
  case DTValueVector:
    return *(vector<KeyValue>*)itsValuePtr;
  default:
    throw std::runtime_error("KeyValue::getVector - invalid data type");
  }
  return getVector();            // to satisfy compiler
}

const KeyValueMap& KeyValue::getValueMap() const
{
  switch (itsDataType) {
  case DTValueMap:
    return *(KeyValueMap*)itsValuePtr;
  default:
    throw std::runtime_error("KeyValue::getValueMap - invalid data type");
  }
  return getValueMap();            // to satisfy compiler
}

void KeyValue::get (KeyValueMap& value) const
{
  value = getValueMap();
}


ostream& operator<< (ostream& os, const KeyValue& param)
{
  switch (param.itsDataType) {
  case KeyValue::DTBool:
    os << *(bool*)(param.itsValuePtr);
    break;
  case KeyValue::DTInt:
    os << *(int32*)(param.itsValuePtr);
    break;
  case KeyValue::DTFloat:
    os << *(float*)(param.itsValuePtr);
    break;
  case KeyValue::DTDouble:
    os << *(double*)(param.itsValuePtr);
    break;
  case KeyValue::DTFComplex:
    {
      const fcomplex& val = *(fcomplex*)(param.itsValuePtr);
      os << val.real() << '+' << val.imag() << 'i';
    }
    break;
  case KeyValue::DTDComplex:
    {
      const dcomplex& val = *(dcomplex*)(param.itsValuePtr);
      os << val.real() << '+' << val.imag() << 'i';
    }
    break;
  case KeyValue::DTString:
    os << *(string*)(param.itsValuePtr);
    break;
  case KeyValue::DTVecBool:
    {
      const vector<bool>& vec = param.getVecBool();
      os << '[';
      for (uint i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case KeyValue::DTVecInt:
    {
      const vector<int32>& vec = param.getVecInt();
      os << '[';
      for (uint i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case KeyValue::DTVecFloat:
    {
      const vector<float>& vec = param.getVecFloat();
      os << '[';
      for (uint i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case KeyValue::DTVecDouble:
    {
      const vector<double>& vec = param.getVecDouble();
      os << '[';
      for (uint i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case KeyValue::DTVecFComplex:
    {
      const vector<fcomplex>& vec = param.getVecFComplex();
      os << '[';
      for (uint i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i].real() << '+' << vec[i].imag() << 'i';
      }
      os << ']';
    }
    break;
  case KeyValue::DTVecDComplex:
    {
      const vector<dcomplex>& vec = param.getVecDComplex();
      os << '[';
      for (uint i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i].real() << '+' << vec[i].imag() << 'i';
      }
      os << ']';
    }
    break;
  case KeyValue::DTVecString:
    {
      const vector<string>& vec = param.getVecString();
      os << '[';
      for (uint i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case KeyValue::DTValueVector:
    {
      const vector<KeyValue>& vec = param.getVector();
      os << '[';
      for (uint i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case KeyValue::DTValueMap:
    {
      const KeyValueMap& blk = param.getValueMap();
      os << '[';
      if (blk.size() == 0) {
	os << '=';
      } else {
	os << blk;
      }
      os << ']';
    }
    break;
  }
  return os;
}

BlobOStream& operator<< (BlobOStream& bs, const KeyValue& param)
{
  bs << int32(param.itsDataType);
  switch (param.itsDataType) {
  case KeyValue::DTBool:
    bs << *(bool*)(param.itsValuePtr);
    break;
  case KeyValue::DTInt:
    bs << *(int32*)(param.itsValuePtr);
    break;
  case KeyValue::DTFloat:
    bs << *(float*)(param.itsValuePtr);
    break;
  case KeyValue::DTDouble:
    bs << *(double*)(param.itsValuePtr);
    break;
  case KeyValue::DTFComplex:
    bs << *(fcomplex*)(param.itsValuePtr);
    break;
  case KeyValue::DTDComplex:
    bs << *(dcomplex*)(param.itsValuePtr);
    break;
  case KeyValue::DTString:
    bs << *(string*)(param.itsValuePtr);
    break;
  case KeyValue::DTVecBool:
    bs.put (*(vector<bool>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecInt:
    bs.put (*(vector<int>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecFloat:
    bs.put (*(vector<float>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecDouble:
    bs.put (*(vector<double>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecFComplex:
    bs.put (*(vector<fcomplex>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecDComplex:
    bs.put (*(vector<dcomplex>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecString:
    bs.put (*(vector<string>*)(param.itsValuePtr));
    break;
  case KeyValue::DTValueVector:
    {
      const vector<KeyValue>& vec = param.getVector();
      bs << uint32(vec.size());
      for (uint i=0; i<vec.size(); i++) {
	bs << vec[i];
      }
    }
    break;
  case KeyValue::DTValueMap:
    bs << *(KeyValueMap*)(param.itsValuePtr);
    break;
  }
  return bs;
}

BlobIStream& operator>> (BlobIStream& bs, KeyValue& param)
{
  param.clear();
  int32 dt;
  bs >> dt;
  param.itsDataType = KeyValue::DataType(dt);
  param.itsExtDT    = param.itsDataType;
  switch (param.itsDataType) {
  case KeyValue::DTBool:
    param.itsValuePtr = new bool;
    bs >> *(bool*)(param.itsValuePtr);
    break;
  case KeyValue::DTInt:
    param.itsValuePtr = new int32;
    bs >> *(int32*)(param.itsValuePtr);
    break;
  case KeyValue::DTFloat:
    param.itsValuePtr = new float;
    bs >> *(float*)(param.itsValuePtr);
    break;
  case KeyValue::DTDouble:
    param.itsValuePtr = new double;
    bs >> *(double*)(param.itsValuePtr);
    break;
  case KeyValue::DTFComplex:
    param.itsValuePtr = new fcomplex;
    bs >> *(fcomplex*)(param.itsValuePtr);
    break;
  case KeyValue::DTDComplex:
    param.itsValuePtr = new dcomplex;
    bs >> *(dcomplex*)(param.itsValuePtr);
    break;
  case KeyValue::DTString:
    param.itsValuePtr = new string;
    bs >> *(string*)(param.itsValuePtr);
    break;
  case KeyValue::DTVecBool:
    param.itsExtDT = KeyValue::DTBool;
    param.itsValuePtr = new vector<bool>;
    bs.get (*(vector<bool>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecInt:
    param.itsExtDT = KeyValue::DTInt;
    param.itsValuePtr = new vector<int32>;
    bs.get (*(vector<int>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecFloat:
    param.itsExtDT = KeyValue::DTFloat;
    param.itsValuePtr = new vector<float>;
    bs.get (*(vector<float>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecDouble:
    param.itsExtDT = KeyValue::DTDouble;
    param.itsValuePtr = new vector<double>;
    bs.get (*(vector<double>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecFComplex:
    param.itsExtDT = KeyValue::DTFComplex;
    param.itsValuePtr = new vector<fcomplex>;
    bs.get (*(vector<fcomplex>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecDComplex:
    param.itsExtDT = KeyValue::DTDComplex;
    param.itsValuePtr = new vector<dcomplex>;
    bs.get (*(vector<dcomplex>*)(param.itsValuePtr));
    break;
  case KeyValue::DTVecString:
    param.itsExtDT = KeyValue::DTString;
    param.itsValuePtr = new vector<string>;
    bs.get (*(vector<string>*)(param.itsValuePtr));
    break;
  case KeyValue::DTValueVector:
    {
      uint32 nr;
      bs >> nr;
      param.itsValuePtr = new vector<KeyValue>(nr);
      vector<KeyValue>& vec = *(vector<KeyValue>*)(param.itsValuePtr);
      for (uint i=0; i<nr; i++) {
	bs >> vec[i];
      }
    }
    break;
  case KeyValue::DTValueMap:
    param.itsValuePtr = new KeyValueMap;
    bs >> *(KeyValueMap*)(param.itsValuePtr);
    break;
  }
  return bs;
}

} // end namespace
