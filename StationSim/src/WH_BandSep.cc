//#  WH_BandSep.cc:
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

#include <Common/Debug.h>
#include <BaseSim/ParamBlock.h>
#include <StationSim/WH_BandSep.h>
#include <stdio.h>                        // for sprintf


WH_BandSep::WH_BandSep (const string& name, unsigned int nsubband,
			const string& coeffFileName, int nout)
: WorkHolder   (1, nsubband * nout, name, "WH_BandSep"),
  itsInHolder  ("in", 1, 1),
  itsOutHolders(0),
  itsNsubband  (nsubband),
  itsCoeffName (coeffFileName),
  itsPos       (0),
  itsNout      (nout)
{
  // Allocate blocks to hold pointers to input and output DH-s.
  if (nsubband > 0) {
    itsOutHolders = new DH_SampleC *[nsubband * nout];
  }
  // Create the input DH-s.
  char str[8];
  char str2[8];

  // Create the output DH-s.
  for (unsigned int j = 0; j < nout; j++) {
    for (unsigned int i = 0; i < nsubband; i++) {
      sprintf (str, "%d", i);
      sprintf (str2, "%d", j);
      itsOutHolders[i + j * nsubband] = new DH_SampleC (string ("out") + 
							str2 + string("_") + str, 1, 1);
    }
  }

  // DEBUG
  handle = gnuplot_init();
  itsFileOutReal.open ((string ("/home/alex/gerdes/subband_real_") + name + string (".txt")).c_str ());
  itsFileOutComplex.open ((string ("/home/alex/gerdes/subband_complex_") + name + string (".txt")).c_str ());
}

WH_BandSep::~WH_BandSep()
{
  for (int i = 0; i < getOutputs(); i++) 
  {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;

  // DEBUG
  gnuplot_close (handle);
  itsFileOutReal.close ();
  itsFileOutComplex.close ();
}

WorkHolder* WH_BandSep::construct(const string& name, int ninput, int noutput, const ParamBlock& params)
{
  return new WH_BandSep(name, params.getInt("nsubband", 10), 
			params.getString("coeffname", "filter.coeff"), ninput);
}

WH_BandSep* WH_BandSep::make(const string& name) const
{
  return new WH_BandSep (name, itsNsubband, itsCoeffName, itsNout);
}

void WH_BandSep::preprocess()
{
  // Initialize
  itsFilterbank = new FilterBank <dcomplex> (itsCoeffName, itsOverlapSamples, isComplex);
  itsBuffer.resize (itsNsubband);
  
  // Check if the given coefficient file matches the given number of subbands
  AssertStr (itsNsubband == itsFilterbank->getItsNumberOfBands(), "The coefficientfile isn't correct!");

  // DEBUG
  //  itsFileOut << itsNsubband << " x 100" << endl << "[ ";
}

void WH_BandSep::process()
{
  if (getOutputs() > 0)  {
	// store the input in a buffer, when the buffer has reached Nsubbands the filter routine must be called
	DH_SampleC::BufferType* bufin = itsInHolder.getBuffer ();
	LoMat_dcomplex subbandSignals (itsNsubband, 1);
	itsBuffer (itsPos) = bufin[0];
	itsPos = (itsPos + 1) % itsBuffer.size ();

	// filter the signal
	if (itsOutHolders[0]->doHandle()) {
	  subbandSignals = itsFilterbank->filter(itsBuffer);
	  
	  // Copy to other output buffers.
	  for (int j = 0; j < itsNout; j++) {
		for (int i = 0; i < itsNsubband; i++) {
 		  getOutHolder (i + j * itsNsubband)->getBuffer ()[0] = subbandSignals (i, 0);
		}
	  }
	  	  
	  // DEBUG
	  for (int i = 0; i < itsNsubband; i++) {
		itsFileOutReal << real (itsOutHolders[i]->getBuffer ()[0]) << " ";
		itsFileOutComplex << imag (itsOutHolders[i]->getBuffer ()[0]) << " ";
	  }
	  itsFileOutReal << endl;	
	  itsFileOutComplex << endl;	
	}
  }
}

void WH_BandSep::dump() const
{
//   cout << "WH_BandSep " << getName() << " Buffer:" << endl;

//   if (getOutputs() > 0) {
//     cout << itsOutHolders[0]->getBuffer()[0] << ','
// 		 << itsOutHolders[0]->getBuffer()[itsNsubband - 1] << endl;
//   }
}


DH_SampleC* WH_BandSep::getInHolder(int channel)
{
  AssertStr (channel == 0, "input channel too high");
  return &itsInHolder;
}

DH_SampleC* WH_BandSep::getOutHolder(int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}
