#include <iostream>
#include <string>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSTable.h>

#include <tables/Tables/TableIter.h>

#include <AOFlagger/rfi/thresholdtools.h>
#include <AOFlagger/imaging/uvimager.h>

using namespace std;

double uvDist(double u, double v, double firstFrequency, double lastFrequency)
{
	const double
		lowU = u * firstFrequency,
		lowV = v * firstFrequency,
		highU = u * lastFrequency,
		highV = v * lastFrequency,
		ud = lowU - highU,
		vd = lowV - highV;
	return sqrt(ud * ud + vd * vd) / UVImager::SpeedOfLight();
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		cerr << "Syntax: " << argv[0] << " <fringe size> <MS>\n";
		cerr << " fringe size is a double and should be given in units of wavelength / fringe.\n";
	} else {
		const double fringeSize = atof(argv[1]);
		const string msFilename = argv[2];
		cout << "Fringe size: " << fringeSize << '\n';

		casa::MeasurementSet ms(msFilename);
		casa::Table table(msFilename, casa::Table::Update);

		//count number of polarizations
		casa::Table polTable = ms.polarization();
		casa::ROArrayColumn<int> corTypeColumn(polTable, "CORR_TYPE"); 
		const unsigned polarizationCount = corTypeColumn(0).shape()[0];
		cout << "Number of polarizations: " << polarizationCount << '\n';

		// Find lowest and highest frequency and check order
		double lowestFrequency = 0.0, highestFrequency = 0.0;
		casa::Table spectralWindowTable = ms.spectralWindow();
		casa::ROArrayColumn<double> frequencyCol(spectralWindowTable, "CHAN_FREQ");
		for(unsigned b=0;b<spectralWindowTable.nrow();++b)
		{
			casa::Array<double> frequencyArray = frequencyCol(b);
			casa::Array<double>::const_iterator frequencyIterator = frequencyArray.begin();
			while(frequencyIterator != frequencyArray.end())
			{
				double frequency = *frequencyIterator;
				if(lowestFrequency == 0.0) lowestFrequency = frequency;
				if(frequency < lowestFrequency || frequency <= highestFrequency)
				{
					cerr << "ERROR: Channels are not ordered in increasing frequency!\n";
					abort();
				}
				highestFrequency = frequency;
				++frequencyIterator;
			}
		}
		cout
			<< "Number of bands: " << spectralWindowTable.nrow()
			<< " (" << round(lowestFrequency/1e6) << " MHz - "
			<< round(highestFrequency/1e6) << " MHz)\n";

		const unsigned frequencyCount =
			casa::ROArrayColumn<casa::Complex>(table, "DATA")(0).shape()[1];
		cout << "Channels per band: " << frequencyCount << '\n';

		const unsigned long totalIterations =
			table.nrow() / spectralWindowTable.nrow();
		cout << "Total iterations: " << totalIterations << '\n';

		// Create the sorted table and iterate over it
		casa::Block<casa::String> names(4);
		names[0] = "TIME";
		names[1] = "ANTENNA1";
		names[2] = "ANTENNA2";
		names[3] = "DATA_DESC_ID";
		cout << "Sorting...\n";
		casa::Table sortab = table.sort(names);
		cout << "Iterating...\n";
		unsigned long iterSteps = 0;
		names.resize(3, true, true);
		casa::TableIterator iter (sortab, names, casa::TableIterator::Ascending, casa::TableIterator::NoSort);
		double maxFringeChannels = 0.0, minFringeChannels = 1e100;
		while (! iter.pastEnd()) {
			casa::Table table = iter.table();
			casa::ROScalarColumn<int> antenna1Column =
					casa::ROScalarColumn<int>(table, "ANTENNA1");
			casa::ROScalarColumn<int> antenna2Column =
					casa::ROScalarColumn<int>(table, "ANTENNA2");

			// Skip autocorrelations
			const int antenna1 = antenna1Column(0), antenna2 = antenna2Column(0);
			if(antenna1 != antenna2)
			{
				casa::ArrayColumn<casa::Complex> dataColumn =
					casa::ArrayColumn<casa::Complex>(table, "DATA");
				casa::ROArrayColumn<double> uvwColumn =
						casa::ROArrayColumn<double>(table, "UVW");
	
				const casa::IPosition &dataShape = dataColumn.shape(0);
				if(dataShape[1] != frequencyCount) {
					std::cerr << "ERROR: bands do not have equal number of channels!\n";
					abort();
				}
				const unsigned bandCount = table.nrow();
	
				const unsigned length = bandCount * frequencyCount;
	
				// Allocate memory for putting the data of all channels in an array, for each polarization
				float
					*realData[polarizationCount],
					*imagData[polarizationCount];
				for(unsigned p=0;p<polarizationCount;++p)
				{
					realData[p] = new float[length];
					imagData[p] = new float[length];
				}
	
				// Copy data from tables in arrays
				for(unsigned i=0;i<bandCount;++i)
				{
					casa::Array<casa::Complex> dataArray = dataColumn(i);
					casa::Array<casa::Complex>::const_iterator dataIterator = dataArray.begin();
					unsigned index = i * frequencyCount;
					for(unsigned f=0;f<frequencyCount;++f)
					{
						for(unsigned p=0;p<polarizationCount;++p)
						{
							realData[p][index] = (*dataIterator).real();
							imagData[p][index] = (*dataIterator).imag();
						}
						++index;
					}
				}
	
				// Convolve the data
				casa::Array<double>::const_iterator uvwIterator = uvwColumn(0).begin();
				const double u = *uvwIterator;
				++uvwIterator;
				const double v = *uvwIterator;
				const double convSize = fringeSize * (double) length / uvDist(u, v, lowestFrequency, highestFrequency);
				if(convSize > maxFringeChannels) maxFringeChannels = convSize;
				if(convSize < minFringeChannels) minFringeChannels = convSize;
				for(unsigned p=0;p<polarizationCount;++p)
				{
					ThresholdTools::OneDimensionalSincConvolution(realData[p], length, convSize);
					ThresholdTools::OneDimensionalSincConvolution(imagData[p], length, convSize);
				}
	
				// Copy data back to tables
				for(unsigned i=0;i<bandCount;++i)
				{
					casa::Array<casa::Complex> dataArray = dataColumn(i);
					casa::Array<casa::Complex>::iterator dataIterator = dataArray.begin();
					unsigned index = i * frequencyCount;
					for(unsigned f=0;f<frequencyCount;++f)
					{
						for(unsigned p=0;p<polarizationCount;++p)
						{
							(*dataIterator).real() = realData[p][index];
							(*dataIterator).imag() = imagData[p][index];
						}
						++index;
					}
					dataColumn.basePut(i, dataArray);
				}
	
				// Free memory
				for(unsigned p=0;p<polarizationCount;++p)
				{
					delete[] realData[p];
					delete[] imagData[p];
				}
			}

			iter.next();
			++iterSteps;
			if((iterSteps * 100UL) % totalIterations < 100UL)
				cout << '.' << flush;
			if((iterSteps * 10UL) % totalIterations < 10UL)
				cout << (iterSteps*100/totalIterations) << '%' << flush;
		}
		cout
			<< "Done. " << iterSteps << " steps taken.\n"
			<< "Maximum filtering fringe size = " << maxFringeChannels << " channels, "
			   "minimum = " << minFringeChannels << " channels. \n";
	}
}
