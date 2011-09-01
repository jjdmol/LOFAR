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

    // Define the RotationMeasure calculation type.
    enum RMType {RM_NONE = 0,    
                 RM_POL  = 1,    // use polarization angle and fraction
                 RM_QU   = 2     // use Q and U
    };


    // Create from source name, type and other info.
    // <br>A positive spectralIndexSize means that BBS will take
    // a spectral index with size terms into account when calculating
    // the flux. The values of the terms are in the associated ParmDB. It
    // uses the given reference frequency (in Hz).
    // <br> <src>rmType</src> tells if and how rotation measure have to be used.
    // If used, the reference wavelength must be given (in meters).
    SourceInfo (const string& name, Type type,
                uint spectralIndexNTerms=0, double spectralIndexRefFreqHz=0.,
                RMType rmType = RM_NONE,
                double rotMeasRefWavelengthM = 0);

    // Copy constructor.
    SourceInfo (const SourceInfo&);

    // Assignment.
    SourceInfo& operator= (const SourceInfo&);

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

    // Get the RotationMeasure calculation type.
    RMType getRMType() const
      { return itsRMType; }

    // Get the reference wavelength (in m) for the rotation measure.
    double getRMRefWavelength() const
      { return itsRMRefWavel; }

    // Set or get the shapelet info.
    // <group>
    const casa::Array<double>& getShapeletCoeffI() const
      { return itsShapeletCoeffI; }
    const casa::Array<double>& getShapeletCoeffQ() const
      { return itsShapeletCoeffQ; }
    const casa::Array<double>& getShapeletCoeffU() const
      { return itsShapeletCoeffU; }
    const casa::Array<double>& getShapeletCoeffV() const
      { return itsShapeletCoeffV; }
    double getShapeletScaleI() const
      { return itsShapeletScaleI; }
    double getShapeletScaleQ() const
      { return itsShapeletScaleQ; }
    double getShapeletScaleU() const
      { return itsShapeletScaleU; }
    double getShapeletScaleV() const
      { return itsShapeletScaleV; }
    void setShapeletCoeff (const casa::Array<double>& I,
                           const casa::Array<double>& Q,
                           const casa::Array<double>& U,
                           const casa::Array<double>& V);
    void setShapeletScale (double scaleI, double scaleQ,
                           double scaleU, double scaleV);
    // </group>

  private:
    string itsName;           // source name
    Type   itsType;           // source type
    uint   itsSpInxNTerms;    // nr of terms in the spectral index function
    double itsSpInxRefFreq;   // reference frequency (Hz) for spectral index
    RMType itsRMType;         // use RM?
    double itsRMRefWavel;     // reference wavelength (m) for rotation measure
    double itsShapeletScaleI; // shapelet scale for I-flux
    double itsShapeletScaleQ;
    double itsShapeletScaleU;
    double itsShapeletScaleV;
    casa::Array<double> itsShapeletCoeffI;  // shapelet coefficients I-flux
    casa::Array<double> itsShapeletCoeffQ;
    casa::Array<double> itsShapeletCoeffU;
    casa::Array<double> itsShapeletCoeffV;
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif
