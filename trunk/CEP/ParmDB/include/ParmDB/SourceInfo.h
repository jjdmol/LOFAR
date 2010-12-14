//# SourceInfo.h: Info about a source
//#
//# Copyright (C) 2008
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

// @file
// @brief Info about a source
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_SOURCEINFO_H
#define LOFAR_PARMDB_SOURCEINFO_H

//# Includes
#include <Common/lofar_string.h>
#include <casa/Arrays/Array.h>


namespace LOFAR {
namespace BBS {

  // @ingroup ParmDB
  // @{

  // @brief Info about a source
  class SourceInfo
  {
  public:
    // Define the source types.
    //# The values should never be changed.
    enum Type {POINT = 0,
               GAUSSIAN = 1,
               DISK = 2,
               SHAPELET = 3,
               SUN = 10,
               MOON = 11,
               JUPITER = 12,
               MARS = 13,
               VENUS = 14
    };


    // Create from source name, type and other info.
    // <br>A positive spectralIndexSize means that BBS will take
    // a spectral index with size terms into account when calculating
    // the flux. The values of the terms are in the associated ParmDB. It
    // uses the given reference frequency (in Hz).
    // <br> useRotationMeasure indicates that Q and U have to be calculated
    // using a rotation measure, polarization angle, and polarized fraction.
    SourceInfo (const string& name, Type type,
                uint spectralIndexNTerms=0, double spectralIndexRefFreqHz=0.,
                bool useRotationMeasure=false)
      : itsName          (name),
        itsType          (type),
        itsSpInxNTerms   (spectralIndexNTerms),
        itsSpInxRefFreq  (spectralIndexRefFreqHz),
        itsUseRotMeas    (useRotationMeasure),
        itsShapeletScale (0)
    {}

    // Get the source name.
    const string& getName() const
      { return itsName; }

    // Get the source type.
    Type getType() const
      { return itsType; }

    // Get the number of terms in the spectral index function.
    // A value 0 means that the spectral index is not used.
    uint getSpectralIndexNTerms() const
      { return itsSpInxNTerms; }

    // Get the reference frequency (in Hz) for the spectral index.
    double getSpectralIndexRefFreq() const
      { return itsSpInxRefFreq; }

    // Tell if Q,U are directly given or have to be calculated from
    // rotation measure, polarisation fraction and angle.
    bool getUseRotationMeasure() const
      { return itsUseRotMeas; }

    // Set or get the shapelet info.
    // <group>
    const casa::Array<double>& getShapeletCoeff() const
      { return itsShapeletCoeff; }
    double getShapeletScale() const
      { return itsShapeletScale; }
    void setShapeletCoeff (const casa::Array<double>& coeff)
      { itsShapeletCoeff = coeff; }
    void setShapeletScale (double scale)
      { itsShapeletScale = scale; }
    // </group>

  private:
    string itsName;          // source name
    Type   itsType;          // source type
    uint   itsSpInxNTerms;   // nr of terms in the spectral index function
    double itsSpInxRefFreq;  // reference frequency (Hz) for spectral index
    bool   itsUseRotMeas;    // true=use RM,PolFrac,PolAngle; false=use Q,U
    casa::Array<double> itsShapeletCoeff;  // shapelet coefficients
    double itsShapeletScale;               // shapelet scale
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif
