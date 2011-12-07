//# MSElementFailureColumns.cc: provides easy access to MSElementFailure columns
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
#include <MSLofar/MSElementFailureColumns.h>
#include <MSLofar/MSElementFailure.h>

namespace LOFAR {

  ROMSElementFailureColumns::ROMSElementFailureColumns
  (const MSElementFailure& msElementFailure)
  {
    attach (msElementFailure);
  }

  ROMSElementFailureColumns::~ROMSElementFailureColumns()
  {}

  ROMSElementFailureColumns::ROMSElementFailureColumns()
  {}

  void ROMSElementFailureColumns::attach
  (const MSElementFailure& msElementFailure)
  {
    antennaFieldId_p.attach (msElementFailure, "ANTENNA_FIELD_ID");
    elementIndex_p.attach   (msElementFailure, "ELEMENT_INDEX");
    time_p.attach           (msElementFailure, "TIME");
    timeMeas_p.attach       (msElementFailure, "TIME");
    timeQuant_p.attach      (msElementFailure, "TIME");
  }


  MSElementFailureColumns::MSElementFailureColumns
  (MSElementFailure& msElementFailure)
  {
    attach (msElementFailure);
  }

  MSElementFailureColumns::~MSElementFailureColumns()
  {}

  MSElementFailureColumns::MSElementFailureColumns()
  {}

  void MSElementFailureColumns::attach
  (MSElementFailure& msElementFailure)
  {
    ROMSElementFailureColumns::attach (msElementFailure);
    antennaFieldId_p.attach (msElementFailure, "ANTENNA_FIELD_ID");
    elementIndex_p.attach   (msElementFailure, "ELEMENT_INDEX");
    time_p.attach           (msElementFailure, "TIME");
    timeMeas_p.attach       (msElementFailure, "TIME");
    timeQuant_p.attach      (msElementFailure, "TIME");
  }

} //# end namespace
