//#  Correlator.h: header files for BGL assembly
//#
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_CORRELATOR_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_CORRELATOR_H

#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/DH_Visibilities.h>

namespace LOFAR {

typedef fcomplex stationInputType[NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS];
typedef fcomplex CorrelatedOutputType[NR_POLARIZATIONS][NR_POLARIZATIONS];

extern "C" {
  void _correlate_2x2(const stationInputType *S0,
		      const stationInputType *S1,
		      const stationInputType *S2,
		      const stationInputType *S3,
		      CorrelatedOutputType *S0_S2,
		      CorrelatedOutputType *S0_S3,
		      CorrelatedOutputType *S1_S2,
		      CorrelatedOutputType *S1_S3);
  
  void _correlate_3x2(const stationInputType *S0,
		      const stationInputType *S1,
		      const stationInputType *S2,
		      const stationInputType *S3,
		      const stationInputType *S4,
		      CorrelatedOutputType *S0_S3,
		      CorrelatedOutputType *S0_S4,
		      CorrelatedOutputType *S1_S3,
		      CorrelatedOutputType *S1_S4,
		      CorrelatedOutputType *S2_S3,
		      CorrelatedOutputType *S2_S4);

  void _auto_correlate_1(const stationInputType *S0,
			 CorrelatedOutputType *S0_S0);

  void _auto_correlate_2(const stationInputType *S0,
			 const stationInputType *S1,
			 CorrelatedOutputType *S0_S0,
			 CorrelatedOutputType *S0_S1,
			 CorrelatedOutputType *S1_S1);

  void _auto_correlate_3(const stationInputType *S0,
			 const stationInputType *S1,
			 const stationInputType *S2,
			 CorrelatedOutputType *S0_S1,
			 CorrelatedOutputType *S0_S2,
			 CorrelatedOutputType *S1_S1,
			 CorrelatedOutputType *S1_S2,
			 CorrelatedOutputType *S2_S2);

  void _clear_correlation(CorrelatedOutputType *S0_S0);

  void _post_process_visibilities(
	DH_Visibilities::VisibilitiesType *visibilities,
	DH_Visibilities::NrValidSamplesType *nrValidSamplesCounted,
	const float correlationWeights[NR_SAMPLES_PER_INTEGRATION + 1],
	const float thresholds[NR_BASELINES][NR_SUBBAND_CHANNELS]);
};

}
#endif
