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

//static unsigned nrSubbands = 248;
static unsigned nrSubbands = 4;
static unsigned nrChannels = 1; // for the NuMoon pipeline, there are no separate channels.
//static unsigned nrSamplesPerIntegration = 768 * 256 / 4; // one quarter of a second
static unsigned nrSamplesPerIntegration = 2048; // one quarter of a second
static double sampleRate = 195312.5;
static double centerFrequency = 384 * sampleRate;
static double signalFrequency = centerFrequency - .5 * sampleRate;

float originalStationPPFWeightsFloat[1024][16];
float* fftInData;
float* fftOutData;

static void initFFT() {
#if defined HAVE_FFTW3
  fftInData = (float*) fftwf_malloc(onStationFilterSize * sizeof(float));
  fftOutData = (float*) fftwf_malloc(onStationFilterSize * sizeof(float));

  plan = fftwf_plan_r2r_1d(onStationFilterSize, fftInData, fftOutData, FFTW_R2HC, FFTW_ESTIMATE);
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

static void destroyFFT() {
#if defined HAVE_FFTW3
  fftwf_free(fftInData);
  fftwf_free(fftOutData);
  fftwf_destroy_plan(plan);
#elif defined HAVE_FFTW2
  free(fftInData);
  free(fftOutData);
  rfftw_destroy_plan(plan);
#endif
}

/*
 static fcomplex toComplex(double phi) {
 double s, c;
 sincos(phi, &s, &c);
 return makefcomplex(c, s);
 }
 */

static void generateInputSignal(InverseFilteredData& originalData) {
  for (unsigned time = 0; time < nrSamplesPerIntegration * onStationFilterSize; time++) {
    double val = sin(signalFrequency * time / sampleRate);
    originalData.samples[time] = val;
    //    double phi = 2 * M_PI * signalFrequency * time / sampleRate;
    //    originalData.samples[time] = toComplex(phi);
  }
}

static void performStationFFT(TransposedBeamFormedData& transposedBeamFormedData, vector<unsigned>& subbandList, unsigned time) {
#if defined HAVE_FFTW3
  fftwf_execute(plan);
#elif defined HAVE_FFTW2
  rfftw_one(itsPlan, (fftw_real*) fftInData, (fftw_real*) fftOutData);
#endif

  // Put data in the right order, go from half complex to normal format
  for (unsigned subbandIndex = 0; subbandIndex < subbandList.size(); subbandIndex++) {
    unsigned subband = subbandList[subbandIndex];
    fcomplex sample = makefcomplex(fftOutData[subband], fftOutData[onStationFilterSize - subband]);
    transposedBeamFormedData.samples[subband][0][time] = sample;
  }
}

static void performStationFilter(InverseFilteredData& originalData, vector<FIR<float> >& FIRs, unsigned time) {
  for (unsigned minorTime = 0; minorTime < onStationFilterSize; minorTime++) {
    unsigned filterIndex = minorTime % onStationFilterSize;
    float sample = originalData.samples[time * onStationFilterSize + minorTime];
    float result = FIRs[filterIndex].processNextSample(sample);
    fftInData[minorTime] = result;
    //    fftInData[minorTime] = sample;
  }
}

static void printData(InverseFilteredData& data) {
  for (unsigned time = 0; time < nrSamplesPerIntegration * onStationFilterSize; time++) {
    float sample = data.samples[time];
    fprintf(stdout, "%20.10lf\n", sample);
  }
}

void cepFilterTest() {
  // CEP filter test
  FilterBank fb(true, 16, 256, KAISER);
  boost::multi_array<FIR<fcomplex> , 1> firs(boost::extents[16]);

  // Init the FIR filters themselves with the weights of the filterbank.
  for (unsigned chan = 0; chan < nrChannels; chan++) {
    firs[chan].initFilter(&fb, chan);
  }

  cout << "START CEP WEIGHTS" << endl;
  fb.printWeights();
  cout << "END CEP WEIGHTS" << endl;
}

int main() {

  // copy the integer filter constants into a float array.
  for (unsigned filter = 0; filter < onStationFilterSize; filter++) {
    for (unsigned tap = 0; tap < nrTaps; tap++) {
      originalStationPPFWeightsFloat[filter][tap] = originalStationPPFWeights[filter][tap];
    }
  }

  FilterBank originalStationFilterBank(true, nrTaps, onStationFilterSize, (float*) originalStationPPFWeightsFloat);
  vector<FIR<float> > FIRs;
  FIRs.resize(onStationFilterSize); // Init the FIR filters themselves with the weights of the filterbank.
  for (unsigned chan = 0; chan < onStationFilterSize; chan++) {
    FIRs[chan].initFilter(&originalStationFilterBank, chan);
  }

  cout << "START ORIG STATION WEIGHTS" << endl;
  originalStationFilterBank.printWeights();
  cout << "END ORIG STATION WEIGHTS" << endl;

  // The original data has the same data format as the original data, so reuse it here for this test
  InverseFilteredData originalData(nrSamplesPerIntegration, onStationFilterSize);
  originalData.allocate();

  TransposedBeamFormedData transposedBeamFormedData(nrSubbands, nrChannels, nrSamplesPerIntegration);
  transposedBeamFormedData.allocate();

  InverseFilteredData invertedFilteredData(nrSamplesPerIntegration, onStationFilterSize);
  invertedFilteredData.allocate();

  vector<unsigned> subbandList;
  subbandList.resize(nrSubbands);

  // for now, we just select the first n subbands.
  for (unsigned sb = 0; sb < nrSubbands; sb++) {
    subbandList[sb] = sb;
  }

  InversePPF inversePPF(subbandList, nrSamplesPerIntegration, nrTaps, onStationFilterSize, true);
  initFFT();

  cerr << "generating input signal" << endl;

  generateInputSignal(originalData);

  //  printData(originalData);

  cerr << "simulating station filter" << endl;

  for (unsigned time = 0; time < nrSamplesPerIntegration; time++) {
    performStationFilter(originalData, FIRs, time);
    performStationFFT(transposedBeamFormedData, subbandList, time);
  }

  //  for (unsigned sb = 0; sb < nrSubbands; sb++) {
  for (unsigned time = 0; time < nrSamplesPerIntegration; time++) {
    fcomplex sample = transposedBeamFormedData.samples[1][0][time];
    //      fprintf(stdout, "%20.10lf\n", real(sample));
  }
  //  }

  cerr << "performing inversePPF" << endl;

  inversePPF.performInversePPF(transposedBeamFormedData, invertedFilteredData);

  cerr << "inversePPF done" << endl;

  //  cout << "result:" << endl;

  printData(invertedFilteredData);

  destroyFFT();
  return 0;
}
