// Copyright Notice

// $ID



#include <uvplot/UVPDataHeader.h>



//==================>>>  UVPDataHeader::UVPDataHeader  <<<==================

UVPDataHeader::UVPDataHeader(int                  correlation,
                             int                  numberOfBaselines,
                             int                  numberOfTimeslots,
                             int                  numberOfChannels,
                             int                  fieldID,
                             const std::string&  fieldName)
  : itsCorrelation(correlation),
    itsNumberOfBaselines(numberOfBaselines),
    itsNumberOfTimeslots(numberOfTimeslots),
    itsNumberOfChannels(numberOfChannels),
    itsFieldID(fieldID),
    itsFieldName(fieldName)
{
}





//====================>>>  operator << <<<====================

std::ostream &operator <<(std::ostream &out, const UVPDataHeader &header)
{
  out << header.itsFieldName << ": #"
      << header.itsFieldID << ", Channels = "
      << header.itsNumberOfChannels << ", Timeslots = " 
      << header.itsNumberOfTimeslots << ", Baselines = "
      << header.itsNumberOfBaselines;

  return out;
}
