//  DH_CorrCube.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////


#include <DH_CorrCube.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{

DH_CorrCube::DH_CorrCube (const string& name, 
			  const int stations, 
			  const int samples, 
			  const int channels,
			  const int polarisations)
: DataHolder     (name, "DH_CorrCube"),
  itsBuffer      (0),
  nstations      (stations),
  nchannels      (channels),
  nsamples       (samples),
  npolarisations (polarisations)
{

}

DH_CorrCube::DH_CorrCube(const DH_CorrCube& that)
  : DataHolder(that),
    itsBuffer(0)
{
}

DH_CorrCube::~DH_CorrCube()
{
  //  delete [] (char*)(itsDataPacket);
  itsBuffer = 0;
}

DataHolder* DH_CorrCube::clone() const
{
  return new DH_CorrCube(*this);
}

void DH_CorrCube::preprocess()
{
  // First delete possible buffers.
  postprocess();

  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = 2 * nstations * nchannels * nsamples ;

  addField ("Buffer", BlobField<BufferType>(1, itsBufSize));
  
  createDataBlock();
  itsBuffer = getData<BufferType> ("Buffer");
  for (unsigned int i=0; i<itsBufSize; i++) {
    itsBuffer[i]=0;
  }
}

void DH_CorrCube::postprocess()
{
  itsBuffer     = 0;
}

void DH_CorrCube::fillDataPointers() {
  itsBuffer = getData<BufferType> ("Buffer");
}




}
