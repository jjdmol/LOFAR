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
  typedef float BufferPrimitive;
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

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

  BufferType* getBufferElement(int channel, int sample, int station, int polarisation);
  void setBufferElement(int channel, int sample, int station, int polarisation, BufferType* value); 

  const int         getFBW() const;

private:
  /// Forbid assignment.
  DH_CorrCube& operator= (const DH_CorrCube&);

  BufferType*  itsBuffer;    // array containing frequency spectrum.
  unsigned int itsBufSize;
  
  int          itsFBW; // number of frequency channels within this beamlet

  int nstations;
  int nchannels;
  int nsamples ;
  int npolarisations;

  void fillDataPointers();
};

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
 
 inline const int DH_CorrCube::getFBW() const
   { return itsFBW; }
 
}

#endif 
