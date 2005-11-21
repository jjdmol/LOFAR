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
#include <TFC_Interface/RectMatrix.h>
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <APS/ParameterSet.h>
#include <stdlib.h>


namespace LOFAR
{

class DH_PPF: public DataHolder
{
public:
  typedef i16complex BufferElementType;
  typedef BufferElementType BufferType[MAX_STATIONS_PER_PPF][NR_STATION_SAMPLES][NR_POLARIZATIONS];

  explicit DH_PPF(const string& name,
		  const short   subband,
		  const LOFAR::ACC::APS::ParameterSet pSet); 


  DH_PPF(const DH_PPF&);

  virtual ~DH_PPF();

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

  const BufferElementType *getBufferElement() const 
  {
    return itsBuffer[0][0][0];
  }

  const size_t getBufferSize() const
  {
    return sizeof(BufferType) / sizeof(BufferElementType);
  }
  
  void setTestPattern()
  {
    (std::cerr << "DH_PPF::setTestPattern() ... ").flush();

#if 0
    for (int stat = 0; stat < MAX_STATIONS_PER_PPF; stat ++) {
      for (int sample = 0; sample < NR_STATION_SAMPLES; sample ++) {
	for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	  float phi = 3 * 2 * 3.14159265358979323846f * sample / NR_STATION_SAMPLES + stat + 1.5707963267948966192f * pol;
	  float s, c;
	  sincosf(phi, &s, &c);

	  (*itsBuffer)[stat][sample][pol] = makei16complex((int) (32767*s), (int) (32767*c));
	}
      }
    }
#else
    for (size_t i = 0; i < getBufferSize(); i++) {
      ((BufferElementType *) itsBuffer)[i] = makei16complex(rand() << 20 >> 20, rand() << 20 >> 20);
    }
#endif

    std::cerr << "done.\n";
  }

private:
  /// Forbid assignment.
  DH_PPF &operator = (const DH_PPF&);

  ACC::APS::ParameterSet itsPS;
  RectMatrix<BufferElementType>* itsMatrix;
  BufferType *itsBuffer;

  void fillDataPointers();
};


}
#endif 
