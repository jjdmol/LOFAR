//# MSLofarFieldColumns.cc: provides easy access to LOFAR's MSField columns
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
#include <MSLofar/MSLofarFieldColumns.h>
#include <MSLofar/MSLofarField.h>

using namespace casa;

namespace LOFAR {

  ROMSLofarFieldColumns::ROMSLofarFieldColumns
  (const MSLofarField& msLofarField)
  {
    attach (msLofarField);
  }

  ROMSLofarFieldColumns::~ROMSLofarFieldColumns()
  {}

  ROMSLofarFieldColumns::ROMSLofarFieldColumns()
  {}

  void ROMSLofarFieldColumns::attach
  (const MSLofarField& msLofarField)
  {
    ROMSFieldColumns::attach (msLofarField);
    tileBeamDir_p.attach (msLofarField, "LOFAR_TILE_BEAM_DIR");
    tileBeamDirMeas_p.attach (msLofarField, "LOFAR_TILE_BEAM_DIR");
  }


  MSLofarFieldColumns::MSLofarFieldColumns
  (MSLofarField& msLofarField)
  {
    attach (msLofarField);
  }

  MSLofarFieldColumns::~MSLofarFieldColumns()
  {}

  MSLofarFieldColumns::MSLofarFieldColumns()
  {}

  void MSLofarFieldColumns::attach
  (MSLofarField& msLofarField)
  {
    MSFieldColumns::attach (msLofarField);
    roTileBeamDir_p.attach (msLofarField, "LOFAR_TILE_BEAM_DIR");
    rwTileBeamDir_p.attach (msLofarField, "LOFAR_TILE_BEAM_DIR");
    roTileBeamDirMeas_p.attach (msLofarField, "LOFAR_TILE_BEAM_DIR");
    rwTileBeamDirMeas_p.attach (msLofarField, "LOFAR_TILE_BEAM_DIR");
  }

  void MSLofarFieldColumns::setDirectionRef(MDirection::Types ref)
  {
    rwTileBeamDirMeas_p.setDescRefCode (ref);
    MSFieldColumns::setDirectionRef (ref);
  }

  void MSLofarFieldColumns::setDirectionRef(MDirection::Types ref,
                                            MDirection::Types beamDirRef)
  {
    rwTileBeamDirMeas_p.setDescRefCode (beamDirRef);
    MSFieldColumns::setDirectionRef (ref);
  }

  ///  void MSLofarFieldColumns::setDirectionOffset(const MDirection& offset)
  ///  {
  ///    rwTileBeamDirMeas_p.setDescOffset (offset);
  ///    MSFieldColumns::setDirectionOffset (offset);
  ///  }

  ///  void MSLofarFieldColumns::setDirectionOffset(const MDirection& offset,
  ///                                               const MDirection& beamDiroffset)
  ///  {
  ///    rwTileBeamDirMeas_p.setDescOffset (beamDirOffset);
  ///    MSFieldColumns::setDirectionOffset (offset);
  ///  }

} //# end namespace
