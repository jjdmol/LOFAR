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

#include <stdio.h>             // for sprintf

#include <StationSim/WH_BandSep.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>

#include <aips/Arrays/ArrayMath.h>
#include <aips/Arrays/Matrix.h>
#include <aips/OS/RegularFile.h>
#include <aips/IO/RegularFileIO.h>


WH_BandSep::WH_BandSep (const string& name,
			unsigned int nout,
			unsigned int nrcu, unsigned int nsubband,
			const string& coeffFileName)
: WorkHolder   (1, nout, name, "WH_BandSep"),
  itsInHolder  ("in", nrcu, 1),
  itsOutHolders(0),
  itsNrcu      (nrcu),
  itsNsubband  (nsubband),
  itsCoeffName (coeffFileName),
  itsSubPos    (0),
  itsFiltPos   (0)
{
  if (nout > 0) 
  {
    itsOutHolders = new DH_SampleC* [nout];
  }

  char str[8];
  
  // Create the output DH-s.
  for (unsigned int i=0; i<nout; i++) 
  {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC(string("out_") + str, nrcu, nsubband);
  }
  osDebugInput.open ("debuginput.dat", ios::out);
  iCount = 0;
}

WH_BandSep::~WH_BandSep()
{
  // DEBUG
  osDebugInput.close();

  for (int i = 0; i < getOutputs(); i++) 
  {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_BandSep::construct(const string& name, int ninput, int noutput,
				   const ParamBlock& params)
{
  Assert (ninput == 1);

  return new WH_BandSep(name, noutput, params.getInt("nrcu", 10),
			params.getInt("nsubband", 10), params.getString("coeffname", 
			"filter.coeff"));
}

WH_BandSep* WH_BandSep::make(const string& name) const
{
  return new WH_BandSep (name, getOutputs(), itsNrcu, itsNsubband,
			 itsCoeffName);
}

void WH_BandSep::preprocess()
{
  // If first time, read the filter coefficients and allocate the buffers.
  if (itsConv.nelements() == 0) 
  {
     ifstream coeffFile(itsCoeffName.c_str());
     AssertStr (coeffFile, "Coeff.file " << itsCoeffName << " not found");
     RegularFile file(itsCoeffName);
     RegularFileIO fio(file);
     int size = fio.length();
     int ncoeff = size / sizeof(double);
     vector<double> coeff(ncoeff);
     fio.read(size, &coeff[0]);

     AssertStr (ncoeff > 0  &&  ncoeff % itsNsubband == 0,
     	       "Incorrect number of filter coefficients given");
    Matrix<double> filter(IPosition(2, itsNsubband, itsFilterLength),
			  &coeff[0], SHARE);
    //filter /= max(filter);
  
    itsFilterLength = ncoeff / itsNsubband;

    // Create for each subband the filter vector from the matrix.
    itsFilters.resize(itsNsubband);

    for (int i = 0; i < itsNsubband; i++) 
    {
      vector<double>& filt = itsFilters[i];
      filt.resize(itsFilterLength, 0);

      for (int j = 0; j < itsFilterLength; j++) 
      {
	filt[j] = filter(i, j);
      }
    }

    // Size and initialize the buffers.
    for (int i = 0; i < itsNrcu; i++) 
    {
      itsBuffers.resize(itsNsubband);
      itsBuffers[i].resize(itsFilterLength * itsNsubband, 0);
    }

    // Create convolution and FFT buffers.
    itsConv.resize(itsNsubband);
    itsFFTBuf.resize((itsNsubband + 2) / 2);
    itsFFTserver.resize(IPosition(1, itsNsubband));
  }
}

void WH_BandSep::process()
{
  // Process receives each time a buffer of nrcu samples.
  // Every nsubband times it convolutes and ffts the buffered data
  // and outputs it.
  if (getOutputs() > 0) 
  {
    DH_SampleR::BufferType* bufin = itsInHolder.getBuffer();

    // Keep the received data per RCU in a buffer.
    Assert(itsSubPos < itsNsubband);
    Assert(itsFiltPos < itsFilterLength);

    int pos = itsSubPos * itsFilterLength + itsFiltPos;

    for (int i = 0; i < itsNrcu; i++) 
    {
      itsBuffers[i][pos] = bufin[i];
    }
    itsSubPos++;
    
    // Calculate the filter output. This only needs to be done if in the
    // process of downsampling it is time to process a bunch of samples
    // (which is controlled by the rate of the output dataholder).
    if (itsOutHolders[0]->doHandle()) 
    {
      Assert(itsSubPos == itsNsubband);
      DH_SampleC::BufferType* bufout = itsOutHolders[0]->getBuffer();
      
      osDebugInput << " Multiplication " << iCount++ << " :" << endl;
      for (int i = 0; i < itsNrcu; i++) 
      {
	osDebugInput << endl << "next antenna: " <<  endl;
	for (int j = 0; j < itsNsubband; j++) 
	{
	  double& conv = itsConv(j);
	  conv = 0;
	  int fp = 0;
	  int pos = j * itsFilterLength + itsFiltPos;

// 	  // Go from begin to end the data.
// 	  for (int k = pos; k >= j * itsFilterLength; k--) 
// 	  {
// 	    conv += itsFilters[j][fp++] * itsBuffers[i][k];
// 	    osDebugInput << itsFilters[j][fp - 1] << ' ' << itsBuffers[i][k] << ' ' << conv << endl;
// 	  }
// 	  for (int k = (j + 1) * itsFilterLength - 1; k > pos; k--) 
// 	  {
// 	    conv += itsFilters[j][fp++] * itsBuffers[i][k];
// 	    osDebugInput << itsFilters[j][fp - 1] << ' ' << itsBuffers[i][k] << ' ' << conv << endl;
// 	  }

	  // Go from end to begin through the data.
	  for (int k = pos + 1; k < (j + 1) * itsFilterLength; k++) 
	  {
	    conv += itsFilters[j][fp++] * itsBuffers[i][k];
	    osDebugInput << itsFilters[j][fp - 1] << ' ' << itsBuffers[i][k] << ' ' << conv << endl;
	  }
	  for (int k = j * itsFilterLength; k <= pos; k++) 
	  {
	    conv += itsFilters[j][fp++] * itsBuffers[i][k];
	    osDebugInput << itsFilters[j][fp - 1] << ' ' << itsBuffers[i][k] << ' ' << conv << endl;
	  }

	  osDebugInput << endl;
	  conv /= itsFilterLength;
	}
		
	itsFFTserver.fft0(itsFFTBuf, itsConv, False);
	itsFFTBuf *= DH_SampleC::BufferType(1./itsNsubband);

	// The FFT output has (n+2)/2 points, where first point is the
	// DC component and last point is nyquist sample (if n is even).
	// Matlab returns all points, so put complex conjugate in remainder.
	// Store the output with rcu as most rapidly varying axis.

	DH_SampleC::BufferType* out = bufout + i;



	// With FFT do:
  	for (int j = 0; j < itsNsubband / 2; j++)
  	{
  	  out[j * itsNrcu] = itsFFTBuf(j);
  	}

  	for (int j = 1; j < (itsNsubband - 1) / 2; j++) 
  	{
  	  out[(itsNsubband - j) * itsNrcu] = conj(itsFFTBuf(j));
  	}

	// Just convolution do:
//   	for (int j = 0; j < itsNsubband; j++)
//    	{
//    	  out[j * itsNrcu] =  itsConv(j);
//    	}
      }

      // Copy to other output buffers.
      for (int i = 1; i < getOutputs(); i++) 
      {
	memcpy (getOutHolder(i)->getBuffer(), bufout,
		itsNrcu * itsNsubband * sizeof(DH_SampleC::BufferType));
      }

      // Move to next buffer position.
      // The buffer is circular, to restart if it is full.
      itsFiltPos++;
      if (itsFiltPos == itsFilterLength) 
      {
	itsFiltPos = 0;
      }
      itsSubPos = 0;
    }
  }
}

void WH_BandSep::dump() const
{
  cout << "WH_BandSep " << getName() << " Buffer:" << endl;

  if (getOutputs() > 0) 
  {
    cout << itsOutHolders[0]->getBuffer()[0] << ','
	 << itsOutHolders[0]->getBuffer()[itsNrcu * itsNsubband - 1] << endl;
  }
}


DH_SampleR* WH_BandSep::getInHolder(int channel)
{
  AssertStr (channel == 0, "input channel too high");

  return &itsInHolder;
}

DH_SampleC* WH_BandSep::getOutHolder(int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");

  return itsOutHolders[channel];
}

//# Instantiate the FFTServer.
#include <aips/Mathematics/FFTServer.cc>
#include <aips/Arrays/ArrayLogical.cc>
template class FFTServer<double, complex<double> >;
template bool allNearAbs(Array<double> const &, double const &, double);
