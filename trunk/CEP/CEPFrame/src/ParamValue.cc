// ParamValue.cc: Class to hold a general value
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
//
//
//
//////////////////////////////////////////////////////////////////////


#include "CEPFrame/ParamValue.h"
#include "CEPFrame/ParamBlock.h"
#include <stdexcept>


ParamValue::ParamValue()
: itsDataType (DTValueVector),
  itsExtDT    (DTValueVector),
  itsValuePtr (new vector<ParamValue>())
{}

ParamValue::ParamValue (bool value)
: itsDataType (DTBool),
  itsExtDT    (DTBool),
  itsValuePtr (new bool(value))
{}

ParamValue::ParamValue (int value)
: itsDataType (DTInt),
  itsExtDT    (DTInt),
  itsValuePtr (new int(value))
{}

ParamValue::ParamValue (float value)
: itsDataType (DTFloat),
  itsExtDT    (DTFloat),
  itsValuePtr (new float(value))
{}

ParamValue::ParamValue (double value)
: itsDataType (DTDouble),
  itsExtDT    (DTDouble),
  itsValuePtr (new double(value))
{}

ParamValue::ParamValue (const complex<float>& value)
: itsDataType (DTComplex),
  itsExtDT    (DTComplex),
  itsValuePtr (new complex<float>(value))
{}

ParamValue::ParamValue (const complex<double>& value)
: itsDataType (DTDComplex),
  itsExtDT    (DTDComplex),
  itsValuePtr (new complex<double>(value))
{}

ParamValue::ParamValue (const char* value)
: itsDataType (DTString),
  itsExtDT    (DTString),
  itsValuePtr (new string(value))
{}

ParamValue::ParamValue (const string& value)
: itsDataType (DTString),
  itsExtDT    (DTString),
  itsValuePtr (new string(value))
{}

ParamValue::ParamValue (const vector<bool>& value)
: itsDataType (DTVecBool),
  itsExtDT    (DTBool),
  itsValuePtr (new vector<bool>(value))
{}

ParamValue::ParamValue (const vector<int>& value)
: itsDataType (DTVecInt),
  itsExtDT    (DTInt),
  itsValuePtr (new vector<int>(value))
{}

ParamValue::ParamValue (const vector<float>& value)
: itsDataType (DTVecFloat),
  itsExtDT    (DTFloat),
  itsValuePtr (new vector<float>(value))
{}

ParamValue::ParamValue (const vector<double>& value)
: itsDataType (DTVecDouble),
  itsExtDT    (DTDouble),
  itsValuePtr (new vector<double>(value))
{}

ParamValue::ParamValue (const vector<complex<float> >& value)
: itsDataType (DTVecComplex),
  itsExtDT    (DTComplex),
  itsValuePtr (new vector<complex<float> >(value))
{}

ParamValue::ParamValue (const vector<complex<double> >& value)
: itsDataType (DTVecDComplex),
  itsExtDT    (DTDComplex),
  itsValuePtr (new vector<complex<double> >(value))
{}

ParamValue::ParamValue (const vector<string>& value)
: itsDataType (DTVecString),
  itsExtDT    (DTString),
  itsValuePtr (new vector<string>(value))
{}

ParamValue::ParamValue (const vector<ParamValue>& value)
: itsDataType (DTValueVector),
  itsExtDT    (DTValueVector),
  itsValuePtr (new vector<ParamValue>(value))
{}

ParamValue::ParamValue (const ParamBlock& value)
: itsDataType (DTBlock),
  itsExtDT    (DTBlock),
  itsValuePtr (new ParamBlock(value))
{}

ParamValue::ParamValue (const ParamValue& that)
: itsValuePtr (0)
{
  copyValue (that);
}
 
ParamValue& ParamValue::operator= (const ParamValue& that)
{
  if (this != &that) {
    clear();
    copyValue (that);
  }
  return *this;
}

ParamValue::~ParamValue()
{
  clear();
}

void ParamValue::clear()
{
  switch (itsDataType) {
  case DTBool:
    delete (bool*)itsValuePtr;
    break;
  case DTInt:
    delete (int*)itsValuePtr;
    break;
  case DTFloat:
    delete (float*)itsValuePtr;
    break;
  case DTDouble:
    delete (double*)itsValuePtr;
    break;
  case DTComplex:
    delete (complex<float>*)itsValuePtr;
    break;
  case DTDComplex:
    delete (complex<double>*)itsValuePtr;
    break;
  case DTString:
    delete (string*)itsValuePtr;
    break;
  case DTVecBool:
    delete (vector<bool>*)itsValuePtr;
    break;
  case DTVecInt:
    delete (vector<int>*)itsValuePtr;
    break;
  case DTVecFloat:
    delete (vector<float>*)itsValuePtr;
    break;
  case DTVecDouble:
    delete (vector<double>*)itsValuePtr;
    break;
  case DTVecComplex:
    delete (vector<complex<float> >*)itsValuePtr;
    break;
  case DTVecDComplex:
    delete (vector<complex<double> >*)itsValuePtr;
    break;
  case DTVecString:
    delete (vector<string>*)itsValuePtr;
    break;
  case DTValueVector:
    delete (vector<ParamValue>*)itsValuePtr;
    break;
  case DTBlock:
    delete (ParamBlock*)itsValuePtr;
    break;
  }
  itsValuePtr = 0;
}

void ParamValue::copyValue (const ParamValue& that)
{
  itsDataType = that.itsDataType;
  itsExtDT    = that.itsExtDT;
  switch (itsDataType) {
  case DTBool:
    itsValuePtr = new bool (that.getBool());
    break;
  case DTInt:
    itsValuePtr = new int (that.getInt());
    break;
  case DTFloat:
    itsValuePtr = new float (that.getFloat());
    break;
  case DTDouble:
    itsValuePtr = new double (that.getDouble());
    break;
  case DTComplex:
    itsValuePtr = new complex<float> (that.getComplex());
    break;
  case DTDComplex:
    itsValuePtr = new complex<double> (that.getDComplex());
    break;
  case DTString:
    itsValuePtr = new string (that.getString());
    break;
  case DTVecBool:
    itsValuePtr = new vector<bool> (that.getVecBool());
    break;
  case DTVecInt:
    itsValuePtr = new vector<int> (that.getVecInt());
    break;
  case DTVecFloat:
    itsValuePtr = new vector<float> (that.getVecFloat());
    break;
  case DTVecDouble:
    itsValuePtr = new vector<double> (that.getVecDouble());
    break;
  case DTVecComplex:
    itsValuePtr = new vector<complex<float> > (that.getVecComplex());
    break;
  case DTVecDComplex:
    itsValuePtr = new vector<complex<double> > (that.getVecDComplex());
    break;
  case DTVecString:
    itsValuePtr = new vector<string> (that.getVecString());
    break;
  case DTValueVector:
    itsValuePtr = new vector<ParamValue> (that.getVector());
    break;
  case DTBlock:
    itsValuePtr = new ParamBlock (that.getBlock());
    break;
  }
}

unsigned int ParamValue::size() const
{
  switch (itsDataType) {
  case DTVecBool:
    return ((vector<bool>*)itsValuePtr)->size();
    break;
  case DTVecInt:
    return ((vector<int>*)itsValuePtr)->size();
    break;
  case DTVecFloat:
    return ((vector<float>*)itsValuePtr)->size();
    break;
  case DTVecDouble:
    return ((vector<double>*)itsValuePtr)->size();
    break;
  case DTVecComplex:
    return ((vector<complex<float> >*)itsValuePtr)->size();
    break;
  case DTVecDComplex:
    return ((vector<complex<double> >*)itsValuePtr)->size();
    break;
  case DTVecString:
    return ((vector<string>*)itsValuePtr)->size();
    break;
  case DTValueVector:
    return ((vector<ParamValue>*)itsValuePtr)->size();
    break;
  default:
    return 1;
  }
}

bool ParamValue::getBool() const
{
  switch (itsDataType) {
  case DTBool:
    return *(bool*)itsValuePtr;
  default:
    throw std::runtime_error("ParamValue::getBool - invalid data type");
  }
  return false;                  // to satisfy compiler
}

int ParamValue::getInt() const
{
  switch (itsDataType) {
  case DTInt:
    return *(int*)itsValuePtr;
  default:
    throw std::runtime_error("ParamValue::getInt - invalid data type");
  }
  return 0;                  // to satisfy compiler
}

float ParamValue::getFloat() const
{
  switch (itsDataType) {
  case DTInt:
    return *(int*)itsValuePtr;
  case DTFloat:
    return *(float*)itsValuePtr;
  default:
    throw std::runtime_error("ParamValue::getFloat - invalid data type");
  }
  return 0;                  // to satisfy compiler
}

double ParamValue::getDouble() const
{
  switch (itsDataType) {
  case DTInt:
    return *(int*)itsValuePtr;
  case DTFloat:
    return *(float*)itsValuePtr;
  case DTDouble:
    return *(double*)itsValuePtr;
  default:
    throw std::runtime_error("ParamValue::getDouble - invalid data type");
  }
  return 0;                  // to satisfy compiler
}

complex<float> ParamValue::getComplex() const
{
  switch (itsDataType) {
  case DTInt:
    return float(*(int*)itsValuePtr);
  case DTFloat:
    return *(float*)itsValuePtr;
  case DTComplex:
    return *(complex<float>*)itsValuePtr;
  default:
    throw std::runtime_error("ParamValue::getComplex - invalid data type");
  }
  return getComplex();               // to satisfy compiler
}

complex<double> ParamValue::getDComplex() const
{
  switch (itsDataType) {
  case DTInt:
    return double(*(int*)itsValuePtr);
  case DTFloat:
    return double(*(float*)itsValuePtr);
  case DTDouble:
    return *(double*)itsValuePtr;
  case DTComplex:
    return complex<double> (((complex<float>*)itsValuePtr)->real(),
			    ((complex<float>*)itsValuePtr)->imag());
  case DTDComplex:
    return *(complex<double>*)itsValuePtr;
  default:
    throw std::runtime_error("ParamValue::getDComplex - invalid data type");
  }
  return getDComplex();               // to satisfy compiler
}

const string& ParamValue::getString() const
{
  switch (itsDataType) {
  case DTString:
    return *(string*)itsValuePtr;
  default:
    throw std::runtime_error("ParamValue::getDComplex - invalid data type");
  }
  return getString();               // to satisfy compiler
}

vector<bool> ParamValue::getVecBool() const
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

vector<int> ParamValue::getVecInt() const
{
  switch (itsDataType) {
  case DTVecInt:
    return *(vector<int>*)itsValuePtr;
  default:
    {
      vector<int> vec(1);
      vec[0] = getInt();
      return vec;
    }
  }
}

vector<float> ParamValue::getVecFloat() const
{
  switch (itsDataType) {
  case DTVecInt:
    {
      const vector<int>& vec1 = *(vector<int>*)itsValuePtr;
      vector<float> vec2(vec1.size());
      for (unsigned int i=0; i<vec2.size(); i++) {
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

vector<double> ParamValue::getVecDouble() const
{
  switch (itsDataType) {
  case DTVecInt:
    {
      const vector<int>& vec1 = *(vector<int>*)itsValuePtr;
      vector<double> vec2(vec1.size());
      for (unsigned int i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecFloat:
    {
      const vector<float>& vec1 = *(vector<float>*)itsValuePtr;
      vector<double> vec2(vec1.size());
      for (unsigned int i=0; i<vec2.size(); i++) {
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

vector<complex<float> > ParamValue::getVecComplex() const
{
  switch (itsDataType) {
  case DTVecInt:
    {
      const vector<int>& vec1 = *(vector<int>*)itsValuePtr;
      vector<complex<float> > vec2(vec1.size());
      for (unsigned int i=0; i<vec2.size(); i++) {
	vec2[i] = float(vec1[i]);
      }
      return vec2;
    }
  case DTVecFloat:
    {
      const vector<float>& vec1 = *(vector<float>*)itsValuePtr;
      vector<complex<float> > vec2(vec1.size());
      for (unsigned int i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecComplex:
    return *(vector<complex<float> >*)itsValuePtr;
  default:
    {
      vector<complex<float> > vec(1);
      vec[0] = getComplex();
      return vec;
    }
  }
}

vector<complex<double> > ParamValue::getVecDComplex() const
{
  switch (itsDataType) {
  case DTVecInt:
    {
      const vector<int>& vec1 = *(vector<int>*)itsValuePtr;
      vector<complex<double> > vec2(vec1.size());
      for (unsigned int i=0; i<vec2.size(); i++) {
	vec2[i] = double(vec1[i]);
      }
      return vec2;
    }
  case DTVecFloat:
    {
      const vector<float>& vec1 = *(vector<float>*)itsValuePtr;
      vector<complex<double> > vec2(vec1.size());
      for (unsigned int i=0; i<vec2.size(); i++) {
	vec2[i] = double(vec1[i]);
      }
      return vec2;
    }
  case DTVecDouble:
    {
      const vector<double>& vec1 = *(vector<double>*)itsValuePtr;
      vector<complex<double> > vec2(vec1.size());
      for (unsigned int i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecComplex:
    {
      const vector<complex<float> >& vec1 = *(vector<complex<float> >*)itsValuePtr;
      vector<complex<double> > vec2(vec1.size());
      for (unsigned int i=0; i<vec2.size(); i++) {
	vec2[i] = vec1[i];
      }
      return vec2;
    }
  case DTVecDComplex:
    return *(vector<complex<double> >*)itsValuePtr;
  default:
    {
      vector<complex<double> > vec(1);
      vec[0] = getDComplex();
      return vec;
    }
  }
}

vector<string> ParamValue::getVecString() const
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

const vector<ParamValue>& ParamValue::getVector() const
{
  switch (itsDataType) {
  case DTValueVector:
    return *(vector<ParamValue>*)itsValuePtr;
  default:
    throw std::runtime_error("ParamValue::getVector - invalid data type");
  }
  return getVector();            // to satisfy compiler
}

const ParamBlock& ParamValue::getBlock() const
{
  switch (itsDataType) {
  case DTBlock:
    return *(ParamBlock*)itsValuePtr;
  default:
    throw std::runtime_error("ParamValue::getBlock - invalid data type");
  }
  return getBlock();            // to satisfy compiler
}

void ParamValue::get (ParamBlock& value) const
{
  value = getBlock();
}


ostream& operator<< (ostream& os, const ParamValue& param)
{
  switch (param.itsDataType) {
  case ParamValue::DTBool:
    os << *(bool*)(param.itsValuePtr);
    break;
  case ParamValue::DTInt:
    os << *(int*)(param.itsValuePtr);
    break;
  case ParamValue::DTFloat:
    os << *(float*)(param.itsValuePtr);
    break;
  case ParamValue::DTDouble:
    os << *(double*)(param.itsValuePtr);
    break;
  case ParamValue::DTComplex:
    {
      const complex<float>& val = *(complex<float>*)(param.itsValuePtr);
      os << val.real() << '+' << val.imag() << 'i';
    }
    break;
  case ParamValue::DTDComplex:
    {
      const complex<double>& val = *(complex<double>*)(param.itsValuePtr);
      os << val.real() << '+' << val.imag() << 'i';
    }
    break;
  case ParamValue::DTString:
    os << *(string*)(param.itsValuePtr);
    break;
  case ParamValue::DTVecBool:
    {
      const vector<bool>& vec = param.getVecBool();
      os << '[';
      for (unsigned i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case ParamValue::DTVecInt:
    {
      const vector<int>& vec = param.getVecInt();
      os << '[';
      for (unsigned i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case ParamValue::DTVecFloat:
    {
      const vector<float>& vec = param.getVecFloat();
      os << '[';
      for (unsigned i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case ParamValue::DTVecDouble:
    {
      const vector<double>& vec = param.getVecDouble();
      os << '[';
      for (unsigned i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case ParamValue::DTVecComplex:
    {
      const vector<complex<float> >& vec = param.getVecComplex();
      os << '[';
      for (unsigned i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i].real() << '+' << vec[i].imag() << 'i';
      }
      os << ']';
    }
    break;
  case ParamValue::DTVecDComplex:
    {
      const vector<complex<double> >& vec = param.getVecDComplex();
      os << '[';
      for (unsigned i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i].real() << '+' << vec[i].imag() << 'i';
      }
      os << ']';
    }
    break;
  case ParamValue::DTVecString:
    {
      const vector<string>& vec = param.getVecString();
      os << '[';
      for (unsigned i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case ParamValue::DTValueVector:
    {
      const vector<ParamValue>& vec = param.getVector();
      os << '[';
      for (unsigned i=0; i<vec.size(); i++) {
	if (i > 0) {
	  os << ',';
	}
	os << vec[i];
      }
      os << ']';
    }
    break;
  case ParamValue::DTBlock:
    {
      const ParamBlock& blk = param.getBlock();
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
