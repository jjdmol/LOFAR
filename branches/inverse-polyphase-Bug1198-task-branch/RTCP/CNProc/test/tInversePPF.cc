#include <lofar_config.h>

#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <Interface/TransposedBeamFormedData.h>
#include <Interface/InverseFilteredData.h>
#include <Interface/Align.h>
#include <Interface/AlignedStdAllocator.h>
#include <vector>
#include <FilterBank.h>
#include <FIR.h>
#include <InversePPF.h>

// #undef HAVE_FFTW3

// On the BG/P, FFT2 uses the double floating point units, FFT3 works, but only uses one.
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

#if defined HAVE_FFTW3
#define fftw_real(x)     ((x)[0])
#define fftw_imag(x)     ((x)[1])
#elif defined HAVE_FFTW2
#define fftw_real(x)     (c_re(x))
#define fftw_imag(x)     (c_im(x))
#endif

using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace LOFAR::TYPES;

#if defined HAVE_FFTW3
fftwf_plan plan;
#elif defined HAVE_FFTW2
fftw_plan plan;
#endif

#include <FIR_OriginalStationPPFWeights.h> // defines originalStationPPFWeights array

static unsigned onStationFilterSize = 1024;
static unsigned nrTaps = 16;

static unsigned nrSubbands = 248;
static unsigned nrChannels = 1; // for the NuMoon pipeline, there are no separate channels.
static unsigned nrSamplesPerIntegration = 768 * 256 / 4; // one quarter of a second
static double sampleRate = 195312.5;
static double centerFrequency = 384 * sampleRate;
static double signalFrequency = centerFrequency - .5 * sampleRate;

static NSTimer firTimer("FIR", true);
static NSTimer fftTimer("FFT", true);
static NSTimer fftInTimer("create FFT input", true);

float originalStationPPFWeightsFloat[1024][16];
float* fftInData;
float* fftOutData;


void initFFT() {
#if defined HAVE_FFTW3
  fftInData = (float*) fftwf_malloc(onStationFilterSize * sizeof(float));
  fftOutData = (float*) fftwf_malloc(onStationFilterSize * sizeof(float));
#elif defined HAVE_FFTW2
  fftInData = (float*) malloc(onStationFilterSize * sizeof(float));
  fftOutData = (float*) malloc(onStationFilterSize * sizeof(float));

  plan = rfftw_create_plan(onStationFilterSize, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
#endif

  if (fftInData == NULL || fftOutData == NULL) {
    cerr << "Out of memory" << endl;
    exit(1);
  }
}

void destroyFFT() {
#if defined HAVE_FFTW3
  fftwf_free(fftInData);
  fftwf_free(fftOutData);
#elif defined HAVE_FFTW2
  rfftw_destroy_plan(plan);
#endif
}


fcomplex toComplex(double phi) {
  double s, c;
  sincos(phi, &s, &c);
  return makefcomplex(c, s);
}

void generateInputSignal(InverseFilteredData& originalData) {
  for (unsigned time = 0; time < nrSamplesPerIntegration*onStationFilterSize; time++) {
    double val = sin(signalFrequency * time / sampleRate);
    originalData.samples[time] = val;
//    double phi = 2 * M_PI * signalFrequency * time / sampleRate;
//    originalData.samples[time] = toComplex(phi);
  }
}

void performStationFFT() {
#if defined HAVE_FFTW3
  fftwf_plan plan = fftwf_plan_r2r_1d(onStationFilterSize, fftInData, fftOutData, FFTW_R2HC, FFTW_ESTIMATE);
  fftwf_execute(plan);
  fftwf_destroy_plan(plan);

#elif defined HAVE_FFTW2
/// @@@ NOT CORRECT YET
  rfftw_one(itsPlan, (fftw_real*) itsFftInData, (fftw_real*) itsFftOutData);
#endif

  // TODO, put data in the right order, go from half complex to normal format

}

int main() {
  double origInputSize = (nrSubbands * nrSamplesPerIntegration * sizeof(fcomplex)) / (1024.0 * 1024.0);
  double fftBufSize = (onStationFilterSize * sizeof(float)) / (1024.0);
  double outputSize = (onStationFilterSize * nrSamplesPerIntegration * sizeof(float)) / (1024.0 * 1024.0);

  cerr << "size of original input data: " << origInputSize << " MB" << endl;
  cerr << "size of FFT buffers: " << fftBufSize << " KB" << endl;
  cerr << "size of output: " << outputSize << " MB" << endl;
  cerr << "total memory usage: " << (origInputSize + outputSize) << " MB" << endl;

  // copy the integer filter constants into a float array.
  for(unsigned filter=0; filter<onStationFilterSize; filter++) {
    for(unsigned tap=0; tap<nrTaps; tap++) {
      originalStationPPFWeightsFloat[filter][tap] = originalStationPPFWeights[filter][tap];
    }
  }

  FilterBank originalStationFilterBank(true, nrTaps, onStationFilterSize, (float*) originalStationPPFWeightsFloat);
  vector<FIR<float> > FIRs;
  FIRs.resize(onStationFilterSize); // Init the FIR filters themselves with the weights of the filterbank.
  for (unsigned chan = 0; chan < onStationFilterSize; chan++) {
    FIRs[chan].initFilter(&originalStationFilterBank, chan);
  }

  // Inverse filtered data is the same data format as the original data, so reuse it here for this test
  InverseFilteredData originalData(nrSamplesPerIntegration, onStationFilterSize);
  originalData.allocate();

  // Inverse filtered data is the same data format as the original data, so reuse it here for this test
  InverseFilteredData originalFilteredData(nrSamplesPerIntegration, onStationFilterSize);
  originalFilteredData.allocate();

  TransposedBeamFormedData transposedBeamFormedData(nrSubbands, nrChannels, nrSamplesPerIntegration);
  transposedBeamFormedData.allocate();

  InverseFilteredData invertedFilteredData(nrSamplesPerIntegration, onStationFilterSize);
  invertedFilteredData.allocate();

  vector<unsigned> subbandList;
  subbandList.resize(nrSubbands);

  // for now, we just select the first 248 subbands.
  for (unsigned sb = 0; sb < nrSubbands; sb++) {
    subbandList[sb] = sb;
  }

  InversePPF inversePPF(subbandList, nrSamplesPerIntegration, nrTaps, onStationFilterSize, true);
  initFFT();

  cerr << "generating input signal" << endl;

  generateInputSignal(originalData);

  cerr << "filtering input signal" << endl;

  for (unsigned time = 0; time < nrSamplesPerIntegration; time++) {
    for (unsigned minorTime = 0; minorTime < onStationFilterSize; minorTime++) {
      unsigned filterIndex = minorTime % onStationFilterSize;
      float sample = originalData.samples[time * onStationFilterSize + minorTime];
      float result = FIRs[filterIndex].processNextSample(sample);
      originalFilteredData.samples[time * onStationFilterSize + minorTime] = result;
    }
  }

  cerr << "performing FFT of input signal" << endl;





  inversePPF.filter(transposedBeamFormedData, invertedFilteredData);

  destroyFFT();
  return 0;
}
