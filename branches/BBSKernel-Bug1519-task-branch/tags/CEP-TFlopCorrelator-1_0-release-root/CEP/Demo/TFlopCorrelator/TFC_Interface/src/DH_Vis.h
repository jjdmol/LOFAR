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
  BufferType* getBufferElement(short station1,
			       short station2,
			       short pol); //todo: also frequency
  int getBufferOffset (short station1,
		       short station2,
		       short pol); //todo: also frequency

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

 inline DH_Vis::BufferType* DH_Vis::getBufferElement(short stationA, // row
	 					     short stationB, // column
						     short pol) {
   DBGASSERTSTR(stationB <= stationA, "DH_Vis::getBufferElement: only lower part of correlation matrix is accessible");
   return &itsBuffer[getBufferOffset(stationA, stationB, pol)];
}


inline const DH_Vis::BufferType* DH_Vis::getBuffer() const
  { return itsBuffer; }

inline const unsigned int DH_Vis::getBufSize() const 
  { return itsBufSize; }  

inline bool DH_Vis::checkCorrelatorTestPattern() {
  bool result = true;
  
  for (int p=0; p< itsNPols; p++) {
    for (int i = 0; i < itsNStations; i++) {
      for (int j = 0; j <= i; j++) {
	// this would be the correct answer for a test pattern consisting 
	// of only (1 + 1I) values
	result = result && (*getBufferElement(i,j,p) == 1+0.i );
      }
    }
  }
  return result;
}
 
}

#endif 



