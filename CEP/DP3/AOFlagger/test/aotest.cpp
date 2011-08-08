#include <iostream>

#include <AOFlagger/test/strategy/algorithms/algorithmstestgroup.h>
#include <AOFlagger/test/experiments/experimentstestgroup.h>

int main(int argc, char *argv[])
{
	AlgorithmsTestGroup mainGroup;
	mainGroup.Run();
	
	if(argc > 1 && std::string(argv[1])=="all")
	{
		ExperimentsTestGroup resultsGroup;
		resultsGroup.Run();
	}
}
