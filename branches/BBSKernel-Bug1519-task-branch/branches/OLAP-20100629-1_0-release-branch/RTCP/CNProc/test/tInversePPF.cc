#include <lofar_config.h>

#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <Interface/TransposedBeamFormedData.h>
#include <Interface/InverseFilteredData.h>
#include <Interface/Align.h>
#include <Interface/AlignedStdAllocator.h>
#include <vector>


// First do an inverse FFT, next do the FIR filter.

//#undef HAVE_FFTW3

#if defined HAVE_FFTW3
#include <fftw3.h>
//#error using fftw3
#elif defined HAVE_FFTW2
#include <fftw.h>
#include <rfftw.h>
//#error using fftw2
#else
#error Should have FFTW3 or FFTW2 installed
#endif

using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace LOFAR::TYPES;

unsigned nrBeams = 1;
unsigned nrSubbands = 40; // 248;
unsigned nrChannels = 1; // for the NuMoon pipeline, there are no separate channels.
unsigned nrSamplesPerIntegration = 768 * 256;
unsigned onStationFilterSize = 64; // 512;

// The #samples per integration time is too large for the entire buffer to fit into memory.
// Therefore, we divide the time into a major and a minor time.
unsigned majorTime = 16; // in how many pieces do we split the samples?
unsigned minorTime = nrSamplesPerIntegration / majorTime;

/*
  Er zijn 3 opties:

 - complex to complex FFT

 - complex to real FFT *** deze gebruik ik nu ***
   * sloopt input data, dus die moet je elke keer opnieuw maken
   * input moet in "half complex" formaat zijn
     input must be stored in the following format: r0, r1, r2, ..., rn/2, i(n+1)/2-1, ..., i2, i1 

 - complex to real FFT, multidimensionale versie met N=1
 * sloopt input data ook
 * normaal input formaat

 TODO: welke heeft de beste performance?
*/

int main() {
  // [nrBeams][nrSubbands][nrChannels][nrSamplesPerIntegration | 2][NR_POLARIZATIONS]
  TransposedBeamFormedData transposedData(nrBeams, nrSubbands, nrChannels, minorTime);
  transposedData.allocate();

  cerr << "size of transposedData = " 
       << ((nrBeams * nrSubbands * minorTime * NR_POLARIZATIONS * sizeof(fcomplex)) / (1024*1024)) 
       << " MB" << endl;

  // buffer is too large to fit in memory.
  // Therefore, allocate a smaller buffer, and perform all operations several times
  InverseFilteredData invertedData(nrBeams, minorTime, onStationFilterSize*2);
  invertedData.allocate();

  cerr << "size of invertedData = " 
       << ((nrBeams * minorTime * onStationFilterSize*2 * NR_POLARIZATIONS * sizeof(float)) / (1024*1024)) 
       << " MB" << endl;

#if defined HAVE_FFTW3
  fftwf_plan plan;
#elif defined HAVE_FFTW2
  fftw_plan  plan;
#endif

#if defined HAVE_FFTW3
  fftwf_complex *buf = static_cast<fftwf_complex *>(fftwf_malloc(onStationFilterSize * sizeof(fftwf_complex)));
  if (buf == 0) {
	  cerr << "Out of memory" << endl;
	  exit(1);
  }
  plan = fftwf_plan_dft_1d(onStationFilterSize, buf, buf, FFTW_BACKWARD, FFTW_ESTIMATE); // in-place
#elif defined HAVE_FFTW2
  std::vector<float, AlignedStdAllocator<float, 32> > buf(onStationFilterSize*2);

  plan = rfftw_create_plan(onStationFilterSize*2, FFTW_BACKWARD, FFTW_ESTIMATE);
#endif

  float* fftInData = (float*) buf;

  NSTimer fftInTimer("create FFT input", true);
  NSTimer fftTimer("FFT", true);

  for(unsigned beam=0; beam < nrBeams; beam++) {
    for(unsigned pol=0; pol < NR_POLARIZATIONS; pol++) {
      for(unsigned major=0; major < majorTime; major++) {
	for(unsigned minor=0; minor < minorTime; minor++) {
	
	  fftInTimer.start();

	  // fill input buffer, using "half complex" format
	  for(unsigned sb=0; sb < onStationFilterSize/2; sb++) {
	    
	    if(sb < nrSubbands) { // for now, just assume we use subbands 0..255. In reality, there can be gaps in this list.
	      fftInData[sb]                                               = real(transposedData.samples[beam][sb][0][minor][pol]); // real
	      fftInData[sb+onStationFilterSize]                           = imag(transposedData.samples[beam][sb][0][minor][pol]); // imag
// todo ook spiegelen!
	      // lower half of the input vector is the complex conjugate of the upper half
	      fftInData[sb + onStationFilterSize/2]                       =  real(transposedData.samples[beam][sb][0][minor][pol]); // real
	      fftInData[sb + onStationFilterSize + onStationFilterSize/2] = -imag(transposedData.samples[beam][sb][0][minor][pol]); // imag
	    } else {
	      fftInData[sb]                                               =  0.0f; // real
	      fftInData[sb+onStationFilterSize]                           =  0.0f; // imag
	      fftInData[sb + onStationFilterSize/2]                       =  0.0f; // real
	      fftInData[sb + onStationFilterSize + onStationFilterSize/2] = -0.0f; // imag
	    }
	  }

	  fftInTimer.stop();

	  fftTimer.start();

#if defined HAVE_FFTW3
	  fftwf_execute(plan);
#elif defined HAVE_FFTW2
	  // Do the inverse FFT. NB: this call destoys the input data.
	  rfftw_one(plan, (fftw_real*) fftInData.data(), (fftw_real*) invertedData.samples[beam][pol][minor].origin());
#endif

	  fftTimer.stop();
	}
      }
    }
  }
  
#if defined HAVE_FFTW3
  fftwf_destroy_plan(plan);
  fftwf_free(buf);
#elif defined HAVE_FFTW2
  fftw_destroy_plan(plan);
#endif

  return 0;
}
