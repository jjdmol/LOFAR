//#  ParameterValue.h: The value of a parameter
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

#ifndef LOFAR_APS_PARAMETERVALUE_H
#define LOFAR_APS_PARAMETERVALUE_H

// \file
// The value of a parameter

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/StringUtil.h>

namespace LOFAR { namespace ACC { namespace APS {

  class ParameterValue
  {
  public:
    // Default constructor uses empty string.
    ParameterValue()
    {}

    // Create from the given string.
    // Optionally left and right whitespace will be removed.
    explicit ParameterValue (const string& value, bool trim=true);

    // Expand the string using StringUtil::expandedArrayString.
    ParameterValue expand() const;

    // Is the value a vector?
    bool isVector() const
      { return itsValue[0] == '['; }

    // Get the value string.
    const string& get() const
      { return itsValue; }

    // Split value into a vector of ParameterValues.
    vector<ParameterValue> getVector() const;

    // Get the parameter value in the given type.
    // <group>
    bool   getBool() const
      { return StringToBool(itsValue); }
    int    getInt() const
      { return StringToInt32(itsValue); }
    uint   getUint() const
      { return StringToUint32(itsValue); }
#ifdef HAVE_LONG_LONG
    int64  getInt64() const
      { return StringToInt64(itsValue); }
    uint64 getUint64() const
      { return StringToUint64(itsValue); }
#endif
    float  getFloat() const
      { return StringToFloat(itsValue); }
    double getDouble() const
      { return StringToDouble(itsValue); }
    string getString() const;
    time_t getTime() const
      { return StringToTime_t(itsValue); }
    vector<bool>   getBoolVector() const;
    vector<int>    getIntVector() const;
    vector<uint>   getUintVector() const;
#ifdef HAVE_LONG_LONG
    vector<int64>  getInt64Vector() const;
    vector<uint64> getUint64Vector() const;
#endif
    vector<float>  getFloatVector() const;
    vector<double> getDoubleVector() const;
    vector<string> getStringVector() const;
    vector<time_t> getTimeVector() const;
    // </group>

    // Convert the parameter value to the given type using conversion operators.
    // <group>
    operator bool() const
      { return getBool(); }
    operator int() const
      { return getInt(); }
    operator uint() const
      { return getUint(); }
    operator float() const
      { return getFloat(); }
    operator double() const
      { return getDouble(); }
    operator string() const
      { return getString(); }
    operator time_t() const
      { return getTime(); }
    operator vector<bool>() const
      { return getBoolVector(); }
    operator vector<int>() const
      { return getIntVector(); }
    operator vector<uint>() const
      { return getUintVector(); }
    operator vector<float>() const
      { return getFloatVector(); }
    operator vector<double>() const
      { return getDoubleVector(); }
    operator vector<string>() const
      { return getStringVector(); }
    operator vector<time_t>() const
      { return getTimeVector(); }
    // <group>

    // Convert a string to a time.
    static time_t StringToTime_t (const string& aString) ;

  private:
    // Return the position of the first non-whitespace character in itsValue
    // starting at st.
    uint ltrim (uint st, uint end) const;
  
    // Return the position after the last non-whitespace character in itsValue
    // back from end.
    uint rtrim (uint st, uint end) const;

    // Return the substring with left and right whitespace removed.
    ParameterValue substr (int st, int end) const
      { return ParameterValue (itsValue.substr (st, end-st)); }

    //# Data Members.
    string itsValue;
  };

}}}

#endif
