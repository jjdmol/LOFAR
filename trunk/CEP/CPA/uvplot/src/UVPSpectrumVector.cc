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


#include <uvplot/UVPSpectrumVector.h>

#if(DEBUG_MODE)
#include <cassert>
InitDebugContext(UVPSpectrumVector, "DEBUG_CONTEXT");
#endif




//==========>>>  UVPSpectrumVector::UVPSpectrumVector  <<<==========

UVPSpectrumVector::UVPSpectrumVector(unsigned int numberOfChannels)
  : std::vector<UVPSpectrum>(),
    itsNumberOfChannels(numberOfChannels),
    itsMinValue(0),
    itsMaxValue(0)
{
#if(DEBUG_MODE)
  TRACERPF2("");
  TRACER2("itsNumberOfChannels: " << itsNumberOfChannels);
  TRACER2("");
#endif
}






//==========>>>  UVPSpectrumVector::UVPSpectrumVector  <<<==========

void UVPSpectrumVector::add(const UVPSpectrum &spectrum)
{
#if(DEBUG_MODE)
  TRACERF2("");
  TRACER2("itsNumberOfChannels: " << itsNumberOfChannels);
  TRACER2("spectrum.getNumberOfChannels(): " << spectrum.getNumberOfChannels());
  assert(spectrum.getNumberOfChannels() == itsNumberOfChannels);
#endif

  push_back(spectrum);
  
  double Min = spectrum.min();
  double Max = spectrum.max();
  
#if(DEBUG_MODE)
  TRACER2("Min = " << Min);
  TRACER2("Max = " << Max);
#endif  

  //If this was the first entry
  if(size() == 1) {
    itsMinValue = Min;
    itsMaxValue = Max;
  } else {
    if(Min < itsMinValue) {
      itsMinValue = Min;
    }
    if(Max > itsMaxValue) {
      itsMaxValue = Max;
    }
  }
#if(DEBUG_MODE)
  TRACERF2("End.");
#endif
}




//====================>>>  UVPSpectrumVector::min  <<<====================

double UVPSpectrumVector::min() const
{
  return itsMinValue;
}






//====================>>>  UVPSpectrumVector::max  <<<====================

double UVPSpectrumVector::max() const
{
  return itsMaxValue;
}



//==============>>>  UVPSpectrumVector::getNumberOfChannels  <<<==============

unsigned int UVPSpectrumVector::getNumberOfChannels() const
{
  return itsNumberOfChannels;
}




