#ifndef BGL_CORRELATOR_H
#define BGL_CORRELATOR_H

#include <TFC_Interface/TFC_Config.h>

namespace LOFAR {

typedef fcomplex stationInputType[NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS];
typedef fcomplex CorrelatedOutputType[NR_POLARIZATIONS][NR_POLARIZATIONS];

extern "C" {
  void _correlate_2x2(const stationInputType *S0,
		      const stationInputType *S1,
		      const stationInputType *S2,
		      const stationInputType *S3,
		      CorrelatedOutputType *S0_S2,
		      CorrelatedOutputType *S1_S2,
		      CorrelatedOutputType *S0_S3,
		      CorrelatedOutputType *S1_S3);
  
  void _correlate_3x2(const stationInputType *S0,
		      const stationInputType *S1,
		      const stationInputType *S2,
		      const stationInputType *S3,
		      const stationInputType *S4,
		      CorrelatedOutputType *S0_S2,
		      CorrelatedOutputType *S1_S2,
		      CorrelatedOutputType *S0_S3,
		      CorrelatedOutputType *S1_S3,
		      CorrelatedOutputType *S0_S4,
		      CorrelatedOutputType *S1_S4);

  void _auto_correlate_1_and_2(const stationInputType *S0,
			       const stationInputType *S1,
			       CorrelatedOutputType *S0_S0,
			       CorrelatedOutputType *S0_S1,
			       CorrelatedOutputType *S1_S1);

  int _auto_correlate_1x1(const stationInputType *S0,
			  CorrelatedOutputType *S0_S0);
};

}
#endif
