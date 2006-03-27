//# DH_Subband.h: DataHolder for subband samples and flags
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_SUBBAND_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_SUBBAND_H

#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/RectMatrix.h>
#include <CS1_Interface/bitset.h>
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <APS/ParameterSet.h>


namespace LOFAR
{

class DH_Subband: public DataHolder
{
public:
  // Samples
#if INPUT_TYPE == I4COMPLEX_TYPE
  typedef i4complex SampleType;
#elif INPUT_TYPE == I16COMPLEX_TYPE
  typedef i16complex SampleType;
#else
#error INPUT_TYPE not supported
#endif
  typedef SampleType AllSamplesType[NR_STATIONS][NR_INPUT_SAMPLES][NR_POLARIZATIONS];

  // Flags
  typedef LOFAR::bitset<NR_INPUT_SAMPLES> AllFlagsType[NR_STATIONS];

  // Fine-grained delays
  typedef struct {
    float delayAtBegin, delayAfterEnd;
  } DelayIntervalType;
  typedef DelayIntervalType AllDelaysType[NR_STATIONS];

  explicit DH_Subband(const string &name,
		      const LOFAR::ACC::APS::ParameterSet &pSet); 

  DH_Subband(const DH_Subband &);

  virtual ~DH_Subband();

  DataHolder *clone() const;

  virtual void init();

  RectMatrix<SampleType> &getDataMatrix() const
  {
    return *itsMatrix;
  }

  // samples are ALWAYS stored in little endian format !

  AllSamplesType *getSamples()
  {
    return itsSamples;
  }

  const AllSamplesType *getSamples() const
  {
    return itsSamples;
  }

  const size_t nrSamples() const
  {
    return sizeof(AllSamplesType) / sizeof(SampleType);
  }
  
  AllFlagsType *getFlags()
  {
    return itsFlags;
  }

  const AllFlagsType *getFlags() const
  {
    return itsFlags;
  }

  AllDelaysType *getDelays()
  {
    return itsDelays;
  }

  const AllDelaysType *getDelays() const
  {
    return itsDelays;
  }

  const size_t nrDelays() const
  {
    return NR_STATIONS;
  }

  void swapBytes();

private:
  /// Forbid assignment.
  DH_Subband &operator = (const DH_Subband &);

  AllSamplesType	 *itsSamples;
  // RectMatrix cannot be used for bitsets, thus not for flags
  RectMatrix<SampleType> *itsMatrix;
  AllFlagsType		 *itsFlags;
  AllDelaysType		 *itsDelays;

  void fillDataPointers();
};

}
#endif 
