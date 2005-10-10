//# DH_PPF.h: PPF DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#

#ifndef TFLOPCORRELATOR_DH_PPF_H
#define TFLOPCORRELATOR_DH_PPF_H

#include <TFC_Interface/TFC_Config.h>
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <APS/ParameterSet.h>
#include <stdlib.h>


namespace LOFAR
{

class DH_PPF: public DataHolder
{
public:
  typedef i16complex BufferType[NR_STATIONS][NR_STATION_SAMPLES][NR_POLARIZATIONS];

  explicit DH_PPF(const string& name,
		  const short   subband,
		  const LOFAR::ACC::APS::ParameterSet pSet); 


  DH_PPF(const DH_PPF&);

  virtual ~DH_PPF();

  DataHolder *clone() const;

  virtual void init();

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
    return sizeof(BufferType) / sizeof(i16complex);
  }
  
  void setTestPattern()
  {
    (std::cerr << "DH_PPF::setTestPattern() ... ").flush();
    for (size_t i = 0; i < getBufferSize(); i++) {
      ((i16complex *) itsBuffer)[i] = makei16complex(rand() << 20 >> 20, rand() << 20 >> 20);
    }
    std::cerr << "done.\n";
  }

private:
  /// Forbid assignment.
  DH_PPF &operator = (const DH_PPF&);

  BufferType *itsBuffer;

  ACC::APS::ParameterSet itsPS;

  void fillDataPointers();
};


}
#endif 
