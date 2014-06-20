#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>

#include <ACCbin/ConfigurationMgr.h>

char* PTnames[] = { "moduleA.ps", "moduleC.ps",  "moduleE.ps",
					"moduleD.ps", "processG.ps", "processF.ps",
					"applicationQ.ps", "" };

using namespace LOFAR;
using namespace LOFAR::ACC;

int main (int argc, char *argv[]) {
	INIT_LOGGER ("default.log_prop");

	if (argc < 1) {
		cout << endl << "Syntax: " << argv[0] << endl;
		return (-1);
	}

	try {
		cout << "Connecting to the database...";
		ConfigurationMgr		CM("dop50", "overeem", "postgres");

		cout << endl << "Reading checking and storing several PTs" << endl;
		int16	index = 0;
		while (PTnames[index][0]) {
			cout << PTnames[index] << ": Reading, ";
			ParameterTemplate		myPT(PTnames[index]);

			cout << "checking, ";
			string  errorReport;
			if (!myPT.check(errorReport)) {
				cout << endl << "Errorreport: " << errorReport;
			}

			cout << "storing, ";
			CM.addPT(myPT);
			cout << "OK" << endl;

			++index;
		}

		cout << "Construct a TemplateUnion..." << endl;
		TemplateUnion*		myPUptr;
		myPUptr = CM.createTemplateUnion("ApplicationQ", "1.1.0");
		cout << endl << "Resulting TU:" << endl;
		cout << *myPUptr;

//		ParameterTemplate*	myPT = CM.getPT("a", "25.0.2");
//		cout << *myPT;

	}
	catch (LOFAR::Exception&	ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		return (0);
	}

}


