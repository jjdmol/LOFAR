//# DH_CorrCube.h: CorrCube DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef TFLOPCORRELATOR_DH_CORRCUBE_H
#define TFLOPCORRELATOR_DH_CORRCUBE_H


#include <TFC_Interface/TFC_Config.h>
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_CorrCube: public DataHolder
{
public:
  typedef fcomplex BufferType[NR_CHANNELS_PER_CORRELATOR][MAX_STATIONS_PER_PPF][NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS];
  //typedef fcomplex BufferType[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_INTEGRATION][NR_CHANNELS_PER_CORRELATOR];

  explicit DH_CorrCube (const string& name, short subband); 


  DH_CorrCube(const DH_CorrCube&);

  virtual ~DH_CorrCube();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

  void setBuffer(BufferType*);

  /// return pointer to array containing time/pol series for specified freqchannel and station 
  /// to be used in correlator inner loop
  //BufferType* getBufferTimePolSeries(int channel, int station);

  /// get/set completely specified element in the buffer
  fcomplex* getBufferElement(int channel, int station, int sample, int polarization);
  void setBufferElement(int channel, int station, int sample, int polarization, fcomplex* value); 

  unsigned int getBufSize() const;

  void setTestPattern();
  void print();

private:
  //size_t offset(int channel, int station, int sample, int polarization);

  /// Forbid assignment.
  DH_CorrCube& operator= (const DH_CorrCube&);

  BufferType*  itsBuffer;
  
  short itsSubBand;

  void fillDataPointers();
};


 inline DH_CorrCube::BufferType* DH_CorrCube::getBuffer()
   { return itsBuffer; }
 
 inline const DH_CorrCube::BufferType* DH_CorrCube::getBuffer() const
   { return itsBuffer; }
 
 inline void DH_CorrCube::setBuffer(DH_CorrCube::BufferType* buffer)
   { itsBuffer = buffer; }
   

 inline fcomplex* DH_CorrCube::getBufferElement(int channel, 
						  int station,
						  int sample,
						  int polarization)     
   { return &(*itsBuffer)[channel][station][sample][polarization]; }
   //{ return &(*itsBuffer)[station][polarization][sample][channel]; }
 
 inline void DH_CorrCube::setBufferElement(int channel, 
					   int sample, 
					   int station, 
					   int polarization,
					   fcomplex* valueptr) {
   { (*itsBuffer)[channel][station][sample][polarization] = *valueptr; }
   //{ (*itsBuffer)[station][polarization][sample][channel] = *valueptr; }
 }
 
 inline unsigned int DH_CorrCube::getBufSize() const {
   return sizeof *itsBuffer / sizeof(fcomplex);
 }

 inline void DH_CorrCube::setTestPattern() { 
   for (unsigned int i = 0; i < getBufSize(); i++) {
     ((fcomplex *) itsBuffer)[i] = makefcomplex(rand() << 20 >> 20, rand() << 20 >> 20);
   }
 }

}
#endif 
