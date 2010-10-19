//# DH_Vis.h: Vis DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//# $Id$

#ifndef TFLOPCORR_DH_VIS_H
#define TFLOPCORR_DH_VIS_H

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <TFC_Interface/TFC_Config.h>

namespace LOFAR
{
class DH_Vis: public DataHolder
{
public:
  typedef fcomplex BufferType[NR_BASELINES][NR_CHANNELS_PER_CORRELATOR][NR_POLARIZATIONS][NR_POLARIZATIONS];

  // Constructor with centerFreq being the center frequency of the subband
  explicit DH_Vis(const string& name, double centerFreq, 
		  const LOFAR::ACC::APS::ParameterSet pSet);

  DH_Vis(const DH_Vis&);

  virtual ~DH_Vis();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  static int baseline(int station1, int station2)
  {
    DBGASSERTSTR(station2 >= station1, "only lower part of correlation matrix is accessible");
    return station2 * (station2 + 1) / 2 + station1;
  }

  /// Get write access to the Buffer.
  BufferType* getBuffer()
  {
    return itsBuffer;
  }

  fcomplex (*getChannels(int station1, int station2)) [NR_CHANNELS_PER_CORRELATOR][NR_POLARIZATIONS][NR_POLARIZATIONS]
  {
    return &(*itsBuffer)[baseline(station1, station2)];
  }

  /// Get read access to the Buffer.
  const BufferType* getBuffer() const
  {
    return itsBuffer;
  }

  const size_t getBufSize() const
  {
    return sizeof(BufferType) / sizeof(fcomplex);
  }

  double getCenterFreq() const
  {
    return itsCenterFreq;
  }

  /// Test pattern methods used for regression tests
  void setStorageTestPattern();

  /// Test pattern methods used for regression tests of the correlator
  bool checkCorrelatorTestPattern();

private:
  /// Forbid assignment.
  DH_Vis& operator= (const DH_Vis&);

  BufferType*  itsBuffer;    // data array 

  const double itsCenterFreq; // Subband center frequency
  
  void fillDataPointers();
};
} // Namespace LOFAR

#endif 



