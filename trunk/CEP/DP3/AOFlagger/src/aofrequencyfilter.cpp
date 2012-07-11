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
		delete obs;
	}
}
