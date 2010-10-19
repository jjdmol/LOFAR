//# SourceValue.cc: The parameters of a single source
//#
//# Copyright (C) 2008
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

#include <SourceDB/SourceValue.h>
#include <Common/LofarLogger.h>

namespace LOFAR {
namespace SourceDB {

  SourceValue::SourceValue()
    : itsRA        (0),
      itsDEC       (0),
      itsSpIndex   (0),
      itsDomain    (-1e30, 1e30, -1e30, 1e30)
  {
    itsFlux[0] = 1;
    itsFlux[1] = 0;
    itsFlux[2] = 0;
    itsFlux[3] = 0;
  }

  void SourceValue::setPointSource (const std::string& name,
				    double RA, double DEC,
				    double flux[4], double spectralIndex)
  {
    itsName    = name;
    itsType    = "point";
    itsRA      = RA;
    itsDEC     = DEC;
    itsSpIndex = spectralIndex;
    itsFlux[0] = flux[0];
    itsFlux[1] = flux[1];
    itsFlux[2] = flux[2];
    itsFlux[3] = flux[3];
  }

  void SourceValue::setPos (double RA, double DEC)
  {
    itsRA  = RA;
    itsDEC = DEC;
  }

  void SourceValue::setDomain (const ParmDB::ParmDomain& domain)
  {
    ASSERTSTR (domain.getStart().size() <= 2, "Domain has max. 2 axes");
    if (domain.getStart().size() == 2) {
      itsDomain = domain;
    } else if (domain.getStart().size() == 1) {
      itsDomain = ParmDB::ParmDomain (domain.getStart()[0],
				      domain.getEnd()[0],
				      -1e30, 1e30);
    } else {
      itsDomain = ParmDB::ParmDomain (-1e30, 1e30, -1e30, 1e30);
    }
  }

  void SourceValue::setFreqDomain (double startHz, double endHz)
  {
    // Keep current time.
    itsDomain = ParmDB::ParmDomain (startHz, endHz,
				    itsDomain.getStart()[1],
				    itsDomain.getEnd()[1]);
  }

  void SourceValue::setTimeDomain (double startMJD, double endMJD)
  {
    // Keep current freq.
    itsDomain = ParmDB::ParmDomain (itsDomain.getStart()[0],
				    itsDomain.getEnd()[0],
				    startMJD, endMJD);
  }

} // namespace SourceDB
} // namespace LOFAR
