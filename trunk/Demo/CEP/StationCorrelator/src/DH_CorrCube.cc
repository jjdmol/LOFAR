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
  // Determine the number of bytes needed for DataPacket and buffer.
  //cout << nstations << " " << nchannels << " " << nsamples << " " << npolarisations << endl;
  itsBufSize = nstations * nchannels * nsamples * npolarisations ;
  //  cout << itsBufSize << endl;
}

DH_CorrCube::DH_CorrCube(const DH_CorrCube& that)
  : DataHolder     (that),
    itsBuffer      (0),
    nstations      (that.nstations),
    nchannels      (that.nchannels),
    nsamples       (that.nsamples),
    npolarisations (that.npolarisations)
{
  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = nstations * nchannels * nsamples * npolarisations ;
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

  
  addField ("BlockID", BlobField<int>(1, 1));
  addField ("Flag", BlobField<int>(1, 1));
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
  itsBlockIDptr = getData<int> ("BlockID");
  itsFlagptr = getData<int> ("Flag");
  itsBuffer = getData<BufferType> ("Buffer");
}
}
