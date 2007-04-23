#ifndef MSReaderH
#define MSReaderH

#include "MSInfo.h"

#include <ms/MeasurementSets.h>

#include <tables/Tables.h>

#include <casa/Arrays.h>
#include <casa/aipstype.h>
#include <casa/complex.h>

#include <string>

using namespace std;
using namespace casa;

class MSReader
{
	public:
		MSReader(const string& msName);
		~MSReader();
		int getNumberAntennae();
		Cube<complex<float> > getTimeCube			(int timeSlot , int selectedBand, int polarizationID, int startFreq		 , int stopFreq    );
		Cube<complex<float> > getFrequencyCube(int frequency, int selectedBand, int polarizationID, int startTimeSlot, int stopTimeSlot);
		MSInfo getMSInfo();
	private:
		string msName;
		MeasurementSet* ms;
		//int nAntennae;
		int bandsPerTimeSlot;
		MSInfo msInfo;
};


#endif

