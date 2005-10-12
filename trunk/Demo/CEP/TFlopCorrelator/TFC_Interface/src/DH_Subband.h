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
  typedef i16complex BufferElementType;
  typedef BufferElementType BufferType[NR_STATIONS][NR_STATION_SAMPLES][NR_POLARIZATIONS];

  explicit DH_Subband(const string& name,
		  const short   subband,
		  const LOFAR::ACC::APS::ParameterSet pSet); 


  DH_Subband(const DH_Subband&);

  virtual ~DH_Subband();

  DataHolder *clone() const;

  virtual void init();

  RectMatrix<BufferElementType>& getDataMatrix() const
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
    return sizeof(BufferType) / sizeof(BufferElementType);
  }
  
  void setTestPattern()
  {
    (std::cerr << "DH_Subband::setTestPattern() ... ").flush();
    for (size_t i = 0; i < getBufferSize(); i++) {
      ((BufferElementType *) itsBuffer)[i] = makei16complex(rand() << 20 >> 20, rand() << 20 >> 20);
    }
    std::cerr << "done.\n";
  }

private:
  /// Forbid assignment.
  DH_Subband &operator = (const DH_Subband&);

  ACC::APS::ParameterSet itsPS;
  RectMatrix<BufferElementType>* itsMatrix;
  BufferType *itsBuffer;

  void fillDataPointers();
};


}
#endif 
