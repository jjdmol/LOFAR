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
//#  



#include <math.h>              // for isfinite(x)
#include <stdio.h>             // for sprintf

#include <StationSim/WH_BeamFormer.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>
#include <Math/LCSMath.h>


using namespace blitz;

WH_BeamFormer::WH_BeamFormer (const string& name,
			      unsigned int nin,
			      unsigned int nout,
			      unsigned int nrcu,
			      unsigned int nbeam,
			      unsigned int maxNtarget, 
			      unsigned int maxNrfi,
			      bool tapstream, 
			      string arraycfg, int qms
)
  : WorkHolder    (nin, nout, name,"WH_BeamFormer"),  // Check number of inputs and outputs
  itsInHolders  (0),
  itsOutHolders (0),
  itsWeight     ("weights", nrcu, 1),
  itsNrcu       (nrcu),
  itsNbeam      (nbeam),
  itsMaxNtarget (maxNtarget),
  itsMaxNrfi    (maxNrfi),
  itsTapStream  (tapstream),
  itsArray      (arraycfg),
  sample        (nrcu),
  itsBuffer     (0),
  itsBeamBuffer (0),
  itsPos(0),
  itsQms(qms)
{
  char str[8];
  // the number of inputs is equal to the number of reveiving elements
  if (nin > 0) {
    itsInHolders = new DH_SampleC* [nrcu];
  }
  for (unsigned int i = 0; i < nrcu; i++) {
    sprintf (str, "%d",i);
    itsInHolders[i] = new DH_SampleC (string ("in_") + str, 1, 1);
  }
  // idem for the number of outputs
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  for (unsigned int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string ("out_") + str, 1, 1);

	if (itsTapStream) {
	  itsOutHolders [i]->setOutFile (string ("BF_") + str + string (".dat"));
	}
  }

  //DEBUG
//    itsTestVector.resize(itsNrcu, 300);
//    itsFileInput.open ("/home/chris/PASTD-validation/test_vectorSTA.txt");
//    itsFileInput >> itsTestVector;

  //  handle = gnuplot_init ();
  iCount = 0;
  plotCount = 1;

  // EXPERIMENT TOOLS
  // Buffer size should depend on the particular experiment that is being run.
  int itsBufferSize = 100;
  itsBuffer.resize(itsNrcu, itsBufferSize);
  itsBuffer=0;
  itsBeamBuffer.resize(itsBufferSize);
  itsBeamBuffer=0;

  qm.resize(QM_BASE_SIZE);
  qm = qminterface(qms);

  if (qm(0)) {
    handle_bp = gnuplot_init();
  }

  if (qm(1)) {
    handle_sp = gnuplot_init();
  }

}


WH_BeamFormer::~WH_BeamFormer()
{
  for (int i = 0; i < itsNrcu; i++) {
    delete itsInHolders[i];
  }
  delete [] itsInHolders;
  for (int i = 0; i < getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;

//  gnuplot_close (handle);
  //  DEBUG
  itsFileInput.close ();
}


WorkHolder* WH_BeamFormer::construct (const string& name,
				      int ninput, int noutput, const ParamBlock& params)
{
  return new WH_BeamFormer (name, ninput, noutput,
			    params.getInt ("nrcu", 10),
			    params.getInt ("nbeam", 10),
			    params.getInt ("maxntarget", 10),
			    params.getInt ("maxnrfi", 10),
			    false, 
			    params.getString ("bffilename",""),
			    params.getInt ("qms", 0));
}

WH_BeamFormer* WH_BeamFormer::make (const string& name) const
{
  return new WH_BeamFormer (name, getInputs(), getOutputs(),
			    itsNrcu, itsNbeam, itsMaxNtarget, 
			    itsMaxNrfi, itsTapStream, itsArray.conf_file, itsQms);
}


void WH_BeamFormer::process()
{
  if (getOutputs () > 0) {

    // Fill the buffer to be able to calculate the spectrum
    // Place the next incoming sample vector in the snapshot buffer 
    // Keep a cylic buffer for the input snapshots
    // We don't bother keeping it ordered.. Should not make any
    // difference.
    //    for (int i = 0; i < itsNrcu; i++) {
      //      itsBuffer(i, itsPos) = itsInHolders[i]->getBuffer()[0];
      // 	// DEBUG
      // itsBuffer(i, itsPos) = itsTestVector(i, itsPos);
      //    }
    //    itsPos = (itsPos + 1) % itsBuffer.cols();

    dcomplex temp;
    for (int i = 0; i < itsNrcu; i++) {
      sample(i) = (dcomplex)itsInHolders[i]->getBuffer()[0]; 
    }
    itsBuffer(Range::all(), itsPos) = sample;

    LoVec_dcomplex weight(itsWeight.getBuffer(), itsNrcu, duplicateData);   
    
    dcomplex output = 0; 
    for (int i = 0; i < itsNrcu; i++) {
      output += (2 * real(weight(i)) - weight(i)) * sample(i); // w^H * x(t)
    }
    itsBeamBuffer(itsPos) = output;
    //    itsBeamBuffer(itsPos) = sum( LCSMath::conj( weight ) * itsBuffer(Range::all(), itsPos));
    itsPos = (itsPos + 1) % itsBuffer.cols();

    for (int j = 0; j < getOutputs(); j++) {
      // copy the sample to the outHolders
      itsOutHolders[j]->getBuffer ()[0] = output;
    }
    
    string n = getName ();
    if (iCount++ % 50 == 0 && n == "0" ) {
      if (qm(0)) {
	beamplot (handle_bp, weight, itsNrcu);
      }
      if (qm(1)) {
	spectrumplot(handle_sp, itsBuffer, itsBeamBuffer);
      }
    }
  }
}


void WH_BeamFormer::dump() const
{
//    cout << "WH_BeamFormer " << getName() << " Buffers:" << endl;
//    dcomplex sum = 0;
//    if (getOutputs() > 0) {
//      for (int i=0; i < itsNrcu; i++) {
//        sum = sum + itsOutHolders[i]->getBuffer()[0];
//        //      cout << itsOutHolders[i]->getBuffer()[0] << '.' << endl ;
//      }
//      cout << "Sum of beamformer outputs: "<< sum.real() << endl;
//    }
}


DataHolder* WH_BeamFormer::getInHolder (int channel)
{
  AssertStr (channel < getInputs(), "input channel too high");
  if (channel < getInputs() - 1) {
    return itsInHolders[channel];
  } else {
    return &itsWeight;
  }
}

DataHolder* WH_BeamFormer::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(), "output channel too high");
  return itsOutHolders[channel];
}

void WH_BeamFormer::beamplot (gnuplot_ctrl* handle, const LoVec_dcomplex& w, 
			      const int nrcu)
{
  const int N = 180;                 // The resolution of the sky scan 
  LoMat_double UVpower(N,N);
  UVpower = beam_pattern(w, nrcu, N);

  // plot to file

    gnuplot_close(handle);
    handle = gnuplot_init ();
//    gnuplot_cmd(handle, "set terminal png");
//    char filename[8];
//    sprintf (filename, "set output \"beamshape_%d.png\"", plotCount++);
//    gnuplot_cmd(handle, filename);
    gnuplot_cmd(handle, "set view 0,0");
    gnuplot_splot (handle, UVpower, "Beam pattern");
  //  gnuplot_close(handle);
  //gnuplot_contour_plot (handle, UVpower, "Beam pattern");
}


void WH_BeamFormer::spectrumplot (gnuplot_ctrl* handle, const LoMat_dcomplex& buffer, 
				  const LoVec_dcomplex& beam_buf)
{
  // TODO: Resort the buffer from oldest to newest. Use itsPos. Expensive, but might be
  // necessary.

  // First check the (first element) of the weight vector for useful info
  // Do nothing if it contains only zero
  //  if ( beam_buf(0) != 0 ) {

  int nr = buffer.rows();
  int nc = buffer.cols();
  LoMat_dcomplex o_spec (1, nc); 
  LoMat_dcomplex b_spec (1, nc);

  LoMat_double res0 (1,nc);
  LoMat_double res1 (1,nc);
  
  o_spec = 0;
  b_spec = 0;
  
  for (int i = 0; i < nr; i++) {
    // power spectrum of input signal (add for all antennas)
    // power spectrum = buffer * conj(buffer)
    o_spec (0, Range::all()) += (buffer(i, Range::all()) * (2 * real(buffer(i, Range::all())) - buffer(i, Range::all()))) ;
  }

  // power spectrum of beamformed signal
  b_spec (0, Range::all()) = (beam_buf * (2 * real(beam_buf) - beam_buf)) ;

  // Normalize over a single antenna
  o_spec /= nr;
  b_spec /= nr;

  // Power spectrum before beamforming over first antenna
  res0(0, Range::all()) = 20 * log10 ( sqrt( sqr( real( o_spec( 0, Range::all() ))) + 
					     sqr( imag( o_spec( 0, Range::all() )))));
  // Power spectrum after beamforming
  res1 = 20 * log10 ( sqrt( sqr( real( b_spec  )) + sqr( imag( b_spec ))));
    
  for (int i = 0; i < nc; i++) {
    if ((isnan( res0(0,i) )) || (res0(0,i) < -700)) {
      res0(0,i) = -700;
    }
    if ((isnan( res1(0,i) )) || (res1(0,i) < -700)) {
      res1(0,i) = -700;
    }
  }
  
  // Get the minimum RFI attanuation from the power spectrum 
  // Assume the highest power before and after beamforming are
  // RFI sources.
  double rfi_attanuation = -4000;
  for (int i = 0; i < nc; i++) {
    if (((res1(0,i) - res0(0,i)) > rfi_attanuation) && (res0(0,i) != -700) && (res1(0,i) != -700)) {
      rfi_attanuation = -(res0(0,i) - res1(0,i));
    }
  }
  if (rfi_attanuation != -4000) {
    cout << "RFI attanuation : " << rfi_attanuation <<"dB" << endl;
  }
  
  // saving pictures to file
  gnuplot_close(handle);
  handle = gnuplot_init ();
  gnuplot_setstyle(handle, "lines");
  
  //      gnuplot_cmd(handle, "set terminal png");
  //      char filename[8];
  //      sprintf (filename, "set output \"spectrum_%d.png\"", plotCount++);
  //      gnuplot_cmd(handle, filename);
  gnuplot_cmd(handle, "set multiplot");
  gnuplot_cmd(handle, "set yrange [-350:000]");
  
  gnuplot_plot_x(handle, res0.data(), nc,  "Spectrum before beamforming");
  gnuplot_plot_x(handle, res1.data(), nc,  "Spectrum after beamforming");
  //      gnuplot_cmd(handle, "set nomultiplot");
  //      gnuplot_close(handle);
  //  }
}

void WH_BeamFormer::ml_trans_edge (const LoVec_dcomplex& w, LoMat_double& ref, const int time, 
		    const int nrcu, const int N)
{
  // Calculate d(B(phi)) / d(t)
  // or the gain fluctuations of the beampattern in time
  LoMat_double current(N,N);
  LoMat_double diff(N,N);
  current = beam_pattern (w, nrcu, N);
 
  diff = current - ref;
  diff = diff / time;
}

LoMat_double WH_BeamFormer::beam_pattern (const LoVec_dcomplex& w, const int nrcu, const int N)
{
  // Calculate a beampattern from a weight vector and the known antenna positions
  int index = 0;	
  double theta ;
  double phi ;
  dcomplex pwr;
  LoMat_dcomplex UVplane (N, N);
  LoMat_double UVpower (N, N);
  UVplane = 0;
  
  LoVec_double px(nrcu);
  LoVec_double py(nrcu);
  px = itsArray.getPointX ();
  py = itsArray.getPointY ();
  dcomplex j = dcomplex (0,1);

  // sin(phi) cos(phi) and sin(theta) are calculated outside the inner most loop
  double cosphi;
  double sinphi;
  double sintheta;

  for (int u = 0; u < N; u++) {
    double u1 = -1 + (double)2*u/N;
    for (int v = 0; v < N; v++) {
      double v1 = -1 + (double)2*v/N;
      
      theta = asin(sqrt(pow2(u1)+pow2(v1))); 
      phi = atan2(u1,v1);
      cosphi = cos(phi);
      sinphi = sin(phi);
      sintheta = sin(theta);

      //      if (sqrt (pow ((2 * (double) u / N) - 1, 2) + pow ((2 * (double) v / N) - 1, 2)) <= 1) {
      //      if (sqrt (pow2 ((2 * (double) u / N) - 1) + pow2 ((2 * (double) v / N) - 1)) <= 1) {
      //      sqrt(x) <= 1 <=> x <= 1^2 == 1
      if ((pow2 ((2 * (double) u / N) - 1) + pow2 ((2 * (double) v / N) - 1)) <= 1) {
	for (int k = 0; k < nrcu; k++) {		

	  // dcomplex pwr = exp(-1*j*2*M_PI*(px(k)*sin(theta)*cos(phi)+py(k)*sin(phi)*sin(theta)));
	  // px(k)*sin(theta) * cos(phi) + py(k)*sin(phi)*sin(theta)) = 
	  // sin(theta) * ( px (k) * cos(phi) + py(k)*sin(phi) )
	  pwr = exp(-1*j*2*M_PI*(sintheta * (px(k)*cosphi+py(k)*sinphi)));

	  UVplane (u, v) += pwr * conj (w (k)); 
	}
	UVpower (u, v) = abs (UVplane (u, v));
	index++;
      }
    }
  }
  UVpower /= max (UVpower);
  
  for (int u = 0; u < N; u++) {
    for (int v = 0; v < N; v++) {
      if (UVpower (u, v) > 0) {
	UVpower (u, v) = 20 * log10 (UVpower (u, v));
      } else {
	// place floor at -30 dB so GNUplot can actually plot it
	UVpower (u, v) = -30;
      }
    }
  }
  return UVpower;
}
