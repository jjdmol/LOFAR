
// Copyright notice should go here

// $ID$

#if !defined(UVPSPECTRUM_H)
#define UVPSPECTRUM_H


#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif



// The UVPSpectrum class offers UNPROTECTED, fast access to spectral
// data. This class will come in handy for internal data storage for
// something-versus-time-versus-frequency plots. It does NOT maintain
// what "something" is.
class UVPSpectrum
{
#if(DEBUG_MODE)
  LocalDebugContext;            /* Common/Debug.h */
#endif


 public:

  // Constructor. numberOfChannels must always be defined.
  UVPSpectrum(unsigned int  numberOfChannels=0,
              unsigned int  rowIndex=0,
              const double* values=0);
  
  UVPSpectrum(const UVPSpectrum &other);

  ~UVPSpectrum();

  void operator=(const UVPSpectrum &other);

  void copy(const UVPSpectrum &other);

  
  unsigned int getRowIndex() const;
  
  unsigned int getNumberOfChannels() const;
  
  const double* getValues() const;

  
  double max() const;
  double min() const;

  // Values must be at least itsNumberOfChannels large. No checks are
  // performed. This file only contains a tight loop. Also performs
  // min/max determination.
  void copyFast(const double *values);
  
 protected:
 private:

  unsigned int itsNumberOfChannels;
  unsigned int itsRowIndex;

  double*        itsValues;
  
  double itsMinValue;
  double itsMaxValue;
};



#endif // UVPSPECTRUM_H
