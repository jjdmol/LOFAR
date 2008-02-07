//# BBSStructs.h: Some global structs.
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_BBSSTRUCTS_H
#define LOFAR_BBSCONTROL_BBSSTRUCTS_H

// \file
// Some global structs. The main purpose of these structs is to bundle data
// that are logically related. Most of these structs are used in more than one
// class, which justifies them being defined here, outside these classes.

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>
#include <APS/ParameterSet.h>
#include <BBSKernel/BBSKernelStructs.h>

namespace LOFAR
{
  //# Forward declarations
  namespace ACC { namespace APS { class ParameterSet; } }
  class BlobIStream;
  class BlobOStream;

  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    // Information about the Blackboard database.
    struct BBDB
    {
      BBDB() : port(0) {}
      string host;          ///< Host name or IP address of the BB DBMS
      uint16 port;          ///< Port used by BB DBMS
      string dbName;        ///< Name of the BB database
      string username;      ///< Username for accessing the DBMS
      string password;      ///< Password for accessing the DBMS
    };

    // Information about the parameter database.
    // \note These are currently AIPS++ MS tables.
    struct ParmDB
    {
      string instrument;    ///< Instrument parameters (MS table)
      string localSky;	    ///< Local sky parameters (MS table)
      string history;       ///< History (MS table)
    };

    // Selection of the data domain that is to be processed.
    struct RegionOfInterest
    {
      vector<int32>   frequency;
      //        uint                stepChannel;
      vector<string>  time;
      //        vector<string>      stations;
    };    
    
    // Domain size is defined by two parameters: bandwidth f(Hz), and time
    // interval t(s).
    struct DomainSize
    {
      DomainSize() : bandWidth(0), timeInterval(0) {}
      double bandWidth;	    ///< Bandwidth in Hz.
      double timeInterval;  ///< Time interval is seconds.
    };

    // Sizes of the integration intervals, when applied. Integration can be
    // performed along the frequency axis and the time axis.q
    struct Integration
    {
      Integration() : deltaFreq(0), deltaTime(0) {}
      double deltaFreq;     ///< frequency integration interval: f(Hz)
      double deltaTime;     ///< time integration interval: t(s)
    };

    // Write the contents of these structs in human readable form.
    // @{
    ostream& operator<<(ostream&, const BBDB&);
    ostream& operator<<(ostream&, const ParmDB&);
    ostream& operator<<(ostream&, const DomainSize&);
    ostream& operator<<(ostream&, const RegionOfInterest&);
    ostream& operator<<(ostream&, const Integration&);
    // @}

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
