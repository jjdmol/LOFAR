//# KeyValue.h: Class to hold a general value
//#
//# Copyright (C) 2001
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BLOB_KEYVALUE_H
#define LOFAR_BLOB_KEYVALUE_H

// \file
// Class to hold a general value.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>
#include <Common/LofarTypes.h>

namespace LOFAR {

  // \ingroup Blob
  // \addtogroup KeyValue
  // <group>

  //# Forward Declarations
  class KeyValueMap;
  class BlobOStream;
  class BlobIStream;
  
  // <summary> Class to hold a general value </summary>
  //
  // The KeyValue and KeyValueMap act as a map of heterogeneous values.
  // A KeyValue object can contain a value from a set of data types.
  // The data types supported are all standard types (including string and
  // complex) and a std::vector of these types.
  // Furthermore the data type can be a KeyValueMap making it possible to have
  // nested structs.
  // Finally it can be a std::vector of KeyValue making it possible to have 
  // a vector of heterogeneous values.
  
  class KeyValue
    {
    public:
      enum DataType {DTBool, DTInt, DTFloat, DTDouble,
		     DTFComplex, DTDComplex, DTString,
		     DTValueMap,
		     DTValueVector,
		     DTVecBool, DTVecInt, DTVecFloat, DTVecDouble,
		     DTVecFComplex, DTVecDComplex, DTVecString};
      
      //#  enum AngleType {DMS, HMS, TIME};
      enum AngleType {DMS, HMS};
      
      // \name Constructors
      // Construct value with given type. Default is empty vector<KeyValue>.
      // <group>
      KeyValue();
      KeyValue (bool);
      KeyValue (int32);
      KeyValue (float);
      KeyValue (double);
      KeyValue (const fcomplex&);
      KeyValue (const dcomplex&);
      KeyValue (const char*);
      KeyValue (const string&);
      KeyValue (const vector<bool>&);
      KeyValue (const vector<int32>&);
      KeyValue (const vector<float>&);
      KeyValue (const vector<double>&);
      KeyValue (const vector<fcomplex>&);
      KeyValue (const vector<dcomplex>&);
      KeyValue (const vector<string>&);
      KeyValue (const vector<KeyValue>&);
      KeyValue (const KeyValueMap&);
      // </group>
      
      // Construct from a string representing a time or angle resulting in
      // a double value.
      // <br>HMS has to look as HH:MM:SS.S and is converted to radians.
      // <br>DMS has to look as DD.MM.SS.S and is converted to radians.
      //# <br>TIME has to look as YYYY/MM/DD/HH:MM:SS.S and is converted to MJD.
      KeyValue (const string&, AngleType);
      
      // Copy constructor (copy semantics).
      KeyValue (const KeyValue&);
      
      // Assignment (copy semantics).
      KeyValue& operator= (const KeyValue&);
      
      ~KeyValue();
      
      // Get the data type of the value.
      DataType dataType() const
	{ return itsExtDT; }
      
      // Is the value a vector?
      bool isVector() const
	{ return itsDataType >= DTValueVector; }
      
      // Is the value a value map?
      bool isValueMap() const
	{ return itsDataType == DTValueMap; }
      
      // Return the size of a vector (1 is returned for a scalar).
      unsigned int size() const;
      
      // \name Get functions
      // Get the value with the given type.
      // <group>
      bool getBool() const;
      int getInt() const;
      float getFloat() const;
      double getDouble() const;
      fcomplex getFComplex() const;
      dcomplex getDComplex() const;
      const string& getString() const;
      vector<bool> getVecBool() const;
      vector<int32> getVecInt() const;
      vector<float> getVecFloat() const;
      vector<double> getVecDouble() const;
      vector<fcomplex> getVecFComplex() const;
      vector<dcomplex> getVecDComplex() const;
      vector<string> getVecString() const;
      const vector<KeyValue>& getVector() const;
      const KeyValueMap& getValueMap() const;
      // </group>

      // \name Get functions for templated purposes
      // Get the value with the given type.
      // <group>
      void get (bool& value) const
	{ value = getBool(); }
      void get (int& value) const
	{ value = getInt(); }
      void get (float& value) const
	{ value = getFloat(); }
      void get (double& value) const
	{ value = getDouble(); }
      void get (fcomplex& value) const
	{ value = getFComplex(); }
      void get (dcomplex& value) const
	{ value = getDComplex(); }
      void get (string& value) const
	{ value = getString(); }
      void get (vector<bool>& value) const
	{ value = getVecBool(); }
      void get (vector<int32>& value) const
	{ value = getVecInt(); }
      void get (vector<float>& value) const
	{ value = getVecFloat(); }
      void get (vector<double>& value) const
	{ value = getVecDouble(); }
      void get (vector<fcomplex>& value) const
	{ value = getVecFComplex(); }
      void get (vector<dcomplex>& value) const
	{ value = getVecDComplex(); }
      void get (vector<string>& value) const
	{ value = getVecString(); }
      void get (vector<KeyValue>& value) const
	{ value = getVector(); }
      void get (KeyValueMap& value) const;
      // </group>

      friend ostream& operator<< (ostream&, const KeyValue&);
      
      friend BlobOStream& operator<< (BlobOStream&, const KeyValue&);
      friend BlobIStream& operator>> (BlobIStream&, KeyValue&);
      
      // \name Parse a position value.
      // Parse a position value and return the value in radians.
      // <group>
      static double parsePos (const string& value,
			      KeyValue::AngleType atype);
      static double parse (const string& value, char sep, double factor);
      // </group>
      
    private:
      // Remove the value.
      void clear();
      
      // Copy the value from another one.
      void copyValue (const KeyValue& that);
      
      DataType itsDataType;
      DataType itsExtDT;
      void*    itsValuePtr;
    };

  // </group>

} // end namespace

#endif 
