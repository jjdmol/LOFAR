//  DH_CorrCube.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
			  const int channels)
: DataHolder    (name, "DH_CorrCube"),
  itsBuffer     (0),
  nstations     (stations),
  nchannels     (channels),
  nsamples      (samples)
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
  itsBufSize = nstations * nchannels * nsamples ;

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

DH_CorrCube::BufferType* DH_CorrCube::getBufferElement(int channel,
						       int station, 
						       int sample) {
  return itsBuffer + channel*nstations*nsamples + nstations*sample + station;
}

void DH_CorrCube::setBufferElement(int channel, 
				   int station, 
				   int sample, 
				   DH_CorrCube::BufferType* valueptr) {
  DH_CorrCube::BufferType* ptr = itsBuffer + 
    channel*nstations*nsamples + 
    nstations*sample + station;

  *ptr = *valueptr;
}



}
