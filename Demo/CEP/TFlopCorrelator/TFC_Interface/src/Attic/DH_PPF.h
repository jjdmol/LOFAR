//# DH_PPF.h: SubBand DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef TFLOPCORRELATOR_DH_PPF_H
#define TFLOPCORRELATOR_DH_PPF_H

#include <TFC_Interface/RectMatrix.h>
#include <TFC_Interface/DH_FIR.h>
#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_PPF: public DataHolder
{
public:
  typedef u16complex BufferType;

  explicit DH_PPF (const string& name,
		   const short   subband); 


  DH_PPF(const DH_PPF&);

  virtual ~DH_PPF();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  void InitBankCursor(short station, short pol, short time);
  //void InitTimeCursor(short station, short pol);
  void setNextBank(BufferType &value);
  //BufferType** getnextTime();

  void InitTimeCursor(short station, short pol, short time);
  void setNextBank(BufferType value);

private:
  /// Forbid assignment.
  DH_PPF& operator= (const DH_PPF&);

  BufferType*  itsBuffer; 
  unsigned int itsBufSize;
  unsigned int itsNSamples;
  unsigned int itsNStations;
  unsigned int itsNTimes;
  unsigned int itsNPol;
  unsigned int itsNFilters;

  RectMatrix<BufferType>* itsMatrix;

  // attributes needed to access the RectMatrix
  dimType itsStationDim;
  dimType itsPolDim;
  dimType itsTimeDim;
  dimType itsBankDim;
  RectMatrix<DH_FIR::BufferType>::cursorType itsBankCursor;

  void fillDataPointers();
};

   
}
#endif 
