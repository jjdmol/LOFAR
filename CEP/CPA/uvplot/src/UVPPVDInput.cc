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

#include <uvplot/UVPPVDInput.h>

#include <uvplot/UVPMessagesToFileWP.h>

#include <OCTOPUSSY/Message.h>
#include <UVD/UVD.h>

using namespace UVD;

//====================>>>  UVPPVDInput::UVPPVDInput  <<<====================

UVPPVDInput::UVPPVDInput(const std::string& filename)
  : itsNumberOfAntennae(0),
    itsNumberOfBaselines(0),
    itsNumberOfPolarizations(0),
    itsNumberOfChannels(0),
    itsDataAtom(),
    itsBOIO(filename)
{
  ObjRef oref;
  itsBOIO >> oref;
  const Message &message(dynamic_cast<const Message &>(oref.deref()));

  if(message.id().matches(UVP::IntegraterHeaderHIID)) {
    itsNumberOfBaselines = message[FNumBaselines].as_int();
    itsNumberOfAntennae = (unsigned int)((sqrt(1.0 + 8.0*message[FNumBaselines].as_int()) - 1.0 +0.01)/2.0);
    itsNumberOfChannels      = message[FNumChannels];
    itsDataAtom = UVPDataAtom(itsNumberOfChannels, UVPDataAtomHeader());
  }
}




//====================>>>  UVPPVDInput::numberOfAntennae <<<====================

unsigned int UVPPVDInput::numberOfAntennae() const
{
  return itsNumberOfAntennae;
}





//====================>>>  UVPPVDInput::numberOfChannels  <<<====================

unsigned int UVPPVDInput::numberOfChannels() const
{
  return itsNumberOfChannels;
}






//====================>>>  UVPPVDInput::getDataAtoms <<<====================

bool  UVPPVDInput::getDataAtoms(UVPDataSet* atoms,
                                unsigned int ant1,
                                unsigned int ant2)
{
  bool returnValue;

  ObjRef oref;
  if(itsBOIO >> oref) {
    
    const Message &message(dynamic_cast<const Message &>(oref.deref()));
    if(message.id().matches(UVP::IntegraterHeaderHIID)) {
      // Analyse header ??
    } else if (message.id().matches(UVP::IntegraterMessageHIID)) {
      const DataRecord &record = dynamic_cast<const DataRecord&>(message.payload().deref());
      UVPDataAtomHeader DataHeader;
      
      const std::complex<float>*  DataInRecord = record[FData].as_fcomplex_p();
      const double *              UVWPointer   = record[FUVW].as_double_p();
      const int *                 AntennaIndexPointer = record[FAntennaIndex].as_int_p();
      unsigned int                timeslot     = record[FTimeSlotIndex].as_int();
      
      
      DataHeader.itsTime            = record[FTime].as_double();
      DataHeader.itsExposureTime    = record[FExposure].as_double();
      DataHeader.itsCorrelationType = (UVPDataAtomHeader::Correlation)record[FCorr].as_int();
      DataHeader.itsFieldID         = record[FFieldIndex].as_int();
      
      for(unsigned int ifr = 0; ifr < itsNumberOfBaselines; ifr++) {
        DataHeader.itsAntenna1        = *AntennaIndexPointer++;
        DataHeader.itsAntenna2        = *AntennaIndexPointer++;
        DataHeader.itsUVW[0]          = *UVWPointer++;
        DataHeader.itsUVW[1]          = *UVWPointer++;
        DataHeader.itsUVW[2]          = *UVWPointer++;
        
        itsDataAtom.setHeader(DataHeader);
        itsDataAtom.setData(DataInRecord);
        DataInRecord += itsNumberOfChannels;
        
        if(DataHeader.itsAntenna1 == ant1 || DataHeader.itsAntenna2 == ant1) {
          (*atoms)[DataHeader] = itsDataAtom;
        }
      }

    }
    returnValue =  true;
  }  else {
    returnValue =  false;
  }// if(itsBOIO >> mref)...
  
  return returnValue;
}
