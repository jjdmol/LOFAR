#include <iostream>

#include <AOFlagger/remote/clusteredobservation.h>
#include <AOFlagger/remote/observationtimerange.h>
#include <AOFlagger/remote/processcommander.h>

using namespace std;
using namespace aoRemote;

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		cerr << "Usage: aofrequencyfilter <reffile>\n";
	}
	else {
		ClusteredObservation *obs = ClusteredObservation::Load(argv[1]);
		ProcessCommander commander(*obs);
		commander.PushReadBandTablesTask();
		commander.Run(false);
		commander.CheckErrors();
		
		ObservationTimerange timerange(*obs);
		const std::vector<BandInfo> &bands = commander.Bands();
		for(size_t i=0; i!=bands.size(); ++i)
			timerange.SetBandInfo(i, bands[i]);
		
		unsigned polarizationCount = 4;
		timerange.InitializeChannels(polarizationCount);
		
		size_t timestepCount = 128;
		MSRowDataExt *rowBuffer[obs->Size()];
		for(size_t i=0;i<obs->Size();++i)
			rowBuffer[i] = new MSRowDataExt[timestepCount];
		commander.PushReadDataRowsTask(timerange, 0, timestepCount, rowBuffer);
		commander.Run(false);
		commander.CheckErrors();
		
		delete obs;
	}
}
