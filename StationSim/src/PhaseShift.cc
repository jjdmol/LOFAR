//  PhaseShift.cc:
//
//  Copyright (C) 2002
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
//

#include <StationSim/PhaseShift.h>


namespace PhaseShift 
{
  using namespace blitz;

  // Applies antenna phase to a source & antenna pair.
  // 
  // input_signal is the source signal,
  // doa is its direction-of-arrival at the antenna;
  // freq_shift is a precomputed frequency shift vector:
  //    (((i - nfft/2.0 + 1)*freq_resolution + center_freq)/center_freq);
  // fft_plans are fftw forward/reverse transform plans (use {0,0} to have
  // them created locally)
  LoVec_dcomplex applyAntennaPhase (int nfft, 
									int nbins, 
									const LoVec_dcomplex& input_signal,
									const LoVec_double& freq_shift,
									double doa,
									FFTW::Plan invplan) 
  {
    // compute the freq shift vector for this antenna
    LoVec_dcomplex fs (nfft);
	fs = exp (dcomplex (0, 1) * freq_shift * doa);
	LoVec_dcomplex temp(nfft / 2);

	// make copy of input signal
    LoVec_dcomplex result = input_signal.copy ();
    
	// apply frequency shift repeatedly to each bin of signal
    for (int i0 = 0; i0 < input_signal.size (); i0 += nfft) {
       result (Range (i0, i0 + nfft - 1)) *= fs;
	}

	dprintf1 (2) ("inverse fft length %d bins %d\n", nfft, nbins);

	//AG: Do a ifftshift, put the DC component in the middle of the band
	temp = result (Range (nfft / 2, nfft - 1));
	result (Range (nfft / 2, nfft - 1)) = result (Range (0, nfft / 2 - 1));
	result (Range (0, nfft / 2 - 1)) = temp;

    // do inverse fft
	FFTW::inverse_fft (result, nfft, nbins, invplan);


 	return result;
  }

  // Phase-shifts a source for the given array configuration
  // Returns [n_antennas,signal_length] matrix
  // freq_shift is a precomputed frequency shift vector, determined via
  // bandwidth and center freq.:
  //    LoVec_double freq_shift(nfft);
  //    firstIndex i;//    freq_shift = (((i - nfft/2.0 + 1)*freq_res + centerFreq)/centerFreq;
  LoMat_dcomplex phaseShift (int nfft, 
							 const LoVec_double& source, 
							 const double theta, 
							 const double phi, 
							 const ArrayConfig& antennas, 
							 const LoVec_double& freq_shift, 
							 FFTW::Plan fwdplan,
							 FFTW::Plan invplan) 
  {
    // truncate input signal data to a multiple of nfft (nbins*nfft)
    int siglen = source.size ();
    int nbins = siglen / nfft;	// number of fft bins
	LoVec_dcomplex temp(nfft / 2);

    siglen = nbins * nfft;
    LoVec_double datavec = source (Range (0, siglen - 1));

    dprintf1 (1) ("phase shifting source, siglen=%d, nfft=%d, nbins=%d\n", siglen, nfft, nbins);

    // do forward fft of source
    LoVec_dcomplex cdata = FFTW::forward_fft (source (Range (0, siglen - 1)), nfft, nbins, fwdplan);

	//AG: Do a fftshift, put the DC component in the middle of the band
	temp = cdata (Range (nfft / 2, nfft - 1));
	cdata (Range (nfft / 2, nfft - 1)) = cdata (Range (0, nfft / 2 - 1));
	cdata (Range (0, nfft / 2 - 1)) = temp;

    dprintf1 (1) ("foward fft done\n");

    // compute vector of doas (per each antenna)
    LoVec_double doa = DOA (antennas.getPointX (), antennas.getPointY (), theta, phi, nfft);

    // this is the output matrix: one column of signal per each antenna
    LoMat_dcomplex phased_signal (antennas.size (), siglen);

    dprintf1 (1) ("applying phases for %d antennas\n", antennas.size ());

    // loop over all antennas  
    for (int iant = 0; iant < antennas.size (); iant++) {
      phased_signal (iant, Range::all ()) = applyAntennaPhase (nfft, 
															   nbins, 
															   cdata, 
															   freq_shift,
															   doa (iant), 
															   invplan);
    }
    dprintf1 (1) ("done\n");

    return phased_signal;
  }
  
  // Precomputes the freq_shift vector for phaseShift(), above
  //    LoVec_double freq_shift(nfft);
  //    firstIndex i;
  //    freq_shift = (((i - nfft/2.0 + 1)*freq_res + centerFreq)/centerFreq;
  LoVec_double getFreqShift (double bandwidth, double center_freq, int nfft) 
  {
    LoVec_double fs (nfft);
	//    LoVec_double fs2 (nfft);

	// JD/AG: look at the phased array book
	fs = ((tensor::i - (nfft / 2.0 + 1.0) + 1) * (bandwidth / nfft) + center_freq) / center_freq;

	//	fs2 = ((tensor::i - nfft / 2.0 + 1) * (bandwidth / nfft) + center_freq) / center_freq; // Oleg's original code
    return fs;
  }

  LoVec_double DOA (const LoVec_double& px, const LoVec_double& py, double theta, double phi, int nfft) 
  {
    FailWhen1 (px.size () != py.size (), "vector size mismatch");
    LoVec_double res (px.size ());

    res = -2 * M_PI * (px * sin (theta) * cos (phi) + py * sin (theta) * sin (phi));

	// JD/AG: put in the divide by nfft
	//	return res / nfft;
    return res;
  }
};				// namespace PhaseShift
