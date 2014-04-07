//# SourceInfo.cc: Info about a source
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

//# Includes
#include <ParmDB/SourceInfo.h>
#include <casa/Arrays/Array.h>

namespace LOFAR {
namespace BBS {

  SourceInfo::SourceInfo (const string& name, Type type,
                          uint spectralIndexNTerms,
                          double spectralIndexRefFreqHz,
                          bool useRotationMeasure)
    : itsName           (name),
      itsType           (type),
      itsSpInxNTerms    (spectralIndexNTerms),
      itsSpInxRefFreq   (spectralIndexRefFreqHz),
      itsUseRotMeas     (useRotationMeasure),
      itsShapeletScaleI (0),
      itsShapeletScaleQ (0),
      itsShapeletScaleU (0),
      itsShapeletScaleV (0)
  {}

  SourceInfo::SourceInfo (const SourceInfo& that)
  {
    operator= (that);
  }

  SourceInfo& SourceInfo::operator= (const SourceInfo& that)
  {
    if (this != &that) {
      itsName           = that.itsName;
      itsType           = that.itsType;
      itsSpInxNTerms    = that.itsSpInxNTerms;
      itsSpInxRefFreq   = that.itsSpInxRefFreq;
      itsUseRotMeas     = that.itsUseRotMeas;
      itsShapeletScaleI = that.itsShapeletScaleI;
      itsShapeletScaleQ = that.itsShapeletScaleQ;
      itsShapeletScaleU = that.itsShapeletScaleU;
      itsShapeletScaleV = that.itsShapeletScaleV;
      itsShapeletCoeffI.assign (itsShapeletCoeffI);
      itsShapeletCoeffQ.assign (itsShapeletCoeffQ);
      itsShapeletCoeffU.assign (itsShapeletCoeffU);
      itsShapeletCoeffV.assign (itsShapeletCoeffV);
    }
    return *this;
  }

  void SourceInfo::setShapeletCoeff (const casa::Array<double>& I,
                                     const casa::Array<double>& Q,
                                     const casa::Array<double>& U,
                                     const casa::Array<double>& V)
  {
    itsShapeletCoeffI.assign (I);
    itsShapeletCoeffQ.assign (Q);
    itsShapeletCoeffU.assign (U);
    itsShapeletCoeffV.assign (V);
  }

  void SourceInfo::setShapeletScale (double scaleI, double scaleQ,
                                     double scaleU, double scaleV)
  {
    itsShapeletScaleI = scaleI;
    itsShapeletScaleQ = scaleQ;
    itsShapeletScaleU = scaleU;
    itsShapeletScaleV = scaleV;
  }

} // namespace BBS
} // namespace LOFAR
