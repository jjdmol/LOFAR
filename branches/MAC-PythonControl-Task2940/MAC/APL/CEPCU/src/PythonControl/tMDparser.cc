#include <stdio.h>
#include <Common/ParameterSet.h>
#include <Common/ParameterRecord.h>

using namespace std;
using namespace LOFAR;

int main(int argc, char* argv[])
{
	if (argc != 2) {
		cout << "Syntax: " << argv[0] << " metadatafile" << endl;
		return (0);
	}

	// read parameterset
	ParameterSet	metadata;
	metadata.adoptFile(argv[1]);

	// Loop over the parameterset and send the information to the KVTlogger.
	// During the transition phase from parameter-based to record-based storage in OTDB the
	// nodenames ending in '_' are implemented both as parameter and as record.
	ParameterSet::iterator		iter = metadata.begin();
	ParameterSet::iterator		end  = metadata.end();
	while (iter != end) {
		string	key(iter->first);	// make destoyable copy
		rtrim(key, "[]0123456789");
		bool	doubleStorage(key[key.size()-1] == '_');
		bool	isRecord(iter->second.isRecord());
		//   isRecord  doubleStorage
		// --------------------------------------------------------------
		//      Y          Y           store as record and as parameters
		//      Y          N           store as parameters
		//      N          *           store parameter
		if (!isRecord) {
			cout << "BASIC: " << iter->first << " = " << iter->second << endl;
		}
		else {
			if (doubleStorage) {
				cout << "RECORD: " << iter->first << " = " << iter->second << endl;
			}
			ParameterRecord	pr(iter->second.getRecord());
			ParameterRecord::const_iterator	prIter = pr.begin();
			ParameterRecord::const_iterator	prEnd  = pr.end();
			while (prIter != prEnd) {
				cout << "ELEMENT: " << iter->first+"."+prIter->first << " = " << prIter->second << endl;
				prIter++;
			}
		}
		iter++;
	}
    cout << "Done" << endl;
	return (1);
}

