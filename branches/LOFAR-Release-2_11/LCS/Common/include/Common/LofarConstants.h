//# LofarConstants.h
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_COMMON_LOFARCONSTANTS_H
#define LOFAR_COMMON_LOFARCONSTANTS_H

// \file
//

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/lofar_bitset.h>

namespace LOFAR {
	static const int N_POL					  = 2;				// Number of polarizations
	static const int NR_SUBRACKS_PER_CABINET  = 2;
	static const int NR_RSPBOARDS_PER_SUBRACK = 4;
	static const int NR_TBBOARDS_PER_RSPBOARD = 2;
	static const int NR_RCUS_PER_RSPBOARD     = 8;
	static const int NR_BLPS_PER_RSPBOARD     = 4;
	static const int NR_ANTENNAS_PER_RSPBOARD = (NR_RCUS_PER_RSPBOARD / N_POL);
	static const int NR_RCUS_PER_TBBOARD      = (NR_TBBOARDS_PER_RSPBOARD * NR_RCUS_PER_RSPBOARD);
	static const int NR_RCUS_PER_SUBRACK      = (NR_RCUS_PER_RSPBOARD * NR_RSPBOARDS_PER_SUBRACK);
	static const int NR_RCUS_PER_CABINET      = (NR_RCUS_PER_SUBRACK  * NR_SUBRACKS_PER_CABINET);
	static const int N_HBA_ELEM_PER_TILE	  = 16;						// Number of High Band antenna per tile

	static const int MAX_ANTENNAS			  = 96;						// Max number antenna's of each type.
	static const int MAX_RCUS				  = (MAX_ANTENNAS * N_POL);	// Max number of RCU's in one station
	static const int MAX_SUBBANDS			  = 512;					// Number of subbands that are created
	static const int MAX_RSPBOARDS			  = (MAX_RCUS / NR_RCUS_PER_RSPBOARD);

	static const int NR_RCU_MODES			  = 7;
	static const int NR_SPECTRAL_WINDOWS	  = 5;

	static const int MAX_BITS_PER_SAMPLE	  = 16;
	static const int MIN_BITS_PER_SAMPLE	  = 4;
	static const int MAX_NR_BM_BANKS	      = (MAX_BITS_PER_SAMPLE / MIN_BITS_PER_SAMPLE);
}

#endif
