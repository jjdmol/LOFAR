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
		commander.PushReadAntennaTablesTask();
		commander.PushReadBandTablesTask();
		commander.Run(false);
		commander.CheckErrors();
		
		ObservationTimerange timerange(*obs);
		const std::vector<BandInfo> &bands = commander.Bands();
		for(size_t i=0; i!=bands.size(); ++i)
			timerange.SetBandInfo(i, bands[i]);
		
		unsigned polarizationCount = 4;
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
		
		size_t currentRow = 0;
		while(currentRow < totalRows)
		{
			size_t currentRowCount = rowCountPerRequest;
			if(currentRow + currentRowCount > totalRows)
				currentRowCount = totalRows - currentRow;
			commander.PushReadDataRowsTask(timerange, 0, currentRowCount, rowBuffer);
			commander.Run(false);
			commander.CheckErrors();
			
			currentRow += currentRowCount;
			cout << "Read " << currentRow << '/' << totalRows << '\n';
		}
		
		for(size_t i=0;i<obs->Size();++i)
			delete[] rowBuffer[i];
		delete obs;
	}
}
