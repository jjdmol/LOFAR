#include <iostream>

#include <AOFlagger/test/strategy/algorithms/algorithmstestgroup.h>
#include <AOFlagger/test/results/resultstestgroup.h>

int main(int, char *[])
{
	AlgorithmsTestGroup mainGroup;
	mainGroup.Run();
	
	ResultsTestGroup resultsGroup;
	resultsGroup.Run();
}
