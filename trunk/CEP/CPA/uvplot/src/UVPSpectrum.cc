// Copyright notice should go here

// $ID$

#include <UVPSpectrum.h>

#if(DEBUG_MODE)
#include <cassert>
InitDebugContext(UVPSpectrum, "DEBUG_CONTEXT");
#endif



//====================>>>  UVPSpectrum::UVPSpectrum  <<<====================

UVPSpectrum::UVPSpectrum(unsigned int numberOfChannels,
                         unsigned int rowIndex,
                         const double *values)
  : itsNumberOfChannels(numberOfChannels),
    itsRowIndex(rowIndex),
    itsValues(0),
    itsMinValue(0),
    itsMaxValue(0)
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
  : itsNumberOfChannels(0),
    itsRowIndex(0),
    itsValues(0),
    itsMinValue(0),
    itsMaxValue(0)
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
#if(DEBUG_MODE)
  TRACERF2("");
#endif
  
  if(itsValues != 0) {
    delete[] itsValues;
    itsValues = 0;
  }
  
  itsNumberOfChannels = other.itsNumberOfChannels;
  itsRowIndex         = other.itsRowIndex;
  itsValues           = new double[itsNumberOfChannels];
  
  copyFast(other.itsValues);

#if(DEBUG_MODE)
  TRACERF2("End.");
#endif
}












///====================>>>  UVPSpectrum::copyFast  <<<====================

void UVPSpectrum::copyFast(const double *values)
{
  register unsigned int N(itsNumberOfChannels);
  
  if(N > 0) {
    //    register unsigned int i(0);
    register double tempMin(*values);
    register double tempMax(*values);
    register double temp(0);
    
    register double*       target = itsValues;
    register const double* source = values;
    register const double* end    = source + N;

    while(source != end){
      
      //      itsValues[i] = values[i];    
      //      temp = values[i];
  
      temp      = *source;
      *target++ = *source++;
      tempMin = (temp < tempMin ? temp : tempMin);
      tempMax = (temp > tempMax ? temp : tempMax);
      //      i++;
      //      std::cout << temp << std::endl;
    }

    itsMinValue = tempMin;
    itsMaxValue = tempMax;
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



//====================>>>  UVPSpectrum::min  <<<====================

double UVPSpectrum::min() const
{
  return itsMinValue;
}




//====================>>>  UVPSpectrum::max  <<<====================

double UVPSpectrum::max() const
{
  return itsMaxValue;
}
