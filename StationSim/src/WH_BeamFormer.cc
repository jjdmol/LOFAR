//#  WH_BeamFormer.cc:
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
//#

/******************************************************************************/
/* This Workholder implements the Beamformer. Be aware that that some of this */
/* code will generate warnings with (at least) gcc 2.95.3 . As far as I'm     */
/* aware these can be safely ignored.                                         */
/******************************************************************************/




#include <stdio.h>             // for sprintf

#include <StationSim/WH_BeamFormer.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>

using namespace blitz;

WH_BeamFormer::WH_BeamFormer (const string& name,
			      unsigned int nin,
			      unsigned int nout,
			      unsigned int nrcu,
			      unsigned int nsubband, unsigned int nbeam,
			      unsigned int maxNtarget, unsigned int maxNrfi,
			      unsigned int fifoLength, unsigned int bufLength)
: WorkHolder    (nin, nout, name,"WH_BeamFormer"),  // Check number of inputs and outputs
  itsInHolders  (0),
  itsWeight     ("weight"),
  itsOutHolders (0),
  itsSnapFifo   ("fifo", nrcu, bufLength), // this completes the number of inputs and outputs
  itsNrcu       (nrcu),
  itsNsubband   (nsubband),
  itsNbeam      (nbeam),
  itsMaxNtarget (maxNtarget),
  itsMaxNrfi    (maxNrfi),
  itsFifoLength (fifoLength),
  itsBufferLength (bufLength)
{
  // The first time the weights should not be read.
  itsWeight.setReadDelay (1);
  // the number of inputs is equal to the number of reveiving elements
  if (nin > 0) {
    itsInHolders = new DH_SampleC* [nin];
  }
  char str[8];
  for (unsigned int i=0; i<nin; i++) {
    sprintf (str, "%d",i);
    itsInHolders[i] = new DH_SampleC (string("in_") + str, 1, 1);
  }
  // idem for the number of outputs
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string("out_") + str, 1, 1);
  }
  sample.resize(itsNrcu);
  
  itsFifo.resize(itsNrcu, itsFifoLength);
  itsFifo = -1; // initialise the fifo to some value.. Primarily for debugging.
}


WH_BeamFormer::~WH_BeamFormer()
{
  for (int i=0; i<getInputs(); i++) {
    delete itsInHolders[i];
  }
  delete [] itsInHolders;
  for (int i=0; i<getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}


WorkHolder* WH_BeamFormer::construct (const string& name,
				      int ninput, int noutput,int readpos, int writepos,
				      const ParamBlock& params)
{
  Assert (ninput == 4);
  return new WH_BeamFormer (name, ninput, noutput,
			    params.getInt ("nrcu", 10),
			    params.getInt ("nsubband", 10),
			    params.getInt ("nbeam", 10),
			    params.getInt ("maxntarget", 10),
			    params.getInt ("maxnrfi", 10),
			    params.getInt ("bf_fifolength", 512),
			    params.getInt ("bf_bufferlength", 256));
}

WH_BeamFormer* WH_BeamFormer::make (const string& name) const
{
  return new WH_BeamFormer (name, getInputs(), getOutputs(),
			    itsNrcu, itsNsubband, itsNbeam,
			    itsMaxNtarget, itsMaxNrfi, itsFifoLength, 
                            itsBufferLength);
}

void WH_BeamFormer::preprocess()
{

  // I/O operation for reading array configuration from file

  // Add the current snapshot to the input Fifo
  // Assume the fifo is currently at least partially filled
  // so the last (itsBufferLength) elements are usefull data

//   for (int i = 0; i < itsNrcu; i++) {
//     sample(i) = *itsInHolders[i]->getBuffer();
//   }
//   itsOutFifo.resize(itsNrcu, itsBufferLength) ;
//   // place current sample at position (itsWritePos)
//   itsOutFifo(Range::all(), itsWritePos) = sample;
//   // update the rest of the fifo
//   itsOutFifo(Range::all(), Range(itsWritePos+1, toEnd)) = 
//      itsFifo(Range::all(), Range(itsWritePos, itsFifo.ubound(secondDim) - 1));
//   itsOutFifo(Range::all(), Range(itsOutFifo.lbound(secondDim), itsWritePos - 1)) =
//      itsFifo(Range::all(), Range(itsFifo.lbound(secondDim) + 1, itsWritePos));
// //   // Now update (itsWritePos) 
//   itsWritePos = (itsWritePos + 1) % itsFifoLength;


//   // Now assign the values in the outFifo to the outputs for the AWE
//   memcpy(itsSnapFifo.getBuffer(), itsOutFifo.data(), itsNrcu*itsBufferLength);
}

void WH_BeamFormer::process()
{
  // the weights are calculated in WH_AWE

  if (getOutputs() > 0) {

//     DH_SampleC::BufferType* bufin  = itsInHolders[0]->getBuffer();
//     const DH_Weight::BufferType* weight = itsWeight.getBuffer();
//     DH_SampleC::BufferType* bufout = itsOutHolders[0]->getBuffer();

//     // element wise multiply the input with the weights
//     if (sample.size() == itsWeight.getBuffer()->size()) {
//       sample = sample * *itsWeight.getBuffer();
//      } else {
//        cout << "Someone screwed up.. Sample size and weight size differ" << endl;
//     }
    // assign resulting snapshot to outHolders
    for (int i = 0; i < 1; i++) {
      // FIXME -- this is probably wrong - sample.data() is too large
      // Try to prevent overflow by using only one iteration
      //memcpy(itsOutHolders[i]->getBuffer(), sample.data(), 1);
      memcpy(itsOutHolders[i]->getBuffer(), itsInHolders[i]->getBuffer(),sizeof(DH_SampleC::BufferType));
    }
  }
}



void WH_BeamFormer::dump() const
{
  cout << "WH_BeamFormer " << getName() << " Buffers:" << endl;
  if (getOutputs() > 0) {
    for (int i=0; i < itsNrcu; i++) {
      cout << itsOutHolders[i]->getBuffer() << '.' << endl  ;
    }
  }
  // This in case of extreme debugging
  // Dump the entire fifo to screen
  // This might take a while.

  // cout << itsOutFifo << endl;
}


DataHolder* WH_BeamFormer::getInHolder (int channel)
{
  AssertStr (channel < getInputs(),
	     "input channel too high");
  if (channel >= 0) {
    return itsInHolders[channel];
  } else {
    return &itsWeight;
  }
}

DataHolder* WH_BeamFormer::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return itsOutHolders[channel];
}

