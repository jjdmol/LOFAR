
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

// Chris Broekema, november 2002.

#ifndef STATIONSIM_PASTD_H
#define STATIONSIM_PASTD_H

#include <Math/LCSMath.h>
#include <Common/Lorrays.h>
#include <StationSim/GnuPlotInterface.h>

void pastd_step (LoVec_dcomplex& x, LoMat_dcomplex& W, LoVec_double& d_tmp, 
		 const int nmax, const double Beta) ;

int pastd (LoMat_dcomplex fifo, int numsnaps, const int interval, const double beta,
	   LoVec_double Evalue, LoMat_dcomplex Evector);

#endif
