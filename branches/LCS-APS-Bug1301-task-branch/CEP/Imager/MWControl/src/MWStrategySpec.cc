//# MWStrategySpec.cc: Specification of a BBS strateg
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
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
