//# UpdateBrokenAntennaInfo.h:
//# Copyright (C) 2012-2014
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
//# $Id: $

#ifndef LOFAR_UPDATEBROKENANTENNAINFOTOPVSS_H
#define LOFAR_UPDATEBROKENANTENNAINFOTOPVSS_H


//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

// LOFAR
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>    // needed for split
#include <Common/Exception.h>     // THROW macro for exceptions
#include <Common/Exceptions.h>

// SAS
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/ClassifConv.h>
#include <OTDB/Converter.h>
#include <OTDB/TreeTypeConv.h>

// STL / C
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdio>
#include <unistd.h>

// boost
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace LOFAR;
// using namespace LOFAR::PVSS;
using namespace LOFAR::OTDB;
using namespace std;
using namespace casa;
using namespace boost::posix_time;

namespace LOFAR {
  namespace PVSS {

    class UpdateBrokenAntennaInfoToPVSS
    {
    
    public:
      UpdateBrokenAntennaInfoToPVSS();
      ~UpdateBrokenAntennaInfoToPVSS();

      vector<OTDBvalue> getBrokenAntennaInfo();
      
    private:
      vector<OTDBvalue> getHardwareTree(OTDBconnection &conn, const string &timeNow);
      void parseBrokenHardware (const vector<OTDBvalue> &hardware);
    };
  };
};

#endif

