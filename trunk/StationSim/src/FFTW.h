//  FFTW.h:
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

#ifndef DATAGEN_WFFT_H
#define DATAGEN_WFFT_H 1

#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <fftw.h>

namespace FFTW 
{
  typedef fftw_plan Plan;
  typedef fftw_direction Direction;

  // Does in-place forward or inverse FFTs (nbins transforms of length 
  // nfft each)
  // data must be an nbins*nfft vector.
  // If plan is =0, creates & destroys a local plan, otherwise uses the 
  // supplied plan (must be a valid plan for an in-place complex transform).
  // If plan=0, then a direction must be supplied.
  void do_fft (LoVec_dcomplex& data, 
			   int nfft, 
			   int nbins, 
			   Plan plan,
			   Direction dir);

  // simple aliases for do_fft
  inline void forward_fft (LoVec_dcomplex& data, 
						   int nfft, 
						   int nbins,
						   Plan plan = 0) 
	{
	  do_fft (data, nfft, nbins, plan, FFTW_FORWARD);
	}
 
  inline void inverse_fft (LoVec_dcomplex& data, 
						   int nfft, 
						   int nbins,
						   Plan plan = 0) 
	{
	  do_fft (data, nfft, nbins, plan, FFTW_BACKWARD);
	}

  // Version of forward transform for all-real data
  // (imaginary part is filled in as being all =0). 
  // Plan must be either 0 or a valid forward, in-place plan
  LoVec_dcomplex forward_fft (const LoVec_double& data, 
							  int nfft,
							  int nbins, 
							  Plan plan = 0);


  // Initializes a forward, in-place plan
  inline Plan initPlan (int nfft, Direction dir, int flags = FFTW_IN_PLACE) 
	{
	  return fftw_create_plan (nfft, dir, flags);
	}

  // Initializes a forward, in-place plan
  inline Plan initForwardPlan (int nfft) 
	{
	  return initPlan (nfft, FFTW_FORWARD);
	}

  // Initializes a backward, in-place plan
  inline Plan initInversePlan (int nfft) 
	{
	  return initPlan (nfft, FFTW_BACKWARD);
	}

  // Destroys a plan
  inline void deletePlan (Plan plan) 
	{
	  return fftw_destroy_plan (plan);
	}
};				// namespace FFTW

#endif
