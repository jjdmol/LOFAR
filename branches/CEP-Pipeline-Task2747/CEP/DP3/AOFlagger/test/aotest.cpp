#include <iostream>

#include <AOFlagger/test/strategy/algorithms/algorithmstestgroup.h>
#include <AOFlagger/test/experiments/experimentstestgroup.h>

int main(int argc, char *argv[])
{
  unsigned successes = 0, failures = 0;
	if(argc == 1 || std::string(argv[1])!="only")
	{
		AlgorithmsTestGroup mainGroup;
		mainGroup.Run();
		successes += mainGroup.Successes();
		failures += mainGroup.Failures();
	}
	
	if(argc > 1 && (std::string(argv[1])=="all" || std::string(argv[1])=="only"))
	{
		ExperimentsTestGroup resultsGroup;
		resultsGroup.Run();
		successes += resultsGroup.Successes();
		failures += resultsGroup.Failures();
	}
	
	std::cout << "Succesful tests: " << successes << " / " << (successes + failures) << '\n';
	
	if(failures == 0)
		return 0;
	else
		return 1;
}
