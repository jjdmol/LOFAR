//#  Beamlets.cc: implementation of the Beamlet class
//#
//#  Copyright (C) 2002-2004
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#include <iostream>
#include <queue>
#include <blitz/array.h>

#include "BeamServerConstants.h"
#include "Beamlets.h"
#include "Beam.h"

#undef NDIM
#define NDIM 3

using namespace blitz;
using namespace LOFAR;
using namespace BS;
using namespace std;

const double speedOfLight = SPEED_OF_LIGHT_MS; // speed of light in meters/sec
const complex<double> I_COMPLEX = complex<double>(0.0,1.0);

//
// Beamlets()
//
Beamlets::Beamlets(int nbeamlets) : m_nbeamlets(nbeamlets)
{
	m_beamlets = new Beamlet[m_nbeamlets];
}

//
// ~Beamlets
//
Beamlets::~Beamlets()
{
	delete [] m_beamlets;
}

//
// get()
//
Beamlet* Beamlets::get(int index) const
{
	if (index < 0 || index >= m_nbeamlets) {
		return (0);
	}

	return (m_beamlets + index);
}

//
// calculate_weights(weightsArray)
// weights [ nrIntervals, rcus, nrBeamlets ]
//
void Beamlets::calculate_weights(Array<complex<double>, 3>& weights)
{
	int 	compute_interval = weights.extent(firstDim);
	Range 	all 			 = Range::all();

	ASSERT(weights.extent(thirdDim) == m_nbeamlets);

	weights = complex<double>(0.0, 0.0); // initialize to zero weights

	for (int bi = 0; bi < m_nbeamlets; bi++) {
		Beamlet* beamlet = m_beamlets + bi;
		if (!beamlet || !beamlet->allocated()) {
			continue;
		}

		const Beam* 			 beam  = beamlet->getBeam();
		const CAL::AntennaGains& gains = beam->getCalibration();

		LOG_DEBUG_STR("gains(" << (gains.isDone()?"":"un") << 
					  "calibrated)[" << bi << "]=" << gains.getGains().shape());

		// get coordinates from beam
		if (!beam) {
			LOG_ERROR(formatString("\nno beam for beamlet %d?\n", bi));
			continue;
		}
		const Array<double,2>& lmn = beam->getLMNCoordinates();
		const Array<double,3>& pos = beam->getSubarray().getAntennaPos();
		// note: pos[antennes, polarisations, coordinates]

		// The check that is commented out below is too strict because
		// pos array can be a subset of the full array because of subarraying
		// 
		// pos.extent(firstDim) == nrcus/MEPHeader::N_POL
		ASSERT(pos.extent(firstDim) <= weights.extent(secondDim) &&
				pos.extent(secondDim) == MEPHeader::N_POL &&
				pos.extent(thirdDim) == NDIM);
		ASSERTSTR(compute_interval == lmn.extent(firstDim), 
						compute_interval << "==" << lmn.extent(firstDim));

		double freq = 0.0;
		freq = beamlet->getSPW().getSubbandFreq(beamlet->subband());

		if (bi == 0) {
			LOG_DEBUG_STR("freq= " << freq);
			LOG_DEBUG_STR("pos="   << pos);
			LOG_DEBUG_STR("lmn="   << lmn);
		}

		complex<double> scaling = -2.0 * M_PI * freq * complex<double>(0.0,1.0) / speedOfLight;

		//
		// calculate (xm - yl + zn) for both polarizations
		// of all elements of the subarray
		//
		for (int antenna = 0; antenna < pos.extent(firstDim); antenna++) {
			for (int pol = 0; pol < pos.extent(secondDim); pol++) {
				// srcrcu is index in subarray
				// destrcu is index in total number of rcu's
				int srcrcu = (antenna * MEPHeader::N_POL) + pol;
				int destrcu = beam->getSubarray().getRCUIndex(antenna, pol);

				if (destrcu < 0) {
					continue;
				}
				weights(all, destrcu, bi) = exp(scaling *
					(( pos(srcrcu / MEPHeader::N_POL, srcrcu % MEPHeader::N_POL, 0) * lmn(all, 0))
					 +(pos(srcrcu / MEPHeader::N_POL, srcrcu % MEPHeader::N_POL, 1) * lmn(all, 1))
					 +(pos(srcrcu / MEPHeader::N_POL, srcrcu % MEPHeader::N_POL, 2) * lmn(all, 2))));
			} // for all pols
		} // for all antennas
	} // for all beamlets

//	LOG_DEBUG_STR("weights=" << weights);
	LOG_DEBUG(formatString("sizeof weights() = %d bytes", weights.size()*sizeof(complex<double>)));
	LOG_DEBUG(formatString("contiguous storage? %s", (weights.isStorageContiguous()?"yes":"no")));
}

