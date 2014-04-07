//# SourceData.cc: Class for a Blob file holding sources and their parameters
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

#include <lofar_config.h>
#include <ParmDB/SourceData.h>
#include <ParmDB/ParmMap.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/Exception.h>
#include <iostream>

using namespace std;

namespace LOFAR {
namespace BBS {


  SourceData::SourceData()
    : itsInfo (string(), SourceInfo::POINT)
  {}

  SourceData::SourceData (const SourceInfo& info,
                          const string& patchName,
                          double ra, double dec)
    : itsInfo      (info),
      itsPatchName (patchName),
      itsRa        (ra),
      itsDec       (dec)
  {}
  void SourceData::setParm (const ParmMap& parms, const string& name,
                            double defValue, double& value)
  {
    ParmMap::const_iterator iter = parms.find(name);
    if (iter != parms.end()) {
      const casa::Array<double>& arr =
        iter->second.getFirstParmValue().getValues();
      ASSERTSTR (arr.size()==1, "Error: value " + name + " of source " +
                 itsInfo.getName() + " has multiple values");
      value = arr.data()[0];
    } else {
      value = defValue;
    }
  }

  void SourceData::setParms (const ParmMap& parms)
  {
    setParm (parms, "Ra", itsRa, itsRa);
    setParm (parms, "Dec", itsDec, itsDec);
    setParm (parms, "I", 0, itsI);
    setParm (parms, "Q", 0, itsQ);
    setParm (parms, "U", 0, itsU);
    setParm (parms, "V", 0, itsV);
    setParm (parms, "MajorAxis", 0, itsMajorAxis);
    setParm (parms, "MinorAxis", 0, itsMinorAxis);
    setParm (parms, "Orientation", 0, itsOrientation);
    setParm (parms, "PolarizationAngle", 0, itsPolAngle);
    setParm (parms, "PolarizedFraction", 0, itsPolFrac);
    setParm (parms, "RotationMeasure", 0, itsRM);
    itsSpInx.resize (itsInfo.getSpectralIndexNTerms());
    for (uint i=0; i<itsSpInx.size(); ++i) {
      ostringstream ostr;
      ostr << "SpectralIndex:" << i;
      setParm (parms, ostr.str(), 0, itsSpInx[i]);
    }
  }

  void SourceData::writeSource (BlobOStream& bos)
  {
    bos.putStart ("source", 1);
    itsInfo.write (bos);
    bos << itsPatchName << itsRa << itsDec << itsI << itsQ << itsU << itsV;
    if (itsInfo.getType() == SourceInfo::GAUSSIAN) {
      bos << itsMajorAxis << itsMinorAxis << itsOrientation;
    }
    if (itsInfo.getUseRotationMeasure()) {
      bos << itsPolAngle << itsPolFrac << itsRM;
    }
    if (itsInfo.getSpectralIndexNTerms() > 0) {
      bos.put (itsSpInx);
    }
    bos.putEnd();
  }

  void SourceData::readSource (BlobIStream& bis)
  {
    int version = bis.getStart ("source");
    ASSERT (version == 1);
    itsInfo.read (bis);
    bis >> itsPatchName >> itsRa >> itsDec >> itsI >> itsQ >> itsU >> itsV;
    if (itsInfo.getType() == SourceInfo::GAUSSIAN) {
      bis >> itsMajorAxis >> itsMinorAxis >> itsOrientation;
    } else {
      itsMajorAxis = itsMinorAxis = itsOrientation = 0;
    }
    if (itsInfo.getUseRotationMeasure()) {
      bis >> itsPolAngle >> itsPolFrac >> itsRM;
    } else {
      itsPolAngle = itsPolFrac = itsRM = 0;
    }
    if (itsInfo.getSpectralIndexNTerms() > 0) {
      bis.get (itsSpInx);
    } else {
      itsSpInx.resize(0);
    }
    bis.getEnd();
  }

} // namespace BBS
} // namespace LOFAR
