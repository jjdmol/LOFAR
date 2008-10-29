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

#include<lofar_config.h>
#include <APS/ParameterValue.h>
#include <APS/Exceptions.h>
#include <Common/LofarLogger.h>

namespace LOFAR { namespace ACC { namespace APS {

      ParameterValue::ParameterValue (const string& value, bool trim)
    : itsValue (value)
  {
    if (trim) {
      uint st  = ltrim (0, itsValue.size());
      uint end = rtrim (st, itsValue.size());
      if (st > 0  ||  end < itsValue.size()) {
        itsValue = itsValue.substr(st, end-st);
      }
    }
  }

  ParameterValue ParameterValue::expand() const
  {
    return ParameterValue (expandedArrayString (itsValue));
  }

  uint ParameterValue::ltrim (uint st, uint end) const
  {
    for (; st<end; ++st) {
      if (itsValue[st] != ' '  &&  itsValue[st] != '\t') {
        break;
      }
    }
    return st;
  }
  
  uint ParameterValue::rtrim (uint st, uint end) const
  {
    for (; end>st; --end) {
      if (itsValue[end-1] != ' '  &&  itsValue[end-1] != '\t') {
        break;
      }
    }
    return end;
  }
  
  vector<ParameterValue> ParameterValue::getVector() const
  {
    // A single value if there is no opening and closing bracket.
    int st = 1;
    uint last = itsValue.size() - 1;
    if (!(itsValue[0] == '['  &&  itsValue[last] == ']')) {
      return vector<ParameterValue> (1, *this);
    }
    // Allocate result.
    vector<ParameterValue> result;
    // Split on commas, but take quotes, parentheses, and brackets into account.
    bool squote = false;
    bool dquote = false;
    bool quote = false;
    int nrpar=0;
    int nrbracket=0;
    for (uint i=st; i<last; ++i) {
      if (!dquote) {
        if (itsValue[i] == '\'') {
          squote = !squote;
          quote  = squote;
        }
      } else if (!squote) {
        if (itsValue[i] == '"') {
          dquote = !dquote;
          quote  = dquote;
        }
      }
      if (!quote) {
        if (itsValue[i] == '(') {
          nrpar++;
        } else if (itsValue[i] == ')') {
          nrpar--;
        } else if (itsValue[i] == '[') {
          ASSERT (nrpar == 0);
          nrbracket++;
        } else if (itsValue[i] == ']') {
          nrbracket--;
        } else if (itsValue[i] == ',') {
          if (nrpar+nrbracket == 0) {
            result.push_back (ParameterValue(substr(st, i)));
            st = i+1;
          }
        }
        ASSERT (nrpar >= 0  &&  nrbracket >= 0);
      }
    }
    result.push_back (ParameterValue(substr(st, last)));
    ASSERT (!quote  &&  nrpar == 0  &&  nrbracket == 0);
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

}}}
