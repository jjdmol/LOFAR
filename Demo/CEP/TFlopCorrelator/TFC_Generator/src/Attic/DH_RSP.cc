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
#include <Common/lofar_complex.h>

namespace LOFAR
{

DH_RSP::DH_RSP (const string& name,
                const ACC::APS::ParameterSet pset)
: DataHolder       (name, "DH_RSP"),
  itsEthernetFrame (0),
  itsPSet          (pset)
{
  itsBufSize         = EthernetFrame::getSize(itsPSet);
}

DH_RSP::DH_RSP(const DH_RSP& that)
: DataHolder         (that),
  itsEthernetFrame   (0),
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
  addField ("EthernetFrame", BlobField<char>(1, itsBufSize));
  // Create the data blob
  createDataBlock();

}

void DH_RSP::fillDataPointers()
{
  itsEthernetFrame = new EthernetFrame(itsPSet, getData<char>("EthernetFrame"), itsBufSize);
  itsEthernetFrame->reset();
}


} // end namespace
