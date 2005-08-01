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

namespace LOFAR
{
class DH_Vis: public DataHolder
{
public:
  typedef fcomplex BufferType;

  // Constructor with centerFreq being the center frequency of the subband
  explicit DH_Vis (const string& name, double centerFreq, 
		   const LOFAR::ACC::APS::ParameterSet pSet);

  DH_Vis(const DH_Vis&);

  virtual ~DH_Vis();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Get write access to the Buffer.
  BufferType* getBuffer();

  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

  const unsigned int getBufSize() const;

  /// Test pattern methods used for regression tests
  void setStorageTestPattern();

  /// Test pattern methods used for regression tests of the correlator
  bool checkCorrelatorTestPattern();

private:
  /// Forbid assignment.
  DH_Vis& operator= (const DH_Vis&);

  ACC::APS::ParameterSet itsPS;
  BufferType*  itsBuffer;    // data array 
  unsigned int itsBufSize;

  double itsCenterFreq; // Subband center frequency
  short itsNStations; // #stations in the buffer
  short itsNBaselines;
  short itsNPols;     // #polarisations 
  short itsNFChannels;
  
  // this value is not strictly necessary, but we can 
  // check the testpattern with it.
  short itsNsamples;

  short itsNCorrs;  // #polarisations*#polarisations
  
  void fillDataPointers();
};


inline DH_Vis::BufferType* DH_Vis::getBuffer()
  { return itsBuffer; }

inline const DH_Vis::BufferType* DH_Vis::getBuffer() const
  { return itsBuffer; }

inline const unsigned int DH_Vis::getBufSize() const 
  { return itsBufSize; }  

inline bool DH_Vis::checkCorrelatorTestPattern() {
  bool result = true;
  
  for (int i = 0; i < itsNStations*itsNPols; i++) {
    for (int j = 0; j <= i; j++) {
      // this would be the correct answer for a test pattern consisting 
      // of only (1 + 1I) values
      result = result && ( *(itsBuffer+j*(itsNStations*itsNPols)+i) == 2 * itsNsamples + 0.i);
    }
  }
  return result;
}
 
}

#endif 
