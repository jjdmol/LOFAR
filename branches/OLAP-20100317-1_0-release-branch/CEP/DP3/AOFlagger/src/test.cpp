#include <iostream>
#include <fstream>
#include <sstream>

#include "msio/image2d.h"
#include "msio/measurementset.h"
#include "msio/timefrequencydata.h"
#include "msio/timefrequencyimager.h"

#include "rfi/thresholdconfig.h"
#include "rfi/thresholdtools.h"
#include "rfi/sinusfitter.h"

#include "util/rng.h"

using namespace std;

void AssertEqual(int expected, int actual, const std::string description)
{
	if(expected!=actual)
		cout << "Test failed: " << description << " (expected " << expected << ", was " << actual << ")" << endl;
}

int main(int argc, char *argv[])
{
	// Small script for Parisa
	MeasurementSet set(argv[1]);
	TimeFrequencyImager imager(set);
	size_t antennaCount = set.AntennaCount();
	imager.SetReadData(true);
	imager.SetReadFlags(false);
	imager.SetReadStokesI(true);
	imager.SetReadXX(false);
	imager.SetReadXY(false);
	imager.SetReadYX(false);
	imager.SetReadYY(false);
	size_t height = 0, count = 0;
	for(size_t a1=0;a1<antennaCount;++a1) {
		for(size_t a2=0;a2<antennaCount;++a2) {
			imager.Image(a1, a2, 0, 10, 11);
			TimeFrequencyData data = imager.GetData();
			Image2DCPtr real = data.GetRealPart();
			Image2DCPtr imaginary = data.GetImaginaryPart();
			for(size_t y=0;y<real->Height();++y) {
				std::cout << real->Value(0, y) << " " << imaginary->Value(0, y) << " ";
				++count; }
			height = real->Height();
		}
	}
	std::cout << std::endl;
	//std::cout << "File contains: " << antennaCount << " x " << antennaCount << " x " << height << " = " << count << " values." << std::endl; 
	exit(0);


	// Test the CountMaskLengths operation
	Mask2DPtr mask = Mask2D::CreateSetMaskPtr<false>(1024, 1024);
	int lengths[1024];

	// Test when no samples are flagged
	ThresholdTools::CountMaskLengths(mask, lengths, 1024);
	for(unsigned x=0;x<1024;++x)
		AssertEqual(0, lengths[x], "nothing flagged");

	// Test when all samples are flagged
	for(unsigned y=0;y<mask->Height();++y) {
		for(unsigned x=0;x<mask->Width();++x) {
			mask->SetValue(x, y, true);
		}
	}
	ThresholdTools::CountMaskLengths(mask, lengths, 1024);
	AssertEqual(lengths[1023], 2048, "everything flagged");
	for(unsigned x=0;x<1023;++x)
		AssertEqual(0, lengths[x], "everything flagged");

	// Test single line of size 2
	mask->SetAll<false>();
	mask->SetValue(1,1, true);
	mask->SetValue(1,2, true);
	ThresholdTools::CountMaskLengths(mask, lengths, 1024);
	for(unsigned x=0;x<1024;++x)
		if(x == 1)
			AssertEqual(1, lengths[x], "single line of size 2");
		else
			AssertEqual(0, lengths[x], "single line of size 2");

	// Test single line of size 3
	mask->SetValue(1,0, true);
	ThresholdTools::CountMaskLengths(mask, lengths, 1024);
	for(unsigned x=0;x<1024;++x)
		if(x == 2)
			AssertEqual(1, lengths[x], "single line of size 3");
		else
			AssertEqual(0, lengths[x], "single line of size 3");
	
	// Test combination of horizontal line of size 2 and vertical line of size 3
	mask->SetValue(2,2, true);
	ThresholdTools::CountMaskLengths(mask, lengths, 1024);
	for(unsigned x=0;x<1024;++x)
		if(x == 1)
			AssertEqual(1, lengths[x], "two lines: number with length 2");
		else if(x == 2)
			AssertEqual(1, lengths[x], "two lines: number with length 3");
		else {
			stringstream s;
			s << "two lines: number with length " << (x+1);
			AssertEqual(0, lengths[x], s.str());
		}

	// Test yet another combination
	mask->SetValue(2,3, true);
	ThresholdTools::CountMaskLengths(mask, lengths, 1024);
	for(unsigned x=0;x<1024;++x)
		if(x == 1)
			AssertEqual(2, lengths[x], "combinations: number with length 2");
		else if(x == 2)
			AssertEqual(1, lengths[x], "combinations: number with length 3");
		else {
			stringstream s;
			s << "combinations: number with length " << (x+1);
			AssertEqual(0, lengths[x], s.str());
		}
	
	cout << "All tests were executed." << endl;

	const size_t size = 25000;
	long double dataR[size], dataI[size], datat[size];
	long double frequency = -1.1L*M_PI;
	
	for(long double shift=0.0L;shift<6.4;shift+=0.5) {
		for(unsigned i=0;i<size;++i) {
			datat[i]=(i-size/2)/10.0L;
			dataR[i]=cosl(datat[i] * frequency + shift)*20.0L + 5.0L;
			dataI[i]=-sinl(datat[i] * frequency + shift)*20.0L + 5.0L;
		}
		SinusFitter fitter;
		long double phase=0.0L, amplitude=0.0L;
		fitter.FindPhaseAndAmplitude(phase, amplitude, dataR, datat, size, frequency);
		long double mean = fitter.FindMean(phase, amplitude, dataR, datat, size, frequency);
		cout << "Single amplitude: " << amplitude << ", phase: " << phase << ", mean = " << mean << std::endl;
		fitter.FindPhaseAndAmplitudeComplex(phase, amplitude, dataR, dataI, datat, size, frequency);
		cout << "Complex amplitude: " << amplitude << ", phase: " << phase << ", mean = " << mean << std::endl;
	}
}
