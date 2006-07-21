//#  BBSStrategy.cc: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStep.h>
#include <BBSControl/Exceptions.h>
#include <Common/LofarLogger.h>
#include <BBSControl/StreamFormatting.h>

namespace LOFAR
{
  using ACC::APS::ParameterSet;

  namespace BBS
  {

    //##--------   P u b l i c   m e t h o d s   --------##//

    BBSStrategy::BBSStrategy(const ParameterSet& aParSet)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Get the name of the Measurement Set.
      itsDataSet = aParSet.getString("DataSet");

      // Retrieve the blackboard related key/value pairs.
      itsBBDB.host = aParSet.getString("BBDB.Host");
      itsBBDB.port = aParSet.getUint16("BBDB.Port");
      itsBBDB.dbName = aParSet.getString("BBDB.DBName");
      itsBBDB.username = aParSet.getString("BBDB.UserName");
      itsBBDB.password = aParSet.getString("BBDB.PassWord");

      // Retrieve the parameter database related key/value pairs.
      itsParmDB.instrument = aParSet.getString("ParmDB.Instrument");
      itsParmDB.localSky = aParSet.getString("ParmDB.LocalSky");

      // Create a subset of \a aParSet, containing only the relevant keys for
      // a Strategy.
      ParameterSet ps(aParSet.makeSubset("Strategy."));

      // This strategy consists of the following steps.
      vector<string> steps(ps.getStringVector("Steps"));

      // Create a new step for each name in \a steps.
      for (uint i = 0; i < steps.size(); ++i) {
	itsSteps.push_back(BBSStep::create(steps[i], aParSet, 0));
      }

      // ID's of the stations to be used by this strategy.
      itsStations = ps.getUint32Vector("Stations");

      // Get the name of the MS input data column
      itsInputData = ps.getString("InputData");

      // Get the work domain size for this strategy
      itsDomainSize.bandWidth = ps.getDouble("WorkDomainSize.Freq");
      itsDomainSize.timeInterval = ps.getDouble("WorkDomainSize.Time");

      // Get the correlation product selection (ALL, AUTO, or CROSS)
      string sel = ps.getString("Correlation.Selection");
      if      (sel == "ALL")   itsCorrelation.selection = Correlation::ALL;
      else if (sel == "AUTO")  itsCorrelation.selection = Correlation::AUTO;
      else if (sel == "CROSS") itsCorrelation.selection = Correlation::CROSS;
      else THROW(BBSControlException, 
		 "Invalid correlation selection " << sel);
      itsCorrelation.type = ps.getStringVector("Correlation.Type");

      // Get the integration intervals in frequency (Hz) and time (s).
      itsIntegration.deltaFreq = ps.getDouble("Integration.Freq");
      itsIntegration.deltaTime = ps.getDouble("Integration.Time");
    }


    BBSStrategy::~BBSStrategy()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Clean up all steps.
      for (uint i = 0; i < itsSteps.size(); ++i) {
	delete itsSteps[i];
      }
      itsSteps.clear();
    }


    void BBSStrategy::print(ostream& os) const
    {
      os << endl << indent << "Measurement Set: " << itsDataSet
	 << endl << indent << itsBBDB
	 << endl << indent << itsParmDB
	 << endl << indent << "Strategy:";
      Indent id;
      os << endl << indent << "Input data: " << itsInputData
	 << endl << indent << itsDomainSize
	 << endl << indent << itsCorrelation
	 << endl << indent << itsIntegration
	 << endl << indent << "Stations: " << itsStations;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	os << endl << indent << *itsSteps[i];
      }
    }


//     void BBSStrategy::addStep(const BBSStep*& aStep)
//     {
//       itsSteps.push_back(aStep);
//     }


    //##--------   G l o b a l   m e t h o d s   --------##//

    ostream& operator<<(ostream& os, const BBSStrategy& bs)
    {
      bs.print(os); 
      return os;
    }


  } // namespace BBS

} // namespace LOFAR
