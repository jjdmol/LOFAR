//  FFTW.cc:
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

#include <StationSim/FFTW.h>

using namespace blitz;

namespace FFTW 
{ 
  void do_fft (LoVec_dcomplex& data, 
			   int nfft, 
			   int nbins, 
			   Plan plan,
			   Direction dir) 
  {
    // create local fftw plan if not supplied
    bool created;
    if (plan) {
       created = false;
	} else {
	  plan = initPlan (nfft, dir, FFTW_IN_PLACE);
	}

    int stride = data.stride (0);

	fftw (plan, 
		  nbins, 
		  reinterpret_cast < fftw_complex * >(data.data ()), 
		  stride,
		  nfft * stride, 
		  NULL, 
		  0, 
		  0);

    // destroy local fftw plan
    if (created) {
       deletePlan (plan);
	}
  }

  // transform of all-real data
  LoVec_dcomplex forward_fft (const LoVec_double& data, 
							  int nfft,
							  int nbins, 
							  Plan plan) 
  {
    LoVec_dcomplex cdata (data.size ());
    cdata = data + dcomplex (0, 0);
    forward_fft (cdata, nfft, nbins, plan);
    return cdata;
  }
};				// namespace FFTW
