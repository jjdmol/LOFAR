/*
subbandList staat in de Parset file (en class), precies in het formaat dat we nodig hebben.
#subbands x the index van de FFT bin. Dus 248 x [0..511]

The station PPF first does the FIR filtering, next an FFT.
The station FFT goes from real to complex: 1024 reals to 1024 complex.
Of those 1024 results, the lower half is discarded, since they are the complex conjungates of the upper half. 
From the FFTW manual: In many practical applications, the input data in[i] are purely real numbers, 
in which case the DFT output satisfies the Hermitian redundancy: out[i] is the conjugate of out[n-i].
Next, from the 512 upper values, 248 subbands are selected. I.e. more than half of the frequencies are thrown away.


For the inverse PPF, we first do an inverse FFT, and next the FIR filter with the inverted constants.
In memory, we have to keep 1 beam, 248 subbands. The CEP PPF was bypassed, so there are no channels.
Also, we can assume that each core processes only 1 polarization.
In total, there can be as many as 50 beams and 2 polarizations, so we need 100 cores for the processing.


  Er zijn 3 opties:

  - complex to complex FFT

  - complex to real FFT *** deze gebruik ik nu ***
  * sloopt input data, dus die moet je elke keer opnieuw maken
  * input moet in "half complex" formaat zijn

  This consists of the non-redundant half of the complex output for a 1d real-input DFT of size n, 
  stored as a sequence of n  real numbers (double) in the format:

  r0, r1, r2, ..., rn/2, i(n+1)/2-1, ..., i2, i1

  Here, rk is the real part of the kth output, and ik is the imaginary
  part. (Division by 2 is rounded down.) For a halfcomplex array hc[n],
  the kth component thus has its real part in hc[k] and its imaginary
  part in hc[n-k], with the exception of k == 0 or n/2 (the latter only
  if n is even)—in these two cases, the imaginary part is zero due to
  symmetries of the real-input DFT, and is not stored. Thus, the r2hc
  transform of n real values is a halfcomplex array of length n, and
  vice versa for hc2r. Aside from the differing format, the output of
  FFTW_R2HC/FFTW_HC2R is otherwise exactly the same as for the
  corresponding 1d r2c/c2r transform (i.e. FFTW_FORWARD/FFTW_BACKWARD
  transforms, respectively). Recall that these transforms are
  unnormalized, so r2hc followed by hc2r will result in the original
  data multiplied by n. Furthermore, like the c2r transform, an
  out-of-place hc2r transform will destroy its input array.
  
  - complex to real FFT, multidimensionale versie met N=1
  * sloopt input data ook
  * normaal input formaat

  TODO: welke heeft de beste performance?

  A BG/P compute node has 2 GB of memory, which is shared between 4 cores. 
  So, we have only 512 MB per core.
*/

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
fftw_plan  plan;
#endif

#include <FIR_InvertedStationPPFWeights.h> // defines invertedStationPPFWeights array

float* fftInData;
float* fftOutData;

unsigned nrSubbands = 248;
unsigned nrChannels = 1; // for the NuMoon pipeline, there are no separate channels.
unsigned nrTaps = 16;
unsigned nrSamplesPerIntegration = 768 * 256 / 4; // one quarter of a second
unsigned onStationFilterSize = 1024;

NSTimer firTimer("FIR", true);
NSTimer fftTimer("FFT", true);
NSTimer fftInTimer("create FFT input", true);


void initFFT()
{
#if defined HAVE_FFTW3
	fftInData  = (float*) fftwf_malloc(onStationFilterSize * sizeof(float));
	fftOutData = (float*) fftwf_malloc(onStationFilterSize * sizeof(float));

	if (fftInData == NULL || fftOutData == NULL) {
		cerr << "Out of memory" << endl;
		exit(1);
	}
#elif defined HAVE_FFTW2
	fftInData  = (float*) malloc(onStationFilterSize * sizeof(float));
	fftOutData = (float*) malloc(onStationFilterSize * sizeof(float));

	if (fftInData == NULL || fftOutData == NULL) {
		cerr << "Out of memory" << endl;
		exit(1);
	}	

	plan = rfftw_create_plan(onStationFilterSize, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);
#endif
}

void destroyFFT()
{
#if defined HAVE_FFTW3
	fftwf_free(fftInData);
	fftwf_free(fftOutData);
#elif defined HAVE_FFTW2
	rfftw_destroy_plan(plan);
#endif
}


static void performFilter(vector<FIR<float> >& FIRs, InverseFilteredData& invertedFilteredData, unsigned time, unsigned minorTime)
{
	unsigned filterIndex = minorTime % onStationFilterSize;
	float sample = fftOutData[minorTime];
	float result = FIRs[filterIndex].processNextSample(sample);
	invertedFilteredData.samples[time * onStationFilterSize + minorTime] = result;
}


static void createFFTInput(TransposedBeamFormedData& transposedBeamFormedData, vector<unsigned>& subbandList, unsigned time)
{
	fftInTimer.start();

	// First set the unselected subbands to zero.
	memset(fftInData, 0, onStationFilterSize * sizeof(float));

	// Fill input buffer, using "half complex" format.
        // There can be gaps in the subband list.
	// Copy the samples from the different subbands to their correct places.
	for(unsigned i=0; i < nrSubbands; i++) {
		unsigned sb = subbandList[i];
		fcomplex sample = transposedBeamFormedData.samples[sb][0 /* channel */][time];

		fftInData[sb]                       = real(sample);
		fftInData[onStationFilterSize - sb] = imag(sample);
	}

	fftInTimer.stop();
}


static void performInverseFFT()
{
	fftTimer.start();

#if defined HAVE_FFTW3
	// in and out are not the same buffer, and the input is destroyed by the fftw call.
	// We have to recreate the plan each time, to write directly to the right output location.
	// Fftw caches the plan info, so this should not be a problem. Alternatively, we could use the guru interface,
	// as is already done in the PPF at CEP.

	fftwf_plan plan = fftwf_plan_r2r_1d(onStationFilterSize, fftInData, fftOutData, FFTW_HC2R, FFTW_ESTIMATE);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);

#elif defined HAVE_FFTW2
	// Do the inverse FFT. NB: this call destoys the input data.
	rfftw_one(plan, (fftw_real*) fftInData, (fftw_real*) fftOutData);
#endif

	fftTimer.stop();
}


static void performFiltering(vector<FIR<float> >& FIRs, InverseFilteredData& invertedFilteredData, unsigned time)
{
	firTimer.start();

	for(unsigned minorTime=0; minorTime < onStationFilterSize; minorTime++) {
		performFilter(FIRs, invertedFilteredData, time, minorTime);
	}

	firTimer.stop();
}


static void performInversePolyPhase(vector<FIR<float> >& FIRs, TransposedBeamFormedData& transposedBeamFormedData, 
			     InverseFilteredData& invertedFilteredData, vector<unsigned>& subbandList, unsigned time)
{
	if(nrChannels != 1) {
		cerr << "The number of channels must be 1 for the polyphase inversion." << endl;
		exit(1);
	}

	if(nrSubbands != subbandList.size()) {
		cerr << "The number of subbands does not match the size of the subband list" << endl;
		exit(1);
	}
	
	createFFTInput(transposedBeamFormedData, subbandList, time);
	performInverseFFT();
	performFiltering(FIRs, invertedFilteredData, time);
}


int main() {
	// [nrSubbands][nrChannels][nrSamplesPerIntegration | 2]
	TransposedBeamFormedData transposedBeamFormedData(nrSubbands, nrChannels, nrSamplesPerIntegration);
	transposedBeamFormedData.allocate();

	double origInputSize = (nrSubbands * nrSamplesPerIntegration * sizeof(fcomplex)) / (1024.0*1024.0);
	double fftBufSize =    (onStationFilterSize * sizeof(float)) / (1024.0);
	double outputSize = (onStationFilterSize * nrSamplesPerIntegration * sizeof(float)) / (1024.0*1024.0);

	cerr << "size of original input data: " << origInputSize << " MB" << endl;
	cerr << "size of FFT buffers: " << fftBufSize << " KB" << endl;
	cerr << "size of output: " << outputSize << " MB" << endl;
	cerr << "total memory usage: " << (origInputSize + outputSize) << " MB" << endl;

	InverseFilteredData invertedFilteredData(nrSamplesPerIntegration, onStationFilterSize);
	invertedFilteredData.allocate();

	vector<unsigned> subbandList;
	subbandList.resize(nrSubbands);

	// for now, we just select the first 248 subbands.
	for(unsigned sb=0; sb<nrSubbands; sb++) {
		subbandList[sb] = sb;
	}  

	FilterBank filterBank(false, nrTaps, onStationFilterSize, (float*) invertedStationPPFWeights);
	vector<FIR<float> > FIRs;
	FIRs.resize(onStationFilterSize);

	// Init the FIR filters themselves with the weights of the filterbank.
	for(unsigned chan=0; chan<onStationFilterSize; chan++) {
		FIRs[chan].initFilter(&filterBank, chan);
	}

	initFFT();

	cerr << "starting test" << endl;

	for(unsigned time=0; time < nrSamplesPerIntegration; time++) {
		performInversePolyPhase(FIRs, transposedBeamFormedData, invertedFilteredData, subbandList, time);
	}

	destroyFFT();

	return 0;
}
