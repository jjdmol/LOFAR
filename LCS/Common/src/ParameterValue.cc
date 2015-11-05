//# ParameterValue.h: The value of a parameter
//#
//# Copyright (C) 2008
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

#include<lofar_config.h>
#include <Common/ParameterValue.h>
#include <Common/ParameterRecord.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <cstdio>

namespace LOFAR { 

  ParameterValue::ParameterValue (const string& value, bool trim)
    : itsValue (value)
  {
    if (trim) {
      uint st  = lskipws (0, itsValue.size());
      uint end = rskipws (st, itsValue.size());
      if (st > 0  ||  end < itsValue.size()) {
        itsValue = itsValue.substr(st, end-st);
      }
    }
  }
  
  ParameterValue ParameterValue::expand() const
  {
    return ParameterValue (expandArrayString (itsValue));
  }

  vector<ParameterValue> ParameterValue::splitValue (uint st, uint last) const
  {
    // Allocate result.
    // Empty result if only whitespace left.
    vector<ParameterValue> result;
    st = lskipws (st, last);
    if (st == last) {
      return result;
    }
    // Split on commas, but take quotes, braces, parentheses, and brackets
    // into account.
    int nrpar=0;
    int nrbracket=0;
    int nrbrace=0;
    uint i = st;
    while (i < last) {
      if (itsValue[i] == '\''  ||  itsValue[i] == '"') {
        i = skipQuoted (itsValue, i);
      } else {
        if (itsValue[i] == '(') {
          nrpar++;
        } else if (itsValue[i] == ')') {
          nrpar--;
        } else if (itsValue[i] == '[') {
          ASSERT (nrpar == 0);
          nrbracket++;
        } else if (itsValue[i] == ']') {
          nrbracket--;
        } else if (itsValue[i] == '{') {
          ASSERT (nrpar == 0);
          nrbrace++;
        } else if (itsValue[i] == '}') {
          nrbrace--;
        } else if (itsValue[i] == ',') {
          if (nrpar+nrbracket+nrbrace == 0) {
            result.push_back (ParameterValue(substr(st, i)));
            st = i+1;
          }
        }
        ASSERT (nrpar >= 0  &&  nrbracket >= 0);
        i++;
      }
    }
    result.push_back (ParameterValue(substr(st, last)));
    ASSERT (nrpar == 0  &&  nrbracket == 0  &&  nrbrace == 0);
    return result;
  }

  vector<ParameterValue> ParameterValue::getVector() const
  {
    uint st   = 1;
    uint last = itsValue.size() - 1;
    // A single value if there is no opening and closing bracket.
    if (itsValue.empty() || !(itsValue[0] == '['  &&  itsValue[last] == ']')) {
      return vector<ParameterValue> (1, *this);
    }
    ASSERT (itsValue.size() >= 2  &&  itsValue[0] == '['  &&
            itsValue[last] == ']');
    return splitValue (st, last);
  }

  ParameterRecord ParameterValue::getRecord() const
  {
    uint st   = 1;
    uint last = itsValue.size() - 1;
    ASSERT (itsValue.size() >= 2  &&  itsValue[0] == '{'  &&
            itsValue[last] == '}');
    vector<ParameterValue> values (splitValue (st, last));
    // Loop over all values and split in names and values.
    ParameterRecord result;
    for (vector<ParameterValue>::const_iterator iter = values.begin();
         iter!=values.end(); ++iter) {
      const string& str = iter->get();
      uint st = 0;
      if (str[0] == '"'  ||  str[0] == '\'') {
        st = skipQuoted (str, 0);
      }
      string::size_type pos = str.find(':', st);
      ASSERT (pos != string::npos);
      // Get name and value.
      // ParameterValue is used to remove possible whitespace.
      // getString also removes quotes in case the name was quoted.
      string name (ParameterValue(str.substr(0, pos)).getString());
      string value (ParameterValue(str.substr(pos+1)).get());
      result.add (name, value);
    }
    return result;
  }

  string ParameterValue::getString() const
  {
    // Remove possible quotes around the value.
    uint sz = itsValue.size();
    if (sz < 2  ||  (itsValue[0] != '"'  &&  itsValue[0] != '\'')) {
      return itsValue;
    }
    ASSERT (itsValue[0] == itsValue[sz-1]);
    return itsValue.substr(1, sz-2);
  }

  vector<bool> ParameterValue::getBoolVector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<bool> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (*iter);
    }
    return result;
  }

  vector<int> ParameterValue::getIntVector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<int> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (*iter);
    }
    return result;
  }

  vector<uint> ParameterValue::getUintVector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<uint> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (*iter);
    }
    return result;
  }

  vector<int16> ParameterValue::getInt16Vector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<int16> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (iter->getInt16());
    }
    return result;
  }

  vector<uint16> ParameterValue::getUint16Vector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<uint16> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (iter->getUint16());
    }
    return result;
  }

  vector<int32> ParameterValue::getInt32Vector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<int32> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (iter->getInt32());
    }
    return result;
  }

  vector<uint32> ParameterValue::getUint32Vector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<uint32> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (iter->getUint32());
    }
    return result;
  }

#ifdef HAVE_LONG_LONG
  vector<int64> ParameterValue::getInt64Vector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<int64> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (iter->getInt64());
    }
    return result;
  }

  vector<uint64> ParameterValue::getUint64Vector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<uint64> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (iter->getUint64());
    }
    return result;
  }
#endif

  vector<float> ParameterValue::getFloatVector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<float> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (*iter);
    }
    return result;
  }

  vector<double> ParameterValue::getDoubleVector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<double> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (*iter);
    }
    return result;
  }

  vector<string> ParameterValue::getStringVector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<string> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (*iter);
    }
    return result;
  }

  vector<time_t> ParameterValue::getTimeVector() const
  {
    vector<ParameterValue> vec (getVector());
    vector<time_t> result;
    result.reserve (vec.size());
    for (vector<ParameterValue>::const_iterator iter = vec.begin();
         iter != vec.end(); ++iter) {
      result.push_back (*iter);
    }
    return result;
  }

time_t ParameterValue::StringToTime_t (const string& aString) 
{
  time_t theTime;
  char   unit[1024];
  unit[0] = '\0';
  if (sscanf (aString.c_str(), "%ld%s", &theTime, unit) < 1) {
    THROW (APSException, aString + " is not an time value");
  }
  switch (unit[0]) {
  case 's':
  case 'S':
  case '\0':
    break;
  case 'm':
  case 'M':
    theTime *= 60;
    break;
  case 'h':
  case 'H':
    theTime *= 3600;
    break;
  }
  return theTime;
}

}
