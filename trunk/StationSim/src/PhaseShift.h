//  PhaseShift.h:
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

#ifndef STATIONSIM_PHASESHIFT_H
#define STATIONSIM_PHASESHIFT_H 1

#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <StationSim/FFTW.h>
#include <StationSim/DataGenConfig.h>
#include <Math/LCSMath.h>


namespace PhaseShift 
{
  // Phase-shifts a source for the given array configuration
  // Returns [n_antennas,signal_length] matrix
  // freq_shift can be obtained by getFreqShift, below
  // fft_plans are fftw forward/reverse transform plans (use {0,0} to have
  // them created locally)
  LoMat_dcomplex phaseShift (int nfft, 
							 const LoVec_double& source, 
							 const double theta, 
							 const double phi, 
							 const ArrayConfig& antennas, 
							 const LoVec_double& freq_shift, 
							 FFTW::Plan fwdplan = 0,	     // forward transform plan, if available
							 FFTW::Plan invplan = 0);        // reverse transform plan, if available

  // Applies antenna phase to a source & antenna pair.
  // input_signal is the source signal,
  // doa is its direction-of-arrival at the antenna;
  // freq_shift is a precomputed frequency shift vector:
  //    (((i - nfft/2.0 + 1)*freq_resolution + center_freq)/center_freq);
  // fft_plans are fftw forward/reverse transform plans (use {0,0} to have
  // them created locally)
  LoVec_dcomplex applyAntennaPhase (int nfft, 
									int nbins, 
									const LoVec_dcomplex& input_signal,	// nfft*nbins input signal f-domain)
									const LoVec_double& freq_shift,	    // nfft precomputed frequency shifts
									double doa,	                        // direction of arrival
									FFTW::Plan invplan = 0);            // reverse transform plan

  // freq_shift is a precomputed frequency shift vector, determined via
  // bandwidth and center freq.:
  //    LoVec_double freq_shift(nfft);
  //    firstIndex i;
  //    freq_shift = (((i - nfft/2.0 + 1)*freq_res + centerFreq)/centerFreq;
  LoVec_double getFreqShift (double bandwidth, 
							 double center_freq, 
							 int nfft);

  // computes the direction-of-arrival parameter for given
  // antenna coordinates
  LoVec_double DOA (const LoVec_double& px, 
					const LoVec_double& py,
					double theta, 
					double phi);

}; // namespace PhaseShift

#endif
