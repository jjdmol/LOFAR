//# SourceValue.h: The parameters of a single source
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

#ifndef LOFAR_SOURCEDB_SOURCEVALUE_H
#define LOFAR_SOURCEDB_SOURCEVALUE_H

// \file
// The parameters of a single source.

//# Includes
#include <ParmDB/ParmDomain.h>
#include <string>
#include <vector>

namespace LOFAR {
namespace SourceDB {

// \ingroup SourceDB
// @{

// This class defines the parameters of a source.
// The parameters can be frequency and time dependent.
// By default the time and frequency range are infinite.

class SourceValue
{
public:
  // Create a default SourceValue object.
  SourceValue();

  // Set the various source parameters.
  // <group>
  void setPointSource (const std::string& name, double ra, double dec,
		       double flux[4], double spectralIndex);
  ///  void setGaussianSource ();
  void setPos (double ra, double dec);
  // </group>

  // Set the domain.
  // The domain can have 0, 1, or 2 axes (freq (Hz) and time (MJD seconds)).
  void setDomain (const ParmDB::ParmDomain&);

  // Set the frequency domain (in Hz) for which the parameters are valid.
  void setFreqDomain (double startHz, double endHz);

  // Set the time domain (in MJD seconds) for which the parameters are valid.
  void setTimeDomain (double startMJD, double endMJD);

  // Get the various source parameters.
  // <group>
  const std::string& getName() const
    { return itsName; }
  const std::string& getType() const
    { return itsType; }
  double getRA() const
    { return itsRA; }
  double getDEC() const
    { return itsDEC; }
  double getSpectralIndex() const
    { return itsSpIndex; }
  const double* getFlux() const
    { return itsFlux; }
  // </group>

  // Get source parameters.
  // <group>
  void setRA (double ra)
    { itsRA = ra; }
  void setDEC (double dec)
    { itsDEC = dec; }
  void setFluxI (double flux)
    { itsFlux[0] = flux; }
  void setFluxQ (double flux)
    { itsFlux[1] = flux; }
  void setFluxU (double flux)
    { itsFlux[2] = flux; }
  void setFluxV (double flux)
    { itsFlux[3] = flux; }
  void setSpectralIndex (double spectralIndex)
    { itsSpIndex = spectralIndex; }
  // </group>

  // Get the domain.
  // <group>
  const ParmDB::ParmDomain& getDomain() const
    { return itsDomain; }
  double getStartFreq() const
    { return itsDomain.getStart()[0]; }
  double getEndFreq() const
    { return itsDomain.getEnd()[0]; }
  double getStartTime() const
    { return itsDomain.getStart()[1]; }
  double getEndTime() const
    { return itsDomain.getEnd()[1]; }
  // </group>

private:
  std::string itsName;
  std::string itsType;
  double      itsRA;
  double      itsDEC;
  double      itsFlux[4];
  double      itsSpIndex;
  ParmDB::ParmDomain itsDomain;
};

// @}

} // namespace SourceDB
} // namespace LOFAR

#endif
