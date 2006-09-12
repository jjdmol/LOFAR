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

namespace LOFAR
{
  //# Forward declarations
  class BlobIStream;
  class BlobOStream;

  namespace BBS
  {
    // \addtogroup BBS
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
    };

    // Domain size is defined by two parameters: bandwidth f(Hz), and time
    // interval t(s).
    struct DomainSize
    {
      DomainSize() : bandWidth(0), timeInterval(0) {}
      double bandWidth;	    ///< Bandwidth in Hz.
      double timeInterval;  ///< Time interval is seconds.
    };

    // Information about which correlation products (auto, cross, or both),
    // and which polarizations should be used.
    struct Correlation
    {
      Correlation() : selection(NONE) {}
      enum Selection {
	NONE,               ///< No correlations
	AUTO,               ///< Auto correlation only.
	CROSS,              ///< Cross correlation only.
	ALL                 ///< Both auto- and cross correlation
      } selection;
      vector<string> type;  ///< E.g., ["XX", "XY", "YX", "YY"]
    };

    // Sizes of the integration intervals, when applied. Integration can be
    // performed along the frequency axis and the time axis.q
    struct Integration
    {
      Integration() : deltaFreq(0), deltaTime(0) {}
      double deltaFreq;     ///< frequency integration interval: f(Hz)
      double deltaTime;     ///< time integration interval: t(s)
    };

    // Two vectors of stations names, which, when paired element-wise, define
    // the baselines to be used in the current step. Names may contain
    // wildcards, like \c * and \c ?. If they do, then all possible baselines
    // will be constructed from the expanded names. Expansion of wildcards
    // will be done in the BBS kernel.
    // 
    // For example, suppose that: 
    // \verbatim 
    // station1 = ["CS*", "RS1"]
    // station2 = ["CS*", "RS2"] 
    // \endverbatim
    // Furthermore, suppose that \c CS* expands to \c CS1, \c CS2, and \c
    // CS3. Then, in the BBS kernel, seven baselines will be constructed:
    // \verbatim
    // [ CS1:CS1, CS1:CS2, CS1:CS3, CS2:CS2, CS2:CS3, CS3:CS3, RS1:RS2 ]
    // \endverbatim
    // 
    // \note Station names are \e not expanded by matching with all existing
    // %LOFAR stations, but only with those that took part in a particular
    // observation; i.e., only those stations that are mentioned in the \c
    // ANTENNA table in the Measurement Set.
    struct Baselines
    {
      vector<string> station1;
      vector<string> station2;
    };


    // Write the contents of these structs in human readable form.
    // @{
    ostream& operator<<(ostream&, const BBDB&);
    ostream& operator<<(ostream&, const ParmDB&);
    ostream& operator<<(ostream&, const DomainSize&);
    ostream& operator<<(ostream&, const Correlation&);
    ostream& operator<<(ostream&, const Integration&);
    ostream& operator<<(ostream&, const Baselines&);
    // @}

    // Blob I/O methods for these structs.
    // @{
    BlobOStream& operator<<(BlobOStream&, const BBDB&);
    BlobOStream& operator<<(BlobOStream&, const ParmDB&);
    BlobOStream& operator<<(BlobOStream&, const DomainSize&);
    BlobOStream& operator<<(BlobOStream&, const Correlation&);
    BlobOStream& operator<<(BlobOStream&, const Integration&);
    BlobOStream& operator<<(BlobOStream&, const Baselines&);

    BlobIStream& operator>>(BlobIStream&, BBDB&);
    BlobIStream& operator>>(BlobIStream&, ParmDB&);
    BlobIStream& operator>>(BlobIStream&, DomainSize&);
    BlobIStream& operator>>(BlobIStream&, Correlation&);
    BlobIStream& operator>>(BlobIStream&, Integration&);
    BlobIStream& operator>>(BlobIStream&, Baselines&);
    // @}

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
