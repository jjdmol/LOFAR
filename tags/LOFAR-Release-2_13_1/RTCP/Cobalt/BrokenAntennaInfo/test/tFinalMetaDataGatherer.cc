#include <lofar_config.h>
#include <iostream>

#include <BrokenAntennaInfo/FinalMetaDataGatherer.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;

int main() {
  Parset ps;

  ps.add("Cobalt.FinalMetaDataGatherer.database.host", "localhost");

  ps.add("Observation.startTime", "2014-01-01 00:00:00");
  ps.add("Observation.stopTime",  "2014-01-01 00:01:00");

  try {
    FinalMetaData fmd = getFinalMetaData(ps);

    cout << fmd << endl;
  } catch(Exception &ex) {
    cerr << ex << endl;
  }
}
