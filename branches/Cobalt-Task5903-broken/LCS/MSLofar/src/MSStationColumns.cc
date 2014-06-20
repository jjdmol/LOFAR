//# MSStationColumns.cc: provides easy access to MSStation columns
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
#include <MSLofar/MSStationColumns.h>
#include <MSLofar/MSStation.h>

namespace LOFAR {

  ROMSStationColumns::ROMSStationColumns
  (const MSStation& msStation)
  {
    attach (msStation);
  }

  ROMSStationColumns::~ROMSStationColumns()
  {}

  ROMSStationColumns::ROMSStationColumns()
  {}

  void ROMSStationColumns::attach
  (const MSStation& msStation)
  {
    name_p.attach      (msStation, "NAME");
    clockId_p.attach   (msStation, "CLOCK_ID");
    flagRow_p.attach   (msStation, "FLAG_ROW");
  }


  MSStationColumns::MSStationColumns
  (MSStation& msStation)
  {
    attach (msStation);
  }

  MSStationColumns::~MSStationColumns()
  {}

  MSStationColumns::MSStationColumns()
  {}

  void MSStationColumns::attach
  (MSStation& msStation)
  {
    ROMSStationColumns::attach (msStation);
    name_p.attach      (msStation, "NAME");
    clockId_p.attach   (msStation, "CLOCK_ID");
    flagRow_p.attach   (msStation, "FLAG_ROW");
  }

} //# end namespace
