// Copyright Notice

// $ID



#include <UVPDataHeader.h>



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
