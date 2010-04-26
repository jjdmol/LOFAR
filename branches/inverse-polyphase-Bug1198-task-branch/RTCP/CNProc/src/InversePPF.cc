/*
  For comments on how this class works, see InversePPF.h.
*/

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <InversePPF.h>

using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace LOFAR::TYPES;

#include <FIR_InvertedStationPPFWeights.h> // defines invertedStationPPFWeights array

static NSTimer firTimer("FIR", true);
static NSTimer fftTimer("FFT", true);
static NSTimer fftInTimer("create FFT input", true);


InversePPF::InversePPF(vector<unsigned>& subbandList, unsigned nrSamplesPerIntegration, unsigned nrTaps, unsigned onStationFilterSize, bool verbose)
:
	itsFilterBank(false, nrTaps, onStationFilterSize, (float*) invertedStationPPFWeights),
	itsSubbandList(subbandList),
	itsNrSubbands(itsSubbandList.size()),
	itsNrTaps(nrTaps),
	itsNrSamplesPerIntegration(nrSamplesPerIntegration),
	itsOnStationFilterSize(onStationFilterSize),
	itsVerbose(verbose)
{
	double origInputSize = (itsNrSubbands * itsNrSamplesPerIntegration * sizeof(fcomplex)) / (1024.0*1024.0);
	double fftBufSize =    (itsOnStationFilterSize * sizeof(float)) / (1024.0);
	double outputSize = (itsOnStationFilterSize * itsNrSamplesPerIntegration * sizeof(float)) / (1024.0*1024.0);

	if(itsVerbose) {
		cerr << "size of original input data: " << origInputSize << " MB" << endl;
		cerr << "size of FFT buffers: " << fftBufSize << " KB" << endl;
		cerr << "size of output: " << outputSize << " MB" << endl;
		cerr << "total memory usage: " << (origInputSize + outputSize) << " MB" << endl;
	}

	// Init the FIR filters themselves with the weights of the filterbank.
	itsFIRs.resize(onStationFilterSize);
	for(unsigned chan=0; chan<itsOnStationFilterSize; chan++) {
		itsFIRs[chan].initFilter(&itsFilterBank, chan);
	}

	initFFT();
}


InversePPF::~InversePPF()
{
	destroyFFT();
}


void InversePPF::initFFT()
{
#if defined HAVE_FFTW3
	itsFftInData  = (float*) fftwf_malloc(itsOnStationFilterSize * sizeof(float));
	itsFftOutData = (float*) fftwf_malloc(itsOnStationFilterSize * sizeof(float));
#elif defined HAVE_FFTW2
	itsFftInData  = (float*) malloc(itsOnStationFilterSize * sizeof(float));
	itsFftOutData = (float*) malloc(itsOnStationFilterSize * sizeof(float));

	itsPlan = rfftw_create_plan(onStationFilterSize, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);
#endif

	if (itsFftInData == NULL || itsFftOutData == NULL) {
		cerr << "Out of memory" << endl;
		exit(1);
	}

}


void InversePPF::destroyFFT()
{
#if defined HAVE_FFTW3
	fftwf_free(itsFftInData);
	fftwf_free(itsFftOutData);
#elif defined HAVE_FFTW2
	rfftw_destroy_plan(itsPlan);
#endif
}


void InversePPF::performFilter(InverseFilteredData& invertedFilteredData, unsigned time, unsigned minorTime)
{
	unsigned filterIndex = minorTime % itsOnStationFilterSize;
	float sample = itsFftOutData[minorTime];
	float result = itsFIRs[filterIndex].processNextSample(sample);
	invertedFilteredData.samples[time * itsOnStationFilterSize + minorTime] = result;
}


void InversePPF::createFFTInput(const TransposedBeamFormedData& transposedBeamFormedData, unsigned time)
{
	fftInTimer.start();

	// First set the unselected subbands to zero.
	memset(itsFftInData, 0, itsOnStationFilterSize * sizeof(float));

	// Fill input buffer, using "half complex" format.
        // There can be gaps in the subband list.
	// Copy the samples from the different subbands to their correct places.
	for(unsigned i=0; i < itsNrSubbands; i++) {
		unsigned sb = itsSubbandList[i];
		fcomplex sample = transposedBeamFormedData.samples[sb][0 /* channel */][time];

		itsFftInData[sb]                          = real(sample);
		itsFftInData[itsOnStationFilterSize - sb] = imag(sample);
	}

	fftInTimer.stop();
}


void InversePPF::performInverseFFT()
{
	fftTimer.start();

#if defined HAVE_FFTW3
	// in and out are not the same buffer, and the input is destroyed by the fftw call.
	// We have to recreate the plan each time, to write directly to the right output location.
	// Fftw caches the plan info, so this should not be a problem. Alternatively, we could use the guru interface,
	// as is already done in the PPF at CEP.

	fftwf_plan plan = fftwf_plan_r2r_1d(itsOnStationFilterSize, itsFftInData, itsFftOutData, FFTW_HC2R, FFTW_ESTIMATE);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);

#elif defined HAVE_FFTW2
	// Do the inverse FFT. NB: this call destoys the input data.
	rfftw_one(itsPlan, (fftw_real*) itsFftInData, (fftw_real*) itsFftOutData);
#endif

	fftTimer.stop();
}


void InversePPF::performFiltering(InverseFilteredData& invertedFilteredData, unsigned time)
{
	firTimer.start();

	for(unsigned minorTime=0; minorTime < itsOnStationFilterSize; minorTime++) {
		performFilter(invertedFilteredData, time, minorTime);
	}

	firTimer.stop();
}


void InversePPF::performInversePolyPhase(const TransposedBeamFormedData& transposedBeamFormedData, 
					 InverseFilteredData& invertedFilteredData, unsigned time)
{
	createFFTInput(transposedBeamFormedData, time);
	performInverseFFT();
	performFiltering(invertedFilteredData, time);
}


void InversePPF::filter(const TransposedBeamFormedData& transposedBeamFormedData, InverseFilteredData& invertedFilteredData)
{
	for(unsigned time=0; time < itsNrSamplesPerIntegration; time++) {
		performInversePolyPhase(transposedBeamFormedData, invertedFilteredData, time);
	}

}
