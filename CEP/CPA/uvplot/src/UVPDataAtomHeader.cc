// Copyright notice

#include <UVPDataAtomHeader.h>


//===============>>>  UVPDataAtomHeader::UVPDataAtomHeader <<<===============

UVPDataAtomHeader::UVPDataAtomHeader(unsigned int timeslot         = 0,
                                     unsigned int correlationIndex = 0,
                                     unsigned int ifr              = 0,
                                     unsigned int patchID          = 0,
                                     unsigned int fieldID          = 0)
  : itsFieldID(fieldID),
    itsPatchID(patchID),
    itsIFR(ifr),
    itsCorrelationIndex(correlationIndex),
    itsTimeslot(timeslot)
{
}
