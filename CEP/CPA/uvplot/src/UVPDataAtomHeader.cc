//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <uvplot/UVPDataAtomHeader.h>





//===============>>>  UVPDataAtomHeader::UVPDataAtomHeader <<<===============

UVPDataAtomHeader::UVPDataAtomHeader(unsigned int antenna1,
                                     unsigned int antenna2,
                                     double       time,
                                     float        exposureTime,
                                     Correlation  correlationType,
                                     unsigned int fieldID,
                                     unsigned int spectralWindowID,
                                     const std::vector<double>& uvw,
                                     DataType     dataType)
  : itsAntenna1(antenna1),
    itsAntenna2(antenna2),
    itsTime(time),
    itsExposureTime(exposureTime),
    itsCorrelationType(correlationType),
    itsSpectralWindowID(spectralWindowID),
    itsFieldID(fieldID),
    itsDataType(dataType)
{
    itsUVW[0] = uvw[0];
    itsUVW[1] = uvw[1];
    itsUVW[2] = uvw[2];
}







//===============>>>  UVPDataAtomHeader::operator < <<<===============

bool UVPDataAtomHeader::operator < (const UVPDataAtomHeader &other) const
{
  if(itsAntenna1 < other.itsAntenna1) {
    return true;
  } else if(itsAntenna1 == other.itsAntenna1) {
    if(itsAntenna2 < other.itsAntenna2) {
      return true;
    } else if(itsAntenna2 == other.itsAntenna2) {
      if(itsSpectralWindowID < other.itsSpectralWindowID) {
        return true;
      } else if(itsSpectralWindowID == other.itsSpectralWindowID) {
        if(itsCorrelationType < other.itsCorrelationType) {
          return true;
        } else if(itsCorrelationType == other.itsCorrelationType) {
          if(itsTime < other.itsTime) {
            return true;
          } else if(itsTime == other.itsTime) {
            return false;
          } // itsTime
        } // itsCorrelationType
      } // itsSpectralWindowID
    } // itsAntenna2
  } // itsAntenna1
  
  // If we're still in this function by now, we are larger than or equal to the other
  return false;
}






//====================>>>  UVPDataAtomHeader::store  <<<====================

void UVPDataAtomHeader::store(std::ostream& out) const
{
  out.write((const char*)&itsAntenna1    , sizeof(unsigned int));
  out.write((const char*)&itsAntenna2    , sizeof(unsigned int));
  out.write((const char*)&itsTime        , sizeof(double));
  out.write((const char*)&itsExposureTime, sizeof(float));
  out.write((const char*)&itsSpectralWindowID, sizeof(unsigned int));
  out.write((const char*)&itsFieldID     , sizeof(unsigned int));
  out.write((const char*)&itsUVW         , 3*sizeof(double));
  out.write((const char*)&itsCorrelationType, 1);
  out.write((const char*)&itsDataType       , 1);
}




//====================>>>  UVPDataAtomHeader::load  <<<====================

void UVPDataAtomHeader::load(std::istream& in)
{
  in.read((char*)&itsAntenna1    , sizeof(unsigned int));
  in.read((char*)&itsAntenna2    , sizeof(unsigned int));
  in.read((char*)&itsTime        , sizeof(double));
  in.read((char*)&itsExposureTime, sizeof(float));
  in.read((char*)&itsSpectralWindowID, sizeof(unsigned int));
  in.read((char*)&itsFieldID     , sizeof(unsigned int));
  in.read((char*)&itsUVW         , 3*sizeof(double));
  in.read((char*)&itsCorrelationType, 1);
  in.read((char*)&itsDataType       , 1);
}
