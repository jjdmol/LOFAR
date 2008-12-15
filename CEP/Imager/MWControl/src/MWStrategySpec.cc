//# MWStrategySpec.cc: The specification of a strategy
//#
//# Copyright (C) 2007

#include <MWControl/MWStrategySpec.h>
#include <MWControl/MWSpec.h>
#include <MWControl/MWParameterHandler.h>
#include <Common/StreamUtil.h>
#include <ostream>

namespace std {
  using LOFAR::operator<<;
}
using namespace std;

namespace LOFAR { namespace CEP {

  MWStrategySpec::MWStrategySpec (const std::string& name,
				  const ParameterSet& parset)
    : itsName (name)
  {
    // Get all definitions for this strategy.
    itsParSet = parset.makeSubset(itsName + ".");
    // Get all the steps from the parset definition.
    itsSteps  = MWParameterHandler(parset).getSteps(name);
  }

  WorkDomainSpec MWStrategySpec::getWorkDomainSpec() const
  {
    WorkDomainSpec wds;
    double freqSize = itsParSet.getDouble("WorkDomainSize.Freq");
    double timeSize = itsParSet.getDouble("WorkDomainSize.Time");
    wds.setShape           (DomainShape(freqSize, timeSize));
    wds.setFreqIntegration (itsParSet.getDouble("Integration.Freq", -1));
    wds.setTimeIntegration (itsParSet.getDouble("Integration.Time", -1));
    wds.setInColumn        (itsParSet.getString("InputData", "DATA"));
    wds.setAntennaNames    (itsParSet.getStringVector("Stations"));
    string corrSel = itsParSet.getString("Correlation.Selection", "CROSS");
    wds.setAutoCorr        (corrSel == "AUTO");
    vector<bool> corr(4, false);
    vector<string> corrTypes = itsParSet.getStringVector("Correlation.Type");
    for (unsigned i=0; i<corrTypes.size(); ++i) {
      if (corrTypes[i] == "XX") {
	corr[0] = true;
      } else if (corrTypes[i] == "XY") {
	corr[1] = true;
      } else if (corrTypes[i] == "YX") {
	corr[2] = true;
      } else if (corrTypes[i] == "YY") {
	corr[3] = true;
      }
    }
    wds.setCorr (corr);
    return wds;
  }

  void MWStrategySpec::print (std::ostream& os) const
  {
    os << "Strategy specification: " << itsName << endl;
    itsSteps.print (os, "  ");
  }

  std::ostream& operator<< (std::ostream& os, const MWStrategySpec& spec)
  {
    spec.print (os);
    return os;
  }

}} // end namespaces
