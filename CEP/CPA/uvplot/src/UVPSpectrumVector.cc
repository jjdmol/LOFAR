// Copyright notice should go here

// $ID$


#include <UVPSpectrumVector.h>

#if(DEBUG_MODE)
#include <cassert>
#endif

#if(DEBUG_MODE)
InitDebugContext(UVPSpectrumVector, "DEBUG_CONTEXT");
#endif



//==========>>>  UVPSpectrumVector::UVPSpectrumVector  <<<==========

UVPSpectrumVector::UVPSpectrumVector(unsigned int numberOfChannels)
  : std::vector<UVPSpectrum>(),
    itsNumberOfChannels(numberOfChannels),
    itsMinValue(0),
    itsMaxValue(0)
{
}






//==========>>>  UVPSpectrumVector::UVPSpectrumVector  <<<==========

void UVPSpectrumVector::add(const UVPSpectrum &spectrum)
{
#if(DEBUG_MODE)
  TRACER1("UVPSpectrumVector::add");
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




