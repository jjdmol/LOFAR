//# ParameterRecord.h: A record of parameter values
//#
//# Copyright (C) 2012
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

#ifndef LOFAR_COMMON_PARAMETERRECORD_H
#define LOFAR_COMMON_PARAMETERRECORD_H

// \file
// A record of parameter values

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/ParameterSet.h>

namespace LOFAR { 

  class ParameterRecord: public ParameterSet
  {
  public:
    // Define the iterators for this class.
    typedef ParameterSet::iterator       iterator;
    typedef ParameterSet::const_iterator const_iterator;

    // Default constructor creates empty record.
    ParameterRecord()
    {}

    // Try to get a value from the record or from a nested record.
    bool getRecursive (const string& key, ParameterValue& value) const;

    // Put to ostream.
    friend ostream& operator<< (ostream& os, const ParameterRecord&);
  };

}

#endif
