//#  DH_RSP.cc: DataHolder storing RSP raw ethernet frames for 
//#             StationCorrelator demo
//#
//#  Copyright (C) 2000, 2001
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#
//#  $Id$
//#
///////////////////////////////////////////////////////////////////

#include <DH_RSP.h>

namespace LOFAR
{

DH_RSP::DH_RSP (const string& name)
: DataHolder (name, "DH_RSP"),
  itsBuffer  (0)
{

  ACC::ParameterSet  myPS("TFlopCorrelator.cfg");

  itsEPAheaderSize   = myPS.getInt("SzEPAheader"); // default 14
  itsNoBeamlets      = myPS.getInt("NoRSPBeamlets", 92) ; // default 14
  itsNoPolarisations = myPS.getInt("polarisations", 2);   // default 14
  const int NoPacketsInFrame = myPS.getInt("NoPacketsInFrame"); // default 8
  itsBufSize         =  NoPacketsInFrame * 
    (itsEPAheaderSize + itsNoBeamlets * itsNoPolarisations * sizeof(complex<int16>));
}

DH_RSP::DH_RSP(const DH_RSP& that)
: DataHolder         (that),
  itsBuffer          (0),
  itsEPAheaderSize   (that.itsEPAheaderSize),
  itsNoBeamlets      (that.itsNoBeamlets),
  itsNoPolarisations (that.itsNoPolarisations),
  itsBufSize         (that.itsBufSize)
{}

DH_RSP::~DH_RSP()
{}

DataHolder* DH_RSP::clone() const
{
  return new DH_RSP(*this);
}

void DH_RSP::preprocess()
{
  // first delete possible preexisting buffers
  postprocess();
  
  // Add the fields to the data definition.
  addField ("Buffer", BlobField<BufferType>(1,itsBufSize));
  addField ("Flag", BlobField<int>(1, 1));

  // Create the data blob
  createDataBlock();
}

void DH_RSP::fillDataPointers()
{
  // Fill in the buffer pointer.
  itsFlagptr = getData<int> ("Flag");
  itsBuffer  = getData<BufferType> ("Buffer");

  // use memset to null the buffer instead of a for loop
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType));
}

void DH_RSP::postprocess()
{
  itsBuffer = 0;
}

} // end namespace
