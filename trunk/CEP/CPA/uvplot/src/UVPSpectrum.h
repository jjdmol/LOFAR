
// Copyright notice should go here

// $ID$

#if !defined(UVPSPECTRUM_H)
#define UVPSPECTRUM_H


// The UVPSpectrum class offers UNPROTECTED, fast access to spectral
// data. This class will come in handy for internal data storage for
// something-versus-time-versus-frequency plots. It does NOT maintain
// what "something" is.
class UVPSpectrum
{
 public:

  // Constructor. numberOfChannels must always be defined.
  UVPSpectrum(unsigned int  numberOfChannels,
              unsigned int  rowIndex=0,
              const double* values=0);
  
  UVPSpectrum(const UVPSpectrum &other);

  ~UVPSpectrum();

  void operator=(const UVPSpectrum &other);

  void copy(const UVPSpectrum &other);

  
  unsigned int getRowIndex() const;
  
  unsigned int getNumberOfChannels() const;
  
  const double* getValues() const;

  // Values must be at least itsNumberOfChannels large. No checks are
  // performed. This file only contains a tight loop.
  void copyFast(const double *values);
  
 protected:
 private:

  unsigned int itsRowIndex;
  unsigned int itsNumberOfChannels;

  double*        itsValues;
};



#endif // UVPSPECTRUM_H
