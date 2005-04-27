//# DH_Vis.h: Vis DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
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
  typedef float BufferPrimitive;
  typedef fcomplex BufferType;

  explicit DH_Vis (const string& name, short startfreq);

  DH_Vis(const DH_Vis&);

  virtual ~DH_Vis();

  DataHolder* clone() const;


  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  fcomplex* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const fcomplex* getBuffer() const;

  fcomplex* getBufferElement(int station1, int station2, int channel, int polarisation);
  void      setBufferElement(int station1, int station2, int channel, int polarisation, fcomplex* valueptr);

  const unsigned int getBufSize() const;

private:
  /// Forbid assignment.
  DH_Vis& operator= (const DH_Vis&);

  fcomplex*  itsBuffer;    // data array 
  unsigned int itsBufSize;

  short itsStartFreq; // first freq channel ID
  short itsNStations; // #stations in the buffer 
  short itsNPols;     // #polarisations 
  short itsNFChannels;

  void fillDataPointers();
};


inline fcomplex* DH_Vis::getBuffer()
  { return itsBuffer; }
 
inline const fcomplex* DH_Vis::getBuffer() const
  { return itsBuffer; }

#define VISADDRESS_FREQ(freq) itsNFChannels*(freq)    
#define VISADDRESS_BASELINE(freq, station1, station2)  VISADDRESS_FREQ((freq)) + 
#define VISADDRESS_POL(freq, station1, station2, pol) VISADDRESS_BASELINE((freq),(station1),(station2)) + itsNPols*pol

inline fcomplex* DH_Vis::getBufferElement(int station1, int station2, int channel, int polarisation)
  { 
    return itsBuffer + VISADDRESS_POL(station1, station2, channel, polarisation);
  }
 
inline void DH_Vis::setBufferElement(int station1, int station2, int channel, int polarisation, fcomplex* valueptr)
{
  *( itsBuffer + VISADDRESS_POL(station1, station2, channel, polarisation)  ) = *valueptr;
};


inline const unsigned int DH_Vis::getBufSize() const 
  { return itsBufSize; }  

}

#endif 
