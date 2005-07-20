//# DH_FIR.h: FIR DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef TFLOPCORRELATOR_DH_FIR_H
#define TFLOPCORRELATOR_DH_FIR_H

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <TFC_Interface/RectMatrix.h>
#include <APS/ParameterSet.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_FIR: public DataHolder
{
public:
  typedef u16complex BufferType;

  explicit DH_FIR (const string& name,
		   const short   subband,
		   const LOFAR::ACC::APS::ParameterSet pSet); 


  DH_FIR(const DH_FIR&);

  virtual ~DH_FIR();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Get write access to the Buffer.
  u16complex* getBuffer();
  /// Get access to the Buffer.
  const u16complex* getBuffer() const;

  const unsigned int getBufferSize() const;

   RectMatrix<BufferType>& getDataMatrix() const;

private:
  /// Forbid assignment.
  DH_FIR& operator= (const DH_FIR&);

  BufferType*  itsBuffer;    // 
  unsigned int itsBufSize;
  
  short itsFIR;
  short itsNStations;      // #stations per buffer
  short itsNTimes;         // #time samples per buffer
  short itsNPol;           // #polarisations per sample

  RectMatrix<BufferType>* itsMatrix;

  ACC::APS::ParameterSet itsPS;

  void fillDataPointers();
};

 inline DH_FIR::BufferType* DH_FIR::getBuffer()
   { return itsBuffer; }
 
 inline const DH_FIR::BufferType* DH_FIR::getBuffer() const
   { return itsBuffer; }
 
 inline const unsigned int DH_FIR::getBufferSize() const {
   return itsBufSize;
 }

 inline RectMatrix<DH_FIR::BufferType>& DH_FIR::getDataMatrix() const {
   return *itsMatrix; 
 };
 
}
#endif 
