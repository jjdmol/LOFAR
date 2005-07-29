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
#include <complex>

using std::complex;


namespace LOFAR
{

DH_RSP::DH_RSP (const string& name,
                const ACC::APS::ParameterSet pset)
: DataHolder (name, "DH_RSP"),
  itsBuffer  (0),
  itsPSet    (pset)
{
  itsNTimes          = pset.getInt32("Input.NSamplesToDH");
  itsNoPolarisations = pset.getInt32("Input.NPolarisations");
  itsBufSize         = itsNTimes * itsNoPolarisations;
}

DH_RSP::DH_RSP(const DH_RSP& that)
: DataHolder         (that),
  itsBuffer          (0),
  itsNTimes          (that.itsNTimes),
  itsNoPolarisations (that.itsNoPolarisations),
  itsBufSize         (that.itsBufSize),
  itsPSet            (that.itsPSet)
{}

DH_RSP::~DH_RSP()
{}

DataHolder* DH_RSP::clone() const
{
  return new DH_RSP(*this);
}

void DH_RSP::init()
{
  // Add the fields to the data definition.
  addField ("Buffer", BlobField<BufferType>(1,itsBufSize));
  addField ("StationID", BlobField<int>(1));
  addField ("InvalidCount", BlobField<int>(1));
  addField ("Delay", BlobField<int>(1));
  addField ("TimeStamp", BlobField<char>(1, sizeof(timestamp_t)));
  
  // Create the data blob
  createDataBlock();

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Times", itsNTimes));
  vdd.push_back(DimDef("Polarisations", itsNoPolarisations));
  
  itsMatrix = new RectMatrix<BufferType> (vdd);
  itsMatrix->setBuffer(itsBuffer, itsBufSize);
}

void DH_RSP::fillDataPointers()
{
  // Fill in the buffer pointer.
  itsBuffer  = getData<BufferType> ("Buffer");

  // Fill in the StationID pointer
  itsStationID = getData<int> ("StationID");
  
  // Fill in the InvalidCount pointer
  itsInvalidCount = getData<int> ("InvalidCount");

  // Fill in the Delay pointer
  itsDelay = getData<int> ("Delay");
  
  // Fill in TimeStamp pointer
  itsTimeStamp = (timestamp_t*)getData<char> ("TimeStamp");

  // use memset to null the buffer
  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType));
}


} // end namespace
