//# DH_PPF.h: SubBand DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef TFLOPCORRELATOR_DH_PPF_H
#define TFLOPCORRELATOR_DH_PPF_H

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_PPF: public DataHolder
{
public:
  typedef u16complex BufferType;

  explicit DH_PPF (const string& name,
		   const short   subband); 


  DH_PPF(const DH_PPF&);

  virtual ~DH_PPF();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// get/set completely specified element in the buffer
  BufferType* setBufferElement(int sample, BufferType value);

private:
  /// Forbid assignment.
  DH_PPF& operator= (const DH_PPF&);

  BufferType*  itsBuffer; 
  unsigned int itsBufSize;
  unsigned int itsNSamples;

  void fillDataPointers();
};

 inline DH_PPF::BufferType* DH_PPF::setBufferElement(int sample, DH_PPF::BufferType value) {
   itsBuffer[sample] = value;
 }
   
}
#endif 
