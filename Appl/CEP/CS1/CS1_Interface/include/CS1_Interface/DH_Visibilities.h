//# DH_Visibilities.h: Visibilities DataHolder
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_VISIBILITIES_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_DH_VISIBILITIES_H

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <CS1_Interface/CS1_Config.h>

namespace LOFAR
{
class DH_Visibilities: public DataHolder
{
public:
  typedef fcomplex VisibilitiesType[NR_BASELINES][NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS];

#if NR_SAMPLES_PER_INTEGRATION < 256
  typedef unsigned char CountType;
#elif NR_SAMPLES_PER_INTEGRATION < 65536
  typedef unsigned short CountType;
#else
  typedef unsigned CountType;
#endif

  typedef CountType NrValidSamplesType[NR_BASELINES][NR_SUBBAND_CHANNELS];

  // Constructor with centerFreq being the center frequency of the subband
  explicit DH_Visibilities(const string& name,
			   const LOFAR::ACC::APS::ParameterSet &pSet);

  DH_Visibilities(const DH_Visibilities&);

  virtual ~DH_Visibilities();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  static int baseline(int station1, int station2)
  {
    DBGASSERTSTR(station2 >= station1, "only lower part of correlation matrix is accessible");
    return station2 * (station2 + 1) / 2 + station1;
  }

  /// Get write access to the Visibilities.
  VisibilitiesType* getVisibilities()
  {
    return itsVisibilities;
  }

  fcomplex (*getChannels(int station1, int station2)) [NR_SUBBAND_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS]
  {
    return &(*itsVisibilities)[baseline(station1, station2)];
  }

  /// Get read access to the Visibilities.
  const VisibilitiesType* getVisibilities() const
  {
    return itsVisibilities;
  }

  const NrValidSamplesType *getNrValidSamplesCounted() const
  {
    return itsNrValidSamplesCounted;
  }

  NrValidSamplesType *getNrValidSamplesCounted()
  {
    return itsNrValidSamplesCounted;
  }

  const size_t getBufSize() const
  {
    return sizeof(VisibilitiesType) / sizeof(fcomplex);
  }

  /// Test pattern methods used for regression tests of the correlator
  void checkCorrelatorTestPattern();

private:
  /// Forbid assignment.
  DH_Visibilities& operator= (const DH_Visibilities&);

  VisibilitiesType   *itsVisibilities;
  NrValidSamplesType *itsNrValidSamplesCounted;

  void fillDataPointers();
};
} // Namespace LOFAR

#endif 



