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
fftw_plan  plan;
#endif

#include <FIR_InvertedStationPPFWeights.h> // defines invertedStationPPFWeights array

static unsigned nrSubbands = 248;
static unsigned nrChannels = 1; // for the NuMoon pipeline, there are no separate channels.
static unsigned nrTaps = 16;
static unsigned nrSamplesPerIntegration = 768 * 256 / 4; // one quarter of a second
static unsigned onStationFilterSize = 1024;
static double     sampleRate = 195312.5;
static double     centerFrequency	= 384 * sampleRate;
static double     baseFrequency	= centerFrequency - .5 * sampleRate;
static unsigned   testSignalChannel = 5;
static double     signalFrequency	= baseFrequency + testSignalChannel * sampleRate / nrChannels;

static NSTimer firTimer("FIR", true);
static NSTimer fftTimer("FFT", true);
static NSTimer fftInTimer("create FFT input", true);

fcomplex toComplex(double phi)
{
    double s, c;
    sincos(phi, &s, &c);
    fcomplex result = makefcomplex(c, s);
    return result;
}


void generateInputSignal(TransposedBeamFormedData& transposedBeamFormedData) {
	  for (unsigned sb = 0; sb < nrSubbands; sb++) {
		for (unsigned ch = 0; ch < nrChannels; ch++) {
			for (unsigned time = 0; time < nrSamplesPerIntegration; time++) {
				double phi = 2 * M_PI * signalFrequency * time / sampleRate;
				fcomplex sample;
				transposedBeamFormedData.samples[sb][ch][time] = toComplex(phi);
			}
		}
	}
}


int main() {
	TransposedBeamFormedData transposedBeamFormedData(nrSubbands, nrChannels, nrSamplesPerIntegration);
	transposedBeamFormedData.allocate();

	generateInputSignal(transposedBeamFormedData);

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

    InversePPF inversePPF(subbandList, nrSamplesPerIntegration, nrTaps, onStationFilterSize, true);

	cerr << "starting test" << endl;
	inversePPF.filter(transposedBeamFormedData, invertedFilteredData);

	return 0;
}
