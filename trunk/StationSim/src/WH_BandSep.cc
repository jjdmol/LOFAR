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
						unsigned int nrcu, const string& coeffFileName, 
						int nout, bool tapstream, string subselfile, int nselsubband, int qms)
: WorkHolder        (1, nout, name, "WH_BandSep"),
  itsInHolder       ("in", nrcu, 1),
  itsOutHolders     (0),
  itsNsubband       (nsubband),
  itsNrcu           (nrcu),
  itsCoeffName      (coeffFileName),
  itsPos            (0),
  itsNout           (nout),
  itsBuffer         (nrcu, nsubband),
  itsSubbandSignals (nrcu, nsubband, 1),
  itsTemp           (nsubband, 1),
  itsTapStream      (tapstream),
  itsNselsubband    (0),
  itsSubSelFile     (subselfile),
  itsSelection      (nsubband),
  itsSelSubSignals  (nrcu, nselsubband),
  itsCount          (0),
  itsQms            (qms)
{
  // Allocate blocks to hold pointers to input and output DH-s.
  if (nsubband > 0) {
    itsOutHolders = new DH_SampleC * [nout];
  }
  // Create the input DH-s.
  char str[8];

  // Create the output DH-s.
  for (int i = 0; i < nout; i++) {
	sprintf (str, "%d", i);
	itsOutHolders[i] = new DH_SampleC (string ("out_") + str, nrcu, nselsubband);

	if (itsTapStream) {
	  itsOutHolders [i]->setOutFile (string ("FB1_") + str + string (".dat"));
	}
  }

  // Read in the subband selection file
  ifstream selfile;
  selfile.open (subselfile.c_str ());
  selfile >> itsSelection;
  selfile.close();

  for (int i = 0; i < itsNsubband; i++) {
	if (itsSelection (i)) {
	  itsNselsubband++;
	}
  }

  // Check if there is at least one subband selected
  AssertStr (itsNselsubband > 0, "At least one subband most be selected!");
  AssertStr (itsNselsubband == nselsubband, "The subband selection file and number of selected subbands are not the same!");
}

WH_BandSep::~WH_BandSep()
{
  for (int i = 0; i < getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}

WorkHolder* WH_BandSep::construct(const string& name, int ninput, int noutput, 
								  const ParamBlock& params)
{
  return new WH_BandSep(name, params.getInt("nsubband", 16), params.getInt ("nrcu", 92),
						params.getString("coeffname", "filter.coeff"), ninput, false,
						params.getString("selfilename", ""), params.getInt("nselsubband", 16),
						params.getInt("qms", 0));
}

WH_BandSep* WH_BandSep::make(const string& name) const
{
  return new WH_BandSep (name, itsNsubband, itsNrcu, itsCoeffName, itsNout, itsTapStream, 
						 itsSubSelFile, itsNselsubband, itsQms);
}

void WH_BandSep::preprocess()
{
  // Initialize
  itsFilterbank = new FilterBank <dcomplex> * [itsNrcu];
  for (int i = 0; i < itsNrcu; i++) {
	itsFilterbank[i] = new FilterBank <dcomplex> (itsCoeffName, itsOverlapSamples, isComplex);
  }
  
  // Check if the given coefficient file matches the given number of subbands
  AssertStr (itsNsubband == itsFilterbank [0]->getItsNumberOfBands (), "The coefficientfile isn't correct!");
}

void WH_BandSep::process()
{
  Range all = Range::all ();

  if (getOutputs() > 0)  {
	// store the input in a buffer, when the buffer has reached Nsubbands the filter routine must be called
 	for (int i = 0; i < itsNrcu; i++) {
	  itsBuffer (i, itsPos) = itsInHolder.getBuffer ()[i];
	}
 	itsPos = ++itsPos % itsNsubband;

	// filter the signal
	if (itsOutHolders[0]->doHandle()) {
	  for (int i = 0; i < itsNrcu; i++) {
  		// filter the signal per rcu
		itsTemp = itsFilterbank [i]->filter (itsBuffer (i, all));
		//itsSubbandSignals (i, all) = itsTemp (all, 0);	   

		// Do a fftshift, put the DC component in the middle of the band
		itsSubbandSignals (i, Range (fromStart, itsNsubband / 2 - 1)) = 
		  itsTemp (Range (itsNsubband / 2, toEnd), 0);	   

		itsSubbandSignals (i, Range (itsNsubband / 2, toEnd)) = 
		  itsTemp (Range (fromStart, itsNsubband / 2 -1), 0);	   
	  }

	  // do the selection of the subbands
	  int s = 0;
	  for (int i = 0; i < itsNsubband && itsSelection (i); i++) {
		itsSelSubSignals (all, s++) = itsSubbandSignals (all, i);
	  }

	  cout << "WH_BandSep : " << itsCount++ << endl;

	  // Copy to other output buffers.
	  for (int i = 0; i < itsNout; i++) {
		memcpy (getOutHolder (i)->getBuffer (), itsSelSubSignals.data (), 
				sizeof (DH_SampleC::BufferType) * itsNrcu * itsNselsubband);		
	  }
	}
  }
}

void WH_BandSep::dump() const
{}


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

// void WH_BandSep::plot_freq_spectrum(const LoMat_dcomplex& sb_signals, const int pos) 
// {

//   // create the contigeous buffers
//   LoMat_dcomplex cont_sb_signals (sb_signals.shape());
//   int nc = cont_sb_signals.cols();

// //   cont_sb_signals(Range(0, nc - pos - 2)) = 
// //     sb_signals(Range(pos + 1, nc - 1)) ;
// //   cont_sb_signals(Range(nc - pos - 1, nc - 1)) =
// //     sb_signals(Range(0, pos)) ;


//   cont_sb_signals(Range::all(), Range(fromStart, nc - pos - 2)) = 
//     sb_signals(Range::all(), Range(pos+1, toEnd)) ;
//   cont_sb_signals(Range::all(), Range(nc - pos - 1, toEnd)) = 
//     sb_signals(Range::all(), Range(fromStart, pos)) ;


//   LoMat_dcomplex spectrum (sb_signals.shape());
//   LoMat_double pwr(spectrum.shape());

//   spectrum = cont_sb_signals * LCSMath::conj (cont_sb_signals );
//   pwr = 20 * log10( sqrt ( sqr ( real ( spectrum ) ) + sqr ( imag( spectrum ) ) ) );
  
//   // scale -inf to -70 to allow gnuplot to plot something
//   // This is ugly as hell, I know..
//   for (int i = 0; i < pwr.rows(); i++) {
//     for (int j = 0; j < pwr.cols(); j++) {
//       if (pwr(i,j) < -70) {
// 	pwr(i,j) = -70;
//       }
//     }
//   }

//   //  gnuplot_close(handle);
//   //handle = gnuplot_init ();
//   gnuplot_cmd(handle, "set view 61,336");
//   gnuplot_splot (handle, pwr, "Frequency power spectrum over time");  
// }
