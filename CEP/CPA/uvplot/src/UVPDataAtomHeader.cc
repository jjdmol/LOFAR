// Copyright notice

#include <uvplot/UVPDataAtomHeader.h>





//===============>>>  UVPDataAtomHeader::UVPDataAtomHeader <<<===============

UVPDataAtomHeader::UVPDataAtomHeader(unsigned int antenna1,
                                     unsigned int antenna2,
                                     double       time,
                                     float        exposureTime,
                                     Correlation  correlationType,
                                     unsigned int fieldID,
                                     const std::vector<double>& uvw,
                                     DataType     dataType)
  : itsAntenna1(antenna1),
    itsAntenna2(antenna2),
    itsTime(time),
    itsExposureTime(exposureTime),
    itsCorrelationType(correlationType),
    itsFieldID(fieldID),
    itsDataType(dataType)
{
    itsUVW[0] = uvw[0];
    itsUVW[1] = uvw[1];
    itsUVW[2] = uvw[2];

    sortAntennae();
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
      if(itsCorrelationType < other.itsCorrelationType) {
        return true;
      } else if(itsCorrelationType == other.itsCorrelationType) {
        if(itsTime < other.itsTime) {
          return true;
        } else if(itsTime == other.itsTime) {
          return false;
        } // itsTime
      } // itsCorrelationType
    } // itsAntenna2
  } // itsAntenna1
  
  // If we're still in this function by now, we are larger than or equal to the other
  return false;
}




//===============>>>  UVPDataAtomHeader::sortAntennae  <<<===============

void UVPDataAtomHeader::sortAntennae()
{
  if(itsAntenna1 > itsAntenna2) {
    unsigned int tmp = itsAntenna1;
    itsAntenna1 = itsAntenna2;
    itsAntenna2 = tmp;
  }
}




//====================>>>  UVPDataAtomHeader::store  <<<====================

void UVPDataAtomHeader::store(std::ostream& out) const
{
  out.write((const void*)&itsAntenna1    , sizeof(unsigned int));
  out.write((const void*)&itsAntenna2    , sizeof(unsigned int));
  out.write((const void*)&itsTime        , sizeof(double));
  out.write((const void*)&itsExposureTime, sizeof(float));
  out.write((const void*)&itsFieldID     , sizeof(unsigned int));
  out.write((const void*)&itsUVW         , 3*sizeof(double));
  out.write(& (unsigned char)itsCorrelationType, 1);
  out.write(& (unsigned char)itsDataType       , 1);
}




//====================>>>  UVPDataAtomHeader::load  <<<====================

void UVPDataAtomHeader::load(std::istream& in)
{
  in.read((void*)&itsAntenna1    , sizeof(unsigned int));
  in.read((void*)&itsAntenna2    , sizeof(unsigned int));
  in.read((void*)&itsTime        , sizeof(double));
  in.read((void*)&itsExposureTime, sizeof(float));
  in.read((void*)&itsFieldID     , sizeof(unsigned int));
  in.read((void*)&itsUVW         , 3*sizeof(double));
  in.read(& (unsigned char)itsCorrelationType, 1);
  in.read(& (unsigned char)itsDataType       , 1);
}
