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
