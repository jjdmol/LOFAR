//# DH_CorrCube.h: CorrCube DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef ONLINEPROTO_DH_ARRAYTFPLANE_H
#define ONLINEPROTO_DH_ARRAYTFPLANE_H


#include <lofar_config.h>

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
  typedef uint16 BufferPrimitive;
  typedef complex<BufferPrimitive> BufferType;

  explicit DH_CorrCube (const string& name, 
			const int stations,
			const int samples, 
			const int channels,
			const int polarisations); 

  DH_CorrCube(const DH_CorrCube&);

  virtual ~DH_CorrCube();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// accessor functions to the blob 
  int getStationID() const;
  void setStationID(int);
  int getFlag() const;
  void setFlag(int);

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

  BufferType* getBufferElement(int channel, int sample, int station, int polarisation);
  void setBufferElement(int channel, int sample, int station, int polarisation, BufferType* value); 

  const unsigned int getBufSize() const;

private:
  /// Forbid assignment.
  DH_CorrCube& operator= (const DH_CorrCube&);

  /// pointers to data in the blob
  int* itsFlagptr;
  BufferType*  itsBuffer;    // array containing frequency spectrum.
  unsigned int itsBufSize;
  
  int nstations;
  int nchannels;
  int nsamples ;
  int npolarisations;

  void fillDataPointers();
};


inline int DH_CorrCube::getFlag() const
  { return *itsFlagptr; }

inline void DH_CorrCube::setFlag(int flag)
  { *itsFlagptr = flag; }

inline DH_CorrCube::BufferType* DH_CorrCube::getBuffer()
  { return itsBuffer; }

inline const DH_CorrCube::BufferType* DH_CorrCube::getBuffer() const
  { return itsBuffer; }

inline DH_CorrCube::BufferType* DH_CorrCube::getBufferElement(int channel, 
							      int sample,
							      int station,
							      int polarisation							     
							      ) 
  {
    return itsBuffer + 
      npolarisations*nstations*nsamples*channel + 
      npolarisations*nstations*sample + 
      npolarisations*station + 
      polarisation;
  }
 
 inline void DH_CorrCube::setBufferElement(int channel, 
					   int sample, 
					   int station, 
					   int polarisation,
					   DH_CorrCube::BufferType* valueptr) {
   *(itsBuffer + 
     npolarisations*nstations*nsamples*channel + 
     npolarisations*nstations*sample + 
     npolarisations*station + 
     polarisation) = *valueptr;
 }
 
 inline const unsigned int DH_CorrCube::getBufSize() const {
   return itsBufSize;
 }
 
}

#endif 
