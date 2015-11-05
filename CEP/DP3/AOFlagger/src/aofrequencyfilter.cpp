#include <iostream>

#include <fftw3.h>

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
lane<ObservationTimerange*> *writeLane;

fftw_plan fftPlanForward, fftPlanBackward;

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
	
	if(readLane->read(timerange))
	{
		const size_t channelCount = timerange->ChannelCount();
		const unsigned polarizationCount = timerange->PolarizationCount();
		fftw_complex
			*fftIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * channelCount),
			*fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * channelCount);
		do
		{
			for(size_t t=0;t<timerange->TimestepCount();++t)
			{
				if(timerange->Antenna1(t) != timerange->Antenna2(t))
				{
					// Calculate the frequencies to filter
					double u = timerange->U(t), v = timerange->V(t);
					double limitFrequency = uvDist(u, v, timerange->FrequencyWidth()) / filterFringeSize;
					
					if(limitFrequency < channelCount) // otherwise no frequencies had to be removed
					{
						if(limitFrequency > maxFilterSizeInChannels) maxFilterSizeInChannels = limitFrequency;
						if(limitFrequency < minFilterSizeInChannels) minFilterSizeInChannels = limitFrequency;
						for(unsigned p=0;p<polarizationCount;++p)
						{
							// Copy data in buffer
							num_t *realPtr = timerange->RealData(t) + p;
							num_t *imagPtr = timerange->ImagData(t) + p;
							
							for(size_t c=0;c<channelCount;++c)
							{
								fftIn[c][0] = *realPtr;
								fftIn[c][1] = *imagPtr;
								realPtr += polarizationCount;
								imagPtr += polarizationCount;
							}
							
							fftw_execute_dft(fftPlanForward, fftIn, fftOut);
							// Remove the high frequencies [n/2-filterIndexSize : n/2+filterIndexSize]
							size_t filterIndexSize = (limitFrequency > 1.0) ? (size_t) ceil(limitFrequency) : 1;
							for(size_t f=filterIndexSize;f<channelCount - filterIndexSize;++f)
							{
								fftOut[f][0] = 0.0;
								fftOut[f][1] = 0.0;
							}
							fftw_execute_dft(fftPlanBackward, fftOut, fftIn);

							// Copy data back; fftw multiplies data with n, so divide by n.
							double factor = 1.0 / (double) channelCount;
							realPtr = timerange->RealData(t) + p;
							imagPtr = timerange->ImagData(t) + p;
							for(size_t c=0;c<channelCount;++c)
							{
								*realPtr = fftIn[c][0] * factor;
								*imagPtr = fftIn[c][1] * factor;
								realPtr += polarizationCount;
								imagPtr += polarizationCount;
							}
						}
					}
				}
			}
			delete timerange;
		} while(readLane->read(timerange));
		fftw_free(fftIn);
		fftw_free(fftOut);
	}
	std::cout << "Worker finished. Filtersize range in channel: " << minFilterSizeInChannels << "-" << maxFilterSizeInChannels << '\n';
}

void writeThread()
{
}

void initializeFFTW(size_t channelCount)
{
	fftw_complex *fftIn, *fftOut;
	
	fftIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * channelCount);
	fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * channelCount);
	fftPlanForward = fftw_plan_dft_1d(channelCount, fftIn, fftOut, FFTW_FORWARD, FFTW_MEASURE);
	fftPlanBackward = fftw_plan_dft_1d(channelCount, fftIn, fftOut, FFTW_BACKWARD, FFTW_MEASURE);

	fftw_free(fftIn);
	fftw_free(fftOut);
}

void deinitializeFFTW()
{
	fftw_destroy_plan(fftPlanForward);
	fftw_destroy_plan(fftPlanBackward);
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
		const size_t rowCountPerRequest = 128;
		
		timerange.Initialize(polarizationCount, rowCountPerRequest);
		
		cout << "Initializing FFTW..." << std::flush;
		initializeFFTW(timerange.ChannelCount());
		cout << " Done.\n";
		
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
		writeLane = new lane<ObservationTimerange*>(processorCount);
		
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
		
		writeLane->write_end();
		
		delete readLane;
		delete writeLane;
		
		for(size_t i=0;i<obs->Size();++i)
			delete[] rowBuffer[i];
		delete obs;
	}
}
