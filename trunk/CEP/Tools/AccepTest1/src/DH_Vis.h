//# DH_Vis.h: Vis DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# $Id$

#ifndef ONLINEPROTO_DH_VIS_H
#define ONLINEPROTO_DH_VIS_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{
class DH_Vis: public DataHolder
{
public:
  typedef complex<float> BufferType;

  explicit DH_Vis (const string& name, const int stations, const int channel);

  DH_Vis(const DH_Vis&);

  virtual ~DH_Vis();

  DataHolder* clone() const;


  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;
  BufferType*       getBufferElement(int station1, int station2, int channel);
  void              addBufferElementVal(int station1, int station2, int channel, BufferType value);
  void              setBufferElement(int station1, int station2, int channel, BufferType* valueptr);
  const int         getFBW() const;


private:
  /// Forbid assignment.
  DH_Vis& operator= (const DH_Vis&);

  BufferType*  itsBuffer;    // array containing frequency spectrum.
  unsigned int itsBufSize;
  int          itsFBW; // number of frequency channels within this beamlet

  int nstations;
  int nchannels;

  void fillDataPointers();
};

inline DH_Vis::BufferType* DH_Vis::getBuffer()
  { return itsBuffer; }

inline const DH_Vis::BufferType* DH_Vis::getBuffer() const
  { return itsBuffer; }


inline DH_Vis::BufferType* DH_Vis::getBufferElement(int station1, int station2, int channel)
  { 
    return itsBuffer+station1*nstations*nchannels+station2*nchannels+channel;
  }
 
inline void DH_Vis::setBufferElement(int station1, int station2, int channel, BufferType* valueptr)
{
  *(itsBuffer+station1*nstations*nchannels+station2*nchannels+channel) = *valueptr;
};

inline void DH_Vis::addBufferElementVal(int station1, int station2, int channel, BufferType value)
{
  *(itsBuffer+station1*nstations*nchannels+station2*nchannels+channel) += value;
};

inline const int DH_Vis::getFBW() const
  { return itsFBW; }

}

#endif 
