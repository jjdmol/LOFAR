
// Copyright notice should go here

#if !defined(UVPSPECTRUM_H)
#define UVPSPECTRUM_H

// $Id$


#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif



//! The UVPSpectrum class offers UNPROTECTED, fast access to spectral
//! data.
/*! This class will come in handy for internal data storage for
 something-versus-time-versus-frequency plots. It does NOT know
 what exactly that "something" is.
*/
class UVPSpectrum
{
#if(DEBUG_MODE)
  LocalDebugContext;            /* Common/Debug.h */
#endif


 public:

  //! Constructor.
  /*!
   */
  UVPSpectrum(unsigned int  numberOfChannels=0,
              unsigned int  rowIndex=0,
              const double* values=0);
  
  //! Copy constructor. Calls copy().
  UVPSpectrum(const UVPSpectrum &other);

  //! Destructor
  ~UVPSpectrum();

  //! Assignment operator. Calls copy().
  void operator=(const UVPSpectrum &other);

  //! copies other to itself.
  /*! Destroys its own internal data and reallocates memory to store
    the data in "other". Copies data in other.itsValues using
    copyFast() in order to redetermine its minimum and maximum.
   */
  void copy(const UVPSpectrum &other);

  
  unsigned int getRowIndex() const;

  //! \returns the number of channels stored in itsValues
  unsigned int getNumberOfChannels() const;

  
  //! \returns a const pointer to the data.
  /*!
    This pointer may be used to iterate over the data very quickly.
   */
  const double* getValues() const;

  //! \returns the maximum value.
  double max() const;
  //! |returns the minimum value.
  double min() const;

  //! Copies values to itsValues.
  /*! copyFast assumes that the valoues and itsValues arrays have equal
      length: itsNumberOfChannels.

      \param values must be at least itsNumberOfChannels large. No checks are
      performed. This file only contains a tight loop. Also performs
      min/max determination. values MAY NOT BE 0 !!!
  */
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
