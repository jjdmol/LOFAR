// Copyright notice should go here

#if !defined(UVPSPECTRUMVECTOR_H)
#define UVPSPECTRUMVECTOR_H

// $Id$

#include <uvplot/UVPSpectrum.h>
#include <vector>

#if(DEBUG_MODE)
#include <Common/Debug.h>
#endif

//! List of UVPSpectrum objects that maintains min/max values
/*! It is used in UVPTimeFrequencyPlot to represent a time-frequency
  plane with real values.
 */
class UVPSpectrumVector: public std::vector<UVPSpectrum>
{
#if(DEBUG_MODE)
  LocalDebugContext;            /* Common/Debug.h */
#endif
  
 public:

  //! Constructor
  /*! \param numberOfChannels specifies the number of channels that
    every spectrum that is added should have.
   */
  UVPSpectrumVector(unsigned int numberOfChannels=0);
  
  
  //! Add a spectrum to the vector. Calculates min/max values.
  /*! Make sure that spectrum has the same number of channels as was
    set in the constructor. In order to use the min/max feature, one
    is required to use this method instead of push_back.*/
  void add(const UVPSpectrum &spectrum);

  //! \returns the minimum value of all data points.
  double min() const;
  
  //! \returns the maximum value of all data points.
  double max() const;
  
  //! \returns the number of channels that was set in the constructor.
  unsigned int getNumberOfChannels() const;
  

  
 protected:
 private:

  unsigned int itsNumberOfChannels;
  double       itsMinValue;
  double       itsMaxValue;

};


#endif // UVPSPECTRUMVECTOR_H
