//# DH_Vis.h: Vis DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//# $Id$

#ifndef TFLOPCORR_DH_VIS_H
#define TFLOPCORR_DH_VIS_H

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>
#include <ACC/ParameterSet.h>
#include <TFC_Interface/RectMatrix.h>

namespace LOFAR
{
class DH_Vis: public DataHolder
{
public:
  typedef u16complex BufferType;

  explicit DH_Vis (const string& name, short startfreq, const LOFAR::ACC::APS::ParameterSet pSet);

  DH_Vis(const DH_Vis&);

  virtual ~DH_Vis();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

  const unsigned int getBufSize() const;

  RectMatrix<BufferType>& getDataMatrix() const;

private:
  /// Forbid assignment.
  DH_Vis& operator= (const DH_Vis&);

  ACC::APS::ParameterSet itsPS;
  BufferType*  itsBuffer;    // data array 
  unsigned int itsBufSize;

  short itsStartFreq; // first freq channel ID
  short itsNStations; // #stations in the buffer
  short itsNBaselines;
  short itsNPols;     // #polarisations 
  short itsNFChannels;

  RectMatrix<BufferType>* itsMatrix;

  void fillDataPointers();
};


inline DH_Vis::BufferType* DH_Vis::getBuffer()
  { return itsBuffer; }
 
inline const DH_Vis::BufferType* DH_Vis::getBuffer() const
  { return itsBuffer; }

inline const unsigned int DH_Vis::getBufSize() const 
  { return itsBufSize; }  

inline RectMatrix<DH_Vis::BufferType>& DH_Vis::getDataMatrix() const 
  { return *itsMatrix; }

}

#endif 
