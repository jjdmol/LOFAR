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
#include <Math/LCSMath.h>


WH_BandSep::WH_BandSep (const string& name, unsigned int nsubband,
			const string& coeffFileName, int nout, 
			bool tapstream, int qms)
: WorkHolder   (1, nsubband * nout, name, "WH_BandSep"),
  itsInHolder  ("in", 1, 1),
  itsOutHolders(0),
  itsNsubband  (nsubband),
  itsCoeffName (coeffFileName),
  itsPos       (0),
  itsNout      (nout),
  itsTapStream (tapstream),
  itsQms       (qms)
{
  // Allocate blocks to hold pointers to input and output DH-s.
  if (nsubband > 0) {
    itsOutHolders = new DH_SampleC *[nsubband * nout];
  }
  // Create the input DH-s.
  char str[8];
  char str2[8];

  // Create the output DH-s.
  for (int j = 0; j < nout; j++) {
    for (unsigned int i = 0; i < nsubband; i++) {
      sprintf (str, "%d", i);
      sprintf (str2, "%d", j);
      itsOutHolders[i + j * nsubband] = new DH_SampleC (string ("out") + 
							str2 + string("_") + str, 1, 1);
      if (itsTapStream) {
	itsOutHolders [i + j * nsubband]->setOutFile (string ("FB1_") + str2 + 
						      string ("_") + str + string (".dat"));
      }
    }
  }
  qm.resize(QM_BASE_SIZE);
  qm = qminterface(itsQms);

  
  if (qm(2) && name =="1") {
    plot = true;
    handle = gnuplot_init ();
    buffersize=200;
    buffer.resize(nsubband, buffersize);
    buffer_pos = 0;
  } else {
    plot = false;
  }
}

WH_BandSep::~WH_BandSep()
{
  for (int i = 0; i < getOutputs(); i++) 
  {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;

  // DEBUG
//   gnuplot_close (handle);
//   itsFileOutReal.close ();
//   itsFileOutComplex.close ();
}

WorkHolder* WH_BandSep::construct(const string& name, int ninput, int noutput, const ParamBlock& params)
{
  return new WH_BandSep(name, params.getInt("nsubband", 10), 
			params.getString("coeffname", "filter.coeff"), ninput, false,
			params.getInt("qms",0));
}

WH_BandSep* WH_BandSep::make(const string& name) const
{
  return new WH_BandSep (name, itsNsubband, itsCoeffName, itsNout, itsTapStream, itsQms);
}

void WH_BandSep::preprocess()
{
  // Initialize
  itsFilterbank = new FilterBank <dcomplex> (itsCoeffName, itsOverlapSamples, isComplex);
  itsBuffer.resize (itsNsubband);
  
  // Check if the given coefficient file matches the given number of subbands
  AssertStr (itsNsubband == itsFilterbank->getItsNumberOfBands(), "The coefficientfile isn't correct!");
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

	  // DEBUG
//  	  itsNsubband = 4;
//  	  subbandSignals.resize(itsNsubband, 1);
//  	  subbandSignals = 1, 2, 3, 4; 
//  	  cout << subbandSignals << endl;
	  // !DEBUG

	  //AG: Do a fftshift, put the DC component in the middle of the band
  	  int nfft = itsNsubband;	
	  
  	  LoMat_dcomplex temp (nfft / 2, 1);
	  //  	  LoMat_dcomplex cdata(itsNsubband, 1);
	  LoMat_dcomplex cdata(subbandSignals.shape());
  	  cdata = subbandSignals;

  	  temp = cdata (Range (nfft / 2, toEnd));
  	  cdata (Range (nfft / 2, toEnd)) = cdata (Range (fromStart, nfft / 2 - 1));
  	  cdata (Range (fromStart, nfft / 2 - 1)) = temp;
	  
  	  subbandSignals=cdata;
	  
	  if (qm(2) && plot) {
	    // Plot only the spectrum for the first antenna
	    buffer(Range::all(), buffer_pos) = subbandSignals(Range::all(), 0);
	    buffer_pos = (buffer_pos + 1) % buffersize; 	    
	    if (buffer_pos % 50 == 0) {
	      plot_freq_spectrum(buffer, buffer_pos);
	    }
  	  }
	  
	  // Copy to other output buffers.
	  for (int j = 0; j < itsNout; j++) {
	    for (int i = 0; i < itsNsubband; i++) {
	      getOutHolder (i + j * itsNsubband)->getBuffer ()[0] = subbandSignals (i, 0);
	    }
	  }
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

void WH_BandSep::plot_freq_spectrum(const LoMat_dcomplex& sb_signals, const int pos) 
{

  // create the contigeous buffers
  LoMat_dcomplex cont_sb_signals (sb_signals.shape());
  int nc = cont_sb_signals.cols();

//   cont_sb_signals(Range(0, nc - pos - 2)) = 
//     sb_signals(Range(pos + 1, nc - 1)) ;
//   cont_sb_signals(Range(nc - pos - 1, nc - 1)) =
//     sb_signals(Range(0, pos)) ;


  cont_sb_signals(Range::all(), Range(fromStart, nc - pos - 2)) = 
    sb_signals(Range::all(), Range(pos+1, toEnd)) ;
  cont_sb_signals(Range::all(), Range(nc - pos - 1, toEnd)) = 
    sb_signals(Range::all(), Range(fromStart, pos)) ;


  LoMat_dcomplex spectrum (sb_signals.shape());
  LoMat_double pwr(spectrum.shape());

  spectrum = cont_sb_signals * LCSMath::conj (cont_sb_signals );
  pwr = 20 * log10( sqrt ( sqr ( real ( spectrum ) ) + sqr ( imag( spectrum ) ) ) );
  
  // scale -inf to -70 to allow gnuplot to plot something
  // This is ugly as hell, I know..
  for (int i = 0; i < pwr.rows(); i++) {
    for (int j = 0; j < pwr.cols(); j++) {
      if (pwr(i,j) < -70) {
	pwr(i,j) = -70;
      }
    }
  }

  //  gnuplot_close(handle);
  //handle = gnuplot_init ();
  gnuplot_cmd(handle, "set view 61,336");
  gnuplot_splot (handle, pwr, "Frequency power spectrum over time");  
}
