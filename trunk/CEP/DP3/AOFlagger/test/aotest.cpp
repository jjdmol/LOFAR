#include <iostream>

#include <AOFlagger/test/strategy/algorithms/algorithmstestgroup.h>
#include <AOFlagger/test/experiments/experimentstestgroup.h>

int main(int argc, char *argv[])
{
	if(argc == 1 || std::string(argv[1])!="only")
	{
		AlgorithmsTestGroup mainGroup;
		mainGroup.Run();
	}
	
	if(argc > 1 && (std::string(argv[1])=="all" || std::string(argv[1])=="only"))
	{
		ExperimentsTestGroup resultsGroup;
		resultsGroup.Run();
	}
}
