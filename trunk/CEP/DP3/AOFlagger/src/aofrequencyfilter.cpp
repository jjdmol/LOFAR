#include <iostream>

#include <AOFlagger/remote/clusteredobservation.h>
#include <AOFlagger/remote/observationtimerange.h>
#include <AOFlagger/remote/processcommander.h>

#include <AOFlagger/msio/system.h>

#include <AOFlagger/util/lane.h>

#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/strategy/algorithms/convolutions.h>

using namespace std;
using namespace aoRemote;

lane<ObservationTimerange*> *readLane;

// fringe size is given in units of wavelength / fringe. Fringes smaller than that will be filtered.
double filterFringeSize;

double uvDist(double u, double v, double frequencyWidth)
{
	const double
		ud = frequencyWidth * u,
		vd = frequencyWidth * v;
	return sqrt(ud * ud + vd * vd) / UVImager::SpeedOfLight();
}

void workThread()
{
	ObservationTimerange *timerange;
	
	// These are for diagnostic info
	double maxFilterSizeInChannels = 0.0, minFilterSizeInChannels = 1e100;
	
	while(readLane->read(timerange))
	{
		unsigned polarizationCount = timerange->PolarizationCount();
		size_t channelCount = timerange->ChannelCount();
		num_t realBuffer[channelCount];
		num_t imagBuffer[channelCount];
		
		for(size_t t=0;t<timerange->TimestepCount();++t)
		{
			// Calculate the convolution size
			double u = timerange->U(t), v = timerange->V(t);
			double convolutionSize = filterFringeSize * (double) channelCount / uvDist(u, v, timerange->FrequencyWidth());
			if(convolutionSize > maxFilterSizeInChannels) maxFilterSizeInChannels = convolutionSize;
			if(convolutionSize < minFilterSizeInChannels) minFilterSizeInChannels = convolutionSize;
			
			if(convolutionSize > 1.0)
			{
				for(unsigned p=0;p<polarizationCount;++p)
				{
					// Copy data in buffer
					num_t *realPtr = timerange->RealData(t) + p;
					num_t *imagPtr = timerange->ImagData(t) + p;
					
					for(size_t c=0;c<channelCount;++c)
					{
						realBuffer[c] = *realPtr;
						imagBuffer[c] = *imagPtr;
						realPtr += polarizationCount;
						imagPtr += polarizationCount;
					}
					
					// Convolve the data
						Convolutions::OneDimensionalSincConvolution(realBuffer, channelCount, 1.0/convolutionSize);
						Convolutions::OneDimensionalSincConvolution(imagBuffer, channelCount, 1.0/convolutionSize);

					// Copy data back
					realPtr = timerange->RealData(t) + p;
					imagPtr = timerange->ImagData(t) + p;
					for(size_t c=0;c<channelCount;++c)
					{
						*realPtr = realBuffer[c];
						*imagPtr = imagBuffer[c];
						realPtr += polarizationCount;
						imagPtr += polarizationCount;
					}
				}
			}
		}
		delete timerange;
	}
	std::cout << "Worker finished. Filtersize range in channel: " << minFilterSizeInChannels << "-" << maxFilterSizeInChannels << '\n';
}

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		cerr << "Usage: aofrequencyfilter <reffile> <filterfringesize>\n";
	}
	else {
		filterFringeSize = atof(argv[2]);
		ClusteredObservation *obs = ClusteredObservation::Load(argv[1]);
		ProcessCommander commander(*obs);
		commander.PushReadAntennaTablesTask();
		commander.PushReadBandTablesTask();
		commander.Run(false);
		commander.CheckErrors();
		
		ObservationTimerange timerange(*obs);
		const std::vector<BandInfo> &bands = commander.Bands();
		for(size_t i=0; i!=bands.size(); ++i)
			timerange.SetBandInfo(i, bands[i]);
		
		const unsigned processorCount = System::ProcessorCount();
		cout << "CPUs: " << processorCount << '\n';
		unsigned polarizationCount = commander.PolarizationCount();
		cout << "Polarization count: " << polarizationCount << '\n';
		size_t rowCountPerRequest = 128;
		
		timerange.Initialize(polarizationCount, rowCountPerRequest);
		MSRowDataExt *rowBuffer[obs->Size()];
		for(size_t i=0;i<obs->Size();++i)
			rowBuffer[i] = new MSRowDataExt[rowCountPerRequest];
		
		// We ask for "0" rows, which means we will ask for the total number of rows
		commander.PushReadDataRowsTask(timerange, 0, 0, rowBuffer);
		commander.Run(false);
		commander.CheckErrors();
		const size_t totalRows = commander.RowsTotal();
		cout << "Total rows to filter: " << totalRows << '\n';
		
		readLane = new lane<ObservationTimerange*>(processorCount);
		
		// Start worker threads
		boost::thread *threads[processorCount];
		for(size_t i=0; i<processorCount; ++i)
		{
			threads[i] = new boost::thread(&workThread);
		}
		
		
		size_t currentRow = 0;
		while(currentRow < totalRows)
		{
			size_t currentRowCount = rowCountPerRequest;
			if(currentRow + currentRowCount > totalRows)
				currentRowCount = totalRows - currentRow;
			timerange.SetZero();
			commander.PushReadDataRowsTask(timerange, 0, currentRowCount, rowBuffer);
			commander.Run(false);
			commander.CheckErrors();
			
			currentRow += currentRowCount;
			cout << "Read " << currentRow << '/' << totalRows << '\n';
			readLane->write(new ObservationTimerange(timerange));
		}
		readLane->write_end();
		
		for(size_t i=0; i<processorCount; ++i)
		{
			threads[i]->join();
		}
		
		delete readLane;
		
		for(size_t i=0;i<obs->Size();++i)
			delete[] rowBuffer[i];
		delete obs;
	}
}
