//# DH_Subband.h: Subband DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#

#ifndef TFLOPCORRELATOR_DH_SUBBAND_H
#define TFLOPCORRELATOR_DH_SUBBAND_H

#include <TFC_Interface/TFC_Config.h>
#include <TFC_Interface/RectMatrix.h>
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <APS/ParameterSet.h>
#include <stdlib.h>


namespace LOFAR
{

class DH_Subband: public DataHolder
{
public:
  typedef i16complex BufferType;

  explicit DH_Subband(const string& name,
		  const short   subband,
		  const LOFAR::ACC::APS::ParameterSet pSet); 


  DH_Subband(const DH_Subband&);

  virtual ~DH_Subband();

  DataHolder *clone() const;

  virtual void init();

  RectMatrix<BufferType>& getDataMatrix() const
  {
    return *itsMatrix; 
  }

  BufferType *getBuffer()
  {
    return itsBuffer;
  }

  const BufferType *getBuffer() const
  {
    return itsBuffer;
  }

  const size_t getBufferSize() const
  {
    return itsBufferSize;
  }
  
private:
  /// Forbid assignment.
  DH_Subband &operator = (const DH_Subband&);

  ACC::APS::ParameterSet itsPS;
  RectMatrix<BufferType>* itsMatrix;
  BufferType *itsBuffer;
  int itsBufferSize;

  void fillDataPointers();
};


}
#endif 
