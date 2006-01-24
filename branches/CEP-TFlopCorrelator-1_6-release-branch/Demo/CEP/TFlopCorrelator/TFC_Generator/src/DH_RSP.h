//# DH_RSP.h: DataHolder storing RSP raw ethernet frames for 
//#           StationCorrelator demo
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//# $Id$
//#
/////////////////////////////////////////////////////////////////

#ifndef TFLOPCORRELATOR_DH_RSP_H
#define TFLOPCORRELATOR_DH_RSP_H


#include <APS/ParameterSet.h>
#include <Transport/DataHolder.h>
#include <TFC_Interface/RSPTimeStamp.h>
#include <StationData.h>

namespace LOFAR
{

class DH_RSP: public DataHolder
{
public:
  typedef u16complex BufferType;

  explicit DH_RSP (const string& name,
                   const ACC::APS::ParameterSet pset);

  DH_RSP(const DH_RSP&);

  virtual ~DH_RSP();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  EthernetFrame& getEthernetFrame() const;

 private:
  /// Forbid assignment.
  DH_RSP& operator= (const DH_RSP&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  /// pointers to data in the blob
  EthernetFrame* itsEthernetFrame;

  int itsNoBeamlets;
  int itsNTimes;
  int itsNoPolarisations;
  unsigned int itsBufSize;

  ACC::APS::ParameterSet itsPSet;
};

inline EthernetFrame& DH_RSP::getEthernetFrame() const 
  { return *itsEthernetFrame; }

}

#endif 
