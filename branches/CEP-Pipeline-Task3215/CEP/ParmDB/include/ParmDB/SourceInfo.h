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
#include <Common/LofarTypes.h>
#include <casa/Arrays/Array.h>
#include <measures/Measures/MDirection.h>


namespace LOFAR {

  //# Forward Declaration.
  class BlobIStream;
  class BlobOStream;

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
               SHAPELET = 3
    };

    // Create from source name, type, reference type and other info.
    // <br>The 'type' argument tells the source type (point, gaussian, etc.).
    // <br>The 'refType' argument tells the MDirection reference frame
    // (J2000, SUN, etc.).
    // <br>A positive spectralIndexSize means that BBS will take
    // a spectral index with size terms into account when calculating
    // the flux. The values of the terms are in the associated ParmDB. It
    // uses the given reference frequency (in Hz).
    // <br> useRotationMeasure indicates that Q and U have to be calculated
    // using a rotation measure, polarization angle, and polarized fraction.
    SourceInfo (const string& name, Type type, const string& refType="J2000",
                uint spectralIndexNTerms=0, double spectralIndexRefFreqHz=0.,
                bool useRotationMeasure=false);

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

    // Get the reference type.
    const string& getRefType() const
      { return itsRefType; }

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

    // Write into a blob.
    void write (BlobOStream&);

    // Read from a blob.
    void read (BlobIStream&);

  private:
    string itsName;           // source name
    Type   itsType;           // source type
    string itsRefType;        // reference type
    uint32 itsSpInxNTerms;    // nr of terms in the spectral index function
    double itsSpInxRefFreq;   // reference frequency (Hz) for spectral index
    bool   itsUseRotMeas;     // true=use RM,PolFrac,PolAngle; false=use Q,U
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
