//# MSLofarAntennaColumns.cc: provides easy access to LOFAR's MSAntenna columns
//# Copyright (C) 2011
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
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <MSLofar/MSLofarAntennaColumns.h>
#include <MSLofar/MSLofarAntenna.h>

using namespace casa;

namespace LOFAR {

  ROMSLofarAntennaColumns::ROMSLofarAntennaColumns
  (const MSLofarAntenna& msLofarAntenna)
  {
    attach (msLofarAntenna);
  }

  ROMSLofarAntennaColumns::~ROMSLofarAntennaColumns()
  {}

  ROMSLofarAntennaColumns::ROMSLofarAntennaColumns()
  {}

  void ROMSLofarAntennaColumns::attach
  (const MSLofarAntenna& msLofarAntenna)
  {
    ROMSAntennaColumns::attach (msLofarAntenna);
    stationId_p.attach (msLofarAntenna, "LOFAR_STATION_ID");
    phaseReference_p.attach (msLofarAntenna, "LOFAR_PHASE_REFERENCE");
    phaseReferenceQuant_p.attach (msLofarAntenna, "LOFAR_PHASE_REFERENCE");
    phaseReferenceMeas_p.attach (msLofarAntenna, "LOFAR_PHASE_REFERENCE");
  }


  MSLofarAntennaColumns::MSLofarAntennaColumns
  (MSLofarAntenna& msLofarAntenna)
  {
    attach (msLofarAntenna);
  }

  MSLofarAntennaColumns::~MSLofarAntennaColumns()
  {}

  MSLofarAntennaColumns::MSLofarAntennaColumns()
  {}

  void MSLofarAntennaColumns::attach
  (MSLofarAntenna& msLofarAntenna)
  {
    MSAntennaColumns::attach (msLofarAntenna);
    roStationId_p.attach (msLofarAntenna, "LOFAR_STATION_ID");
    rwStationId_p.attach (msLofarAntenna, "LOFAR_STATION_ID");
    roPhaseReference_p.attach (msLofarAntenna, "LOFAR_PHASE_REFERENCE");
    rwPhaseReference_p.attach (msLofarAntenna, "LOFAR_PHASE_REFERENCE");
    roPhaseReferenceQuant_p.attach (msLofarAntenna, "LOFAR_PHASE_REFERENCE");
    rwPhaseReferenceQuant_p.attach (msLofarAntenna, "LOFAR_PHASE_REFERENCE");
    roPhaseReferenceMeas_p.attach (msLofarAntenna, "LOFAR_PHASE_REFERENCE");
    rwPhaseReferenceMeas_p.attach (msLofarAntenna, "LOFAR_PHASE_REFERENCE");
  }

} //# end namespace
