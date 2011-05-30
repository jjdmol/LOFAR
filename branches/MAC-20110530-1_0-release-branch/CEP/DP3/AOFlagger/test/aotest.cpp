#include <iostream>

#include <AOFlagger/test/strategy/algorithms/algorithmstestgroup.h>
#include <AOFlagger/test/results/resultstestgroup.h>

int main(int argc, char *argv[])
{
	AlgorithmsTestGroup mainGroup;
	mainGroup.Run();
	
	if(argc > 1 && std::string(argv[1])=="all")
	{
		ResultsTestGroup resultsGroup;
		resultsGroup.Run();
	}
}
