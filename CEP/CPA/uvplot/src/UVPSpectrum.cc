// Copyright notice should go here

// $ID$

#include <UVPSpectrum.h>


//====================>>>  UVPSpectrum::UVPSpectrum  <<<====================

UVPSpectrum::UVPSpectrum(unsigned int numberOfChannels,
                         unsigned int rowIndex,
                         const double *values)
  : itsNumberOfChannels(numberOfChannels),
    itsRowIndex(rowIndex),
    itsValues(0)
{
  if(itsNumberOfChannels > 0) {
    itsValues = new double[itsNumberOfChannels];
  }
  
  if(values != 0) {
    copyFast(values);
  }
}





//====================>>>  UVPSPectrum::UVPSpectrum  <<<====================

UVPSpectrum::UVPSpectrum(const UVPSpectrum &other)
{
  copy(other);
}





//====================>>>  UVPSpectrum::~UVPSpectrum  <<<====================

UVPSpectrum::~UVPSpectrum()
{
  if(itsValues != 0)
    {
      delete[] itsValues;
    }
}




//====================>>>  UVPSPectrum::operator =  <<<====================

void UVPSpectrum::operator =(const UVPSpectrum &other)
{
  copy(other);
}





//====================>>>  UVPSPectrum::copy  <<<====================

void UVPSpectrum::copy(const UVPSpectrum &other)
{
  if(itsValues != 0) {
    delete[] itsValues;
    itsValues = 0;
  }
  
  itsNumberOfChannels = other.itsNumberOfChannels;
  itsRowIndex         = other.itsRowIndex;
  itsValues           = new double[itsNumberOfChannels];
  
  copyFast(other.itsValues);
}












///====================>>>  UVPSpectrum::copyFast  <<<====================

void UVPSpectrum::copyFast(const double *values)
{
  register unsigned int i(0);
  register unsigned int N(itsNumberOfChannels);
  while( i < N){
    *itsValues++ = *values++;
    i++;
  }
}




//====================>>>  UVPSpectrum::getRowIndex  <<<====================

unsigned int UVPSpectrum::getRowIndex() const
{
  return itsRowIndex;
}





//=================>>>  UVPSpectrum::getNumberOfChannels  <<<=================

unsigned int UVPSpectrum::getNumberOfChannels() const
{
  return itsNumberOfChannels;
}





//====================>>>  UVPSpectrum::getValues  <<<====================

const double* UVPSpectrum::getValues() const
{
  return itsValues;
}
