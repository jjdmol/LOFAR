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



//#include <math.h>              // for isfinite(x)
#include <stdio.h>             // for sprintf

#include <StationSim/WH_BeamFormer.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_vector.h>
#include <Math/LCSMath.h>

//#define TOSCREEN 1

using namespace blitz;

WH_BeamFormer::WH_BeamFormer (const string& name,
							  unsigned int nin,
							  unsigned int nout,
							  unsigned int nrcu,
							  unsigned int nsubband,
							  bool tapstream)
  : WorkHolder    (nin, nout, name,"WH_BeamFormer"),  // Check number of inputs and outputs
	itsInHolders  (0),
	itsOutHolders (0),
	itsWeight     ("weights", nrcu, nsubband),
	itsNrcu       (nrcu),
	itsNsubband   (nsubband),
	itsW          (nrcu, nsubband),
	itsTapStream  (tapstream),
	itsCount      (0)
{
  char str[8];
  if (nin > 0) {
    itsInHolders = new DH_SampleC* [nin - 1];
  }
  for (unsigned int i = 0; i < nin - 1; i++) {
    sprintf (str, "%d",i);
    itsInHolders[i] = new DH_SampleC (string ("in_") + str, nrcu, nsubband);
  }
  // idem for the number of outputs
  if (nout > 0) {
    itsOutHolders = new DH_SampleC* [nout];
  }
  for (unsigned int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    itsOutHolders[i] = new DH_SampleC (string ("out_") + str, nsubband, 1);

	if (itsTapStream) {
	  itsOutHolders [i]->setOutFile (string ("BF_") + str + string (".dat"));
	}
  }

  itsW = 0;
}


WH_BeamFormer::~WH_BeamFormer()
{
  for (int i = 0; i < getInputs () - 1; i++) {
    delete itsInHolders[i];
  }
  delete [] itsInHolders;
  for (int i = 0; i < getOutputs(); i++) {
    delete itsOutHolders[i];
  }
  delete [] itsOutHolders;
}


WorkHolder* WH_BeamFormer::construct (const string& name,
				      int ninput, int noutput, const ParamBlock& params)
{
  return new WH_BeamFormer (name, ninput, noutput,
							params.getInt ("nrcu", 92),
							params.getInt ("nsubband", 16),
							false);
}

WH_BeamFormer* WH_BeamFormer::make (const string& name) const
{
  return new WH_BeamFormer (name, getInputs(), getOutputs(), itsNrcu, itsNsubband,
							itsTapStream);
}


void WH_BeamFormer::process()
{
  if (getOutputs () > 0) {
	if (itsWeight.doHandle ()) {
	  LoMat_dcomplex weight (itsWeight.getBuffer(), shape (itsNrcu, itsNsubband), duplicateData); 
	  itsW = weight;
	}

	if (itsInHolders[0]->doHandle ()) {
	  LoMat_dcomplex buf (itsInHolders[0]->getBuffer (), shape (itsNrcu, itsNsubband), duplicateData);
	  LoVec_dcomplex output (itsNsubband); 
	  output = 0.0;
	  
	  for (int s = 0; s < itsNsubband; s++) {
		for (int r = 0; r < itsNrcu; r++) {
		  output (s) += conj (itsW (r, s)) * buf (r, s);
		}
	  }
	  
	  // DEBUG
	  cout << "WH_BeamFormer : " << itsCount++  << endl;

	  for (int i = 0; i < getOutputs(); i++) {
		memcpy (itsOutHolders[i]->getBuffer (), output.data (), sizeof (DH_SampleC::BufferType) * itsNsubband);
	  }
	}
  }
}

void WH_BeamFormer::dump() const
{
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





// void WH_BeamFormer::beamplot (gnuplot_ctrl* handle, const LoVec_dcomplex& w, 
// 			      const int nrcu)
// {
//   const int N = 180;                 // The resolution of the sky scan 
//   LoMat_double UVpower(N,N);

// #ifndef TOSCREEN
//   gnuplot_close(handle);
// #endif

//   UVpower = beam_pattern(w, nrcu, N);

//   // plot to file
// #ifndef TOSCREEN
//   handle = gnuplot_init ();
//   gnuplot_cmd(handle, "set terminal png");
//   char filename[25];
//   sprintf (filename, "set output \"beamshape_%d.png\"", plotCount);
//   gnuplot_cmd(handle, filename);
// #endif

//   gnuplot_cmd(handle, "set view 0,0");
//   gnuplot_splot (handle, UVpower, "Beam pattern");

//   //gnuplot_contour_plot (handle, UVpower, "Beam pattern");
// }

// void WH_BeamFormer::spectrumplot (gnuplot_ctrl* handle, const LoVec_dcomplex& look_buf, 
// 				  const LoVec_dcomplex& null_buf, const int pos)
// {
//   int nc = look_buf.size();
//   LoMat_dcomplex o_spec (1, nc); 
//   LoMat_dcomplex b_spec (1, nc);

//   LoVec_dcomplex cont_look_buf(look_buf.shape());
//   LoVec_dcomplex cont_null_buf(null_buf.shape());

//   // create the contigeous buffers
//   cont_look_buf(Range(0, nc - pos - 2)) = 
//     look_buf(Range(pos + 1, nc - 1)) ;
//   cont_look_buf(Range(nc - pos - 1, nc - 1)) =
//     look_buf(Range(0, pos)) ;

//   cont_null_buf(Range(0, nc - pos - 2)) = 
//     null_buf(Range(pos + 1, nc - 1)) ;
//   cont_null_buf(Range(nc - pos - 1, nc - 1)) =
//     null_buf(Range(0, pos)) ;

//   LoMat_double res0 (1,nc);
//   LoMat_double res1 (1,nc);
  
//   o_spec = 0;
//   b_spec = 0;
  

//   o_spec (0, Range::all()) = (cont_look_buf * (2 * real(cont_look_buf) - cont_look_buf)) ;

//   // power spectrum of beamformed signal
//   b_spec (0, Range::all()) = (cont_null_buf * (2 * real(cont_null_buf) - cont_null_buf)) ;


// //    res0(0, Range::all()) = 20 * log10 ( sqrt( sqr( real( o_spec( 0, Range::all() ))) + 
// //  					     sqr( imag( o_spec( 0, Range::all() )))));

//   // Power spectrum before beamforming over first antenna
//   res0 = 20 * log10 ( sqrt ( sqr ( real ( o_spec )) +
// 			     sqr ( imag ( o_spec )))) ;

//   // Power spectrum after beamforming
//   res1 = 20 * log10 ( sqrt ( sqr ( real( b_spec  )) + 
// 			     sqr( imag( b_spec ))));
    
//   for (int i = 0; i < nc; i++) {
//     if ((isnan( res0(0,i) ))) { // || (res0(0,i) < -70)) {
//       res0(0,i) = -70;
//     }
//     if ((isnan( res1(0,i) ))) { // || (res1(0,i) < -70)) {
//       res1(0,i) = -70;
//     }
//   }
  
//   // Get the minimum RFI attanuation from the power spectrum 
//   // Assume the highest power before and after beamforming are
//   // RFI sources.
//   double rfi_attanuation = -4000;
//   for (int i = 0; i < nc; i++) {
//     if (((res1(0,i) - res0(0,i)) > rfi_attanuation) && (res0(0,i) != -700) && (res1(0,i) != -700)) {
//       rfi_attanuation = -(res0(0,i) - res1(0,i));
//     }
//   }
//   if (rfi_attanuation != -4000) {
//     cout << "RFI attanuation : " << rfi_attanuation <<"dB" << endl;
//   }
  
//   // saving pictures to file

// #ifndef TOSCREEN 
//   gnuplot_close(handle);
//   handle = gnuplot_init ();
//   gnuplot_cmd(handle, "set terminal png");
//   char filename[8];
//   sprintf (filename, "set output \"spectrum_%d.png\"", plotCount);
//   gnuplot_cmd(handle, filename);
//   gnuplot_cmd(handle, "set multiplot");
// #endif
//   gnuplot_setstyle(handle, "lines");
//   gnuplot_cmd(handle, "set yrange [-400:200]");
  
//   gnuplot_plot_x(handle, res0.data(), nc,  "Spectrum before nulling");
//   gnuplot_plot_x(handle, res1.data(), nc,  "Spectrum after nulling");
// #ifndef TOSCREEN
//   gnuplot_cmd(handle, "set nomultiplot");
// #endif
// }

// void WH_BeamFormer::ml_trans_edge (gnuplot_ctrl* handle, const LoVec_dcomplex& w, LoVec_dcomplex& ref, 
// 				   const int time, const int nrcu, const int N, LoMat_double& x_sum)
// {
//   // Calculate d(B(phi)) / d(t)
//   // or the gain fluctuations of the beampattern in time
//   LoMat_double current(N,N);
//   LoMat_double nonulled(N,N);
//   LoMat_double diff(N,N);

// #ifndef TOSCREEN
//   gnuplot_close(handle);
// #endif

//   // calculate the current beam and the reference beamx
//   current = beam_pattern (w, nrcu, N);
//   nonulled = beam_pattern (ref, nrcu, N);

//   // calculate the difference compared to the reference beam
//   // (this is the beam without nulling)
//   diff = current - nonulled;
  
//   // x_sum == x' * n (see below)
//   x_sum += diff;

//   // now get the standard deviation from the reference beam
//   // 
//   // s = sqrt ( 1/n * sum (1 to n) (x_i - x')^2 ) 
//   // with x' = 1/n * sum (1 to n) x_i

//   current = x_sum / time; // x'
  
  

// #ifndef TOSCREEN
//   handle = gnuplot_init ();
//   gnuplot_cmd(handle, "set terminal png");
//   char filename[25];
//   sprintf (filename, "set output \"beamvariance_%d.png\"", plotCount);
//   gnuplot_cmd(handle, filename);
// #endif

//   gnuplot_cmd(handle, "set view 0,0");
//   gnuplot_splot (handle, diff, "Main lobe variance over time");
// }

// LoMat_double WH_BeamFormer::beam_pattern (const LoVec_dcomplex& w, const int nrcu, const int N)
// {
//   // Calculate a beampattern from a weight vector and the known antenna positions
//   int index = 0;	
//   double theta ;
//   double phi ;
//   dcomplex pwr;
//   LoMat_dcomplex UVplane (N, N);
//   LoMat_double UVpower (N, N);
//   UVplane = 0;
  
//   LoVec_double px(nrcu);
//   LoVec_double py(nrcu);
//   px = itsArray.getPointX ();
//   py = itsArray.getPointY ();
//   dcomplex j = dcomplex (0,1);

//   // sin(phi) cos(phi) and sin(theta) are calculated outside the inner most loop
//   double cosphi;
//   double sinphi;
//   double sintheta;

//   for (int u = 0; u < N; u++) {
//     double u1 = -1 + (double)2*u/N;
//     for (int v = 0; v < N; v++) {
//       double v1 = -1 + (double)2*v/N;
      
//       theta = asin(sqrt(pow2(u1)+pow2(v1))); 
//       phi = atan2(u1,v1);
//       cosphi = cos(phi);
//       sinphi = sin(phi);
//       sintheta = sin(theta);

//       //      if (sqrt (pow ((2 * (double) u / N) - 1, 2) + pow ((2 * (double) v / N) - 1, 2)) <= 1) {
//       //      if (sqrt (pow2 ((2 * (double) u / N) - 1) + pow2 ((2 * (double) v / N) - 1)) <= 1) {
//       //      sqrt(x) <= 1 <=> x <= 1^2 == 1
//       if ((pow2 ((2 * (double) u / N) - 1) + pow2 ((2 * (double) v / N) - 1)) <= 1) {
// 	for (int k = 0; k < nrcu; k++) {		

// 	  // dcomplex pwr = exp(-1*j*2*M_PI*(px(k)*sin(theta)*cos(phi)+py(k)*sin(phi)*sin(theta)));
// 	  // px(k)*sin(theta) * cos(phi) + py(k)*sin(phi)*sin(theta)) = 
// 	  // sin(theta) * ( px (k) * cos(phi) + py(k)*sin(phi) )
// 	  pwr = exp(-2*M_PI*j*(sintheta * (px(k)*cosphi+py(k)*sinphi)));

// 	  UVplane (u, v) += pwr * conj (w (k)); 
// 	}
// 	UVpower (u, v) = abs (UVplane (u, v));
// 	index++;
//       }
//     }
//   }

//   UVpower /= max (UVpower);
  
//   for (int u = 0; u < N; u++) {
//     for (int v = 0; v < N; v++) {
//       if (UVpower (u, v) > 0) {
// 	UVpower (u, v) = 20 * log10 (UVpower (u, v));
//       } else {
// 	// place floor at -30 dB so GNUplot can actually plot it
// 	UVpower (u, v) = -30;
//       }
//     }
//   }
//   return UVpower;
// }
