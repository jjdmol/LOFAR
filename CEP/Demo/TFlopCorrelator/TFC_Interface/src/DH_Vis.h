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
    DBGASSERTSTR(station1 >= station2, "only lower part of correlation matrix is accessible");
    return station1 * (station1 + 1) / 2 + station2;
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

  /// Test pattern methods used for regression tests
  void setStorageTestPattern();

  /// Test pattern methods used for regression tests of the correlator
  bool checkCorrelatorTestPattern();

  double getCenterFreq();

private:
  /// Forbid assignment.
  DH_Vis& operator= (const DH_Vis&);

  BufferType*  itsBuffer;    // data array 

  const double itsCenterFreq; // Subband center frequency
  
  void fillDataPointers();
};


#if 0
 inline int DH_Vis::getBufferOffset(short stationA, // row
				    short stationB, // column
				    short pol)
  {
    // Addressing: 
    // First determine the start position of the (stationA,stationB) data:
    // start at "upper left" corner with stationA=stationB=0 and
    // call this column 0, row 0. 
    // now address each row sequentially and
    // start with with column0 for the next stationA
    // Finally multiply by 4 to account for all polarisations
    //  (sA,sB) -> (sA*sA+sA)/2+sB
    //
    // This is the start address for the (stationA,stationB) data
    // add pol word to get to the requested polarisation.
    DBGASSERTSTR(stationB <= stationA,"DH_Vis::getBufferOffset: only lower part of correlation matrix is accessible");
    return (2*(stationA*stationA+stationA)+4*stationB)+pol; 
  }

 inline BufferType* DH_Vis::getBufferElement(short stationA, // row
	 					     short stationB, // column
						     short pol) {
   DBGASSERTSTR(stationB <= stationA, "DH_Vis::getBufferElement: only lower part of correlation matrix is accessible");
   return &itsBuffer[getBufferOffset(stationA, stationB, pol)];
  }
#endif


inline bool DH_Vis::checkCorrelatorTestPattern() {
  bool result = true;
  return result;
}

inline double DH_Vis::getCenterFreq() {
  return itsCenterFreq;
}
 
}

#endif 
