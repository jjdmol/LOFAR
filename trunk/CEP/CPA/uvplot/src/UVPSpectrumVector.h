// Copyright notice should go here

// $ID$

#if !defined(UVPSPECTRUMVECTOR_H)
#define UVPSPECTRUMVECTOR_H

#include <UVPSpectrum.h>
#include <vector>

#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif

class UVPSpectrumVector: public std::vector<UVPSpectrum>
{
#if(DEBUG_MODE)
  LocalDebugContext;            /* Common/Debug.h */
#endif
  
 public:

  UVPSpectrumVector(unsigned int numberOfChannels);
  
  
  void add(const UVPSpectrum &spectrum);

  double min() const;
  double max() const;
  
  unsigned int getNumberOfChannels() const;
  

  
 protected:
 private:

  unsigned int     itsNumberOfChannels;
  double itsMinValue;
  double itsMaxValue;

};


#endif // UVPSPECTRUMVECTOR_H
