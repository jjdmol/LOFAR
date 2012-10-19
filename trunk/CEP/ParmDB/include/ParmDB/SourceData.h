//# SourceData.h: Class holding all parameters of a source
//#
//# Copyright (C) 2012
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
// @brief Base class for a table holding sources and their parameters
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_SOURCEDATA_H
#define LOFAR_PARMDB_SOURCEDATA_H

//# Includes
#include <ParmDB/SourceInfo.h>
#include <Common/lofar_vector.h>


namespace LOFAR {

  //# Forward Declarations.
  class BlobIStream;
  class BlobOStream;
  namespace BBS {
    class ParmMap;
  }

namespace BBS {

  // @ingroup ParmDB
  // @{

  // @brief Class holding a  data of a source.
  class SourceData
  {
  public:
    SourceData();

    SourceData (const SourceInfo&, const string& patchName,
                double ra, double dec);

    // Get the various source parameters.
    const SourceInfo& getInfo() const
      { return itsInfo; }
    const string& getPatchName() const
      { return itsPatchName; }
    double getRa() const
      { return itsRa; }
    double getDec() const
      { return itsDec; }
    double getI() const
      { return itsI; }
    double getQ() const
      { return itsQ; }
    double getU() const
      { return itsU; }
    double getV() const
      { return itsV; }
    double getMajorAxis() const
      { return itsMajorAxis; }
    double getMinorAxis() const
      { return itsMinorAxis; }
    double getOrientation() const
      { return itsOrientation; }
    double getPolarizationAngle() const
      { return itsPolAngle; }
    double getPolarizedFraction() const
      { return itsPolFrac; }
    double getRotationMeasure() const
      { return itsRM; }
    const vector<double>& getSpectralIndex() const
      { return itsSpInx; }

    // Set the various source parameters.
    void setInfo (const SourceInfo& info)
      { itsInfo = info; }
    void setPatchName (const string& patchName)
      { itsPatchName = patchName; }
    void setRa (double ra)
      { itsRa = ra; }
    void setDec (double dec)
      { itsDec = dec; }
    void setI (double i)
      { itsI = i; }
    void setQ (double q)
      { itsQ = q; }
    void setU (double u)
      { itsU = u; }
    void setV (double v)
      { itsV = v; }
    void setMajorAxis (double majorAxis)
      { itsMajorAxis = majorAxis; }
    void setMinorAxis (double minorAxis)
      { itsMinorAxis = minorAxis; }
    void setOrientation (double orientation)
      { itsOrientation = orientation; }
    void setPolarizationAngle (double polarizationAngle)
      { itsPolAngle = polarizationAngle; }
    void setPolarizedFraction (double polarizedFraction)
      { itsPolFrac =  polarizedFraction; }
    void setRotationMeasure (double potationMeasure)
      { itsRM = potationMeasure; }
    void setSpectralIndex (const vector<double>& spectralIndex)
      { itsSpInx = spectralIndex; }

    // Set the parameters from a ParmMap object.
    void setParms (const ParmMap& defaultParameters);

    // Write the source data into a blob stream.
    void writeSource (BlobOStream&);

    // Read the source data from a blob stream.
    void readSource (BlobIStream&);

  private:
    // Set a parameter.
    // If defined, its value is taken from the map.
    // Otherwise the default value is used.
    void setParm (const ParmMap& parms, const string& name,
                  double defValue, double& value);

    SourceInfo     itsInfo;
    string         itsPatchName;
    double         itsRa;
    double         itsDec;
    double         itsI;
    double         itsQ;
    double         itsU;
    double         itsV;
    double         itsMajorAxis;
    double         itsMinorAxis;
    double         itsOrientation;
    double         itsPolAngle;
    double         itsPolFrac;
    double         itsRM;
    vector<double> itsSpInx;
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif
