//  DH_Vis.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////


#include <DH_Vis.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{

DH_Vis::DH_Vis (const string& name, const int stations, const int channels, const int polarisations)
: DataHolder    (name, "DH_Vis"),
  itsBuffer     (0),
  nstations     (stations),
  nchannels     (channels),
  npolarisations(polarisations)
{

}

DH_Vis::DH_Vis(const DH_Vis& that)
  : DataHolder(that),
    itsBuffer(0)
{
}

DH_Vis::~DH_Vis()
{
}

DataHolder* DH_Vis::clone() const
{
  return new DH_Vis(*this);
}

void DH_Vis::preprocess()
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = nstations * nstations * nchannels * npolarisations; 

  addField("Buffer", BlobField<BufferType>(1, itsBufSize));
  createDataBlock();
  itsBuffer = getData<BufferType> ("Buffer");
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i] = 0;
  }
}

void DH_Vis::postprocess()
{
  itsBuffer     = 0;
}

void DH_Vis::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");

}


}
