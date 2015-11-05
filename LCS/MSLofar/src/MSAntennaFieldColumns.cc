//# MSAntennaFieldColumns.cc: provides easy access to MSAntennaField columns
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
#include <MSLofar/MSAntennaFieldColumns.h>
#include <MSLofar/MSAntennaField.h>

namespace LOFAR {

  ROMSAntennaFieldColumns::ROMSAntennaFieldColumns
  (const MSAntennaField& msAntennaField)
  {
    attach (msAntennaField);
  }

  ROMSAntennaFieldColumns::~ROMSAntennaFieldColumns()
  {}

  ROMSAntennaFieldColumns::ROMSAntennaFieldColumns()
  {}

  void ROMSAntennaFieldColumns::attach
  (const MSAntennaField& msAntennaField)
  {
    antennaId_p.attach              (msAntennaField, "ANTENNA_ID");
    name_p.attach                   (msAntennaField, "NAME");
    position_p.attach               (msAntennaField, "POSITION");
    positionMeas_p.attach           (msAntennaField, "POSITION");
    positionQuant_p.attach          (msAntennaField, "POSITION");
    coordinateAxes_p.attach         (msAntennaField, "COORDINATE_AXES");
    coordinateAxesQuant_p.attach    (msAntennaField, "COORDINATE_AXES");
    elementOffset_p.attach          (msAntennaField, "ELEMENT_OFFSET");
    elementOffsetQuant_p.attach     (msAntennaField, "ELEMENT_OFFSET");
    elementFlag_p.attach            (msAntennaField, "ELEMENT_FLAG");
    tileRotation_p.attach           (msAntennaField, "TILE_ROTATION");
    tileRotationQuant_p.attach      (msAntennaField, "TILE_ROTATION");
    tileElementOffset_p.attach      (msAntennaField, "TILE_ELEMENT_OFFSET");
    tileElementOffsetQuant_p.attach (msAntennaField, "TILE_ELEMENT_OFFSET");
  }


  MSAntennaFieldColumns::MSAntennaFieldColumns
  (MSAntennaField& msAntennaField)
  {
    attach (msAntennaField);
  }

  MSAntennaFieldColumns::~MSAntennaFieldColumns()
  {}

  MSAntennaFieldColumns::MSAntennaFieldColumns()
  {}

  void MSAntennaFieldColumns::attach
  (MSAntennaField& msAntennaField)
  {
    ROMSAntennaFieldColumns::attach (msAntennaField);
    antennaId_p.attach              (msAntennaField, "ANTENNA_ID");
    name_p.attach                   (msAntennaField, "NAME");
    position_p.attach               (msAntennaField, "POSITION");
    positionMeas_p.attach           (msAntennaField, "POSITION");
    positionQuant_p.attach          (msAntennaField, "POSITION");
    coordinateAxes_p.attach         (msAntennaField, "COORDINATE_AXES");
    coordinateAxesQuant_p.attach    (msAntennaField, "COORDINATE_AXES");
    elementOffset_p.attach          (msAntennaField, "ELEMENT_OFFSET");
    elementOffsetQuant_p.attach     (msAntennaField, "ELEMENT_OFFSET");
    elementFlag_p.attach            (msAntennaField, "ELEMENT_FLAG");
    tileRotation_p.attach           (msAntennaField, "TILE_ROTATION");
    tileRotationQuant_p.attach      (msAntennaField, "TILE_ROTATION");
    tileElementOffset_p.attach      (msAntennaField, "TILE_ELEMENT_OFFSET");
    tileElementOffsetQuant_p.attach (msAntennaField, "TILE_ELEMENT_OFFSET");
  }

} //# end namespace
