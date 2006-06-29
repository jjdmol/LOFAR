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
#include <Common/StreamUtil.h>

namespace LOFAR
{
  using ACC::APS::ParameterSet;

  namespace BBS
  {

    using LOFAR::operator<<;

    ostream& operator<<(ostream& os, const BBSStrategy::WorkDomainSize& wds)
    {
      os << indent << "Workdomain size:" << endl;
      Indent id;  // add an indentation level
      os << indent << "Bandwidth: " << wds.bandWidth << " (Hz)" << endl
	 << indent << "Time interval: " << wds.timeInterval << " (s)";
      return os;
    }

    ostream& operator<<(ostream& os, const BBSStrategy::Selection& sel)
    {
      os << indent << "Selection: ";
      switch(sel.corr) {
      case BBSStrategy::Selection::ALL:     os << "ALL";   break;
      case BBSStrategy::Selection::CROSS:   os << "CROSS"; break;
      case BBSStrategy::Selection::AUTO:    os << "AUTO";  break;
      default: os << "*****"; break;
      }
      return os;
    }

    ostream& operator<<(ostream& os, const BBSStrategy::BBDB& bbdb)
    {
      os << indent << "Blackboard database:" << endl;
      Indent id; // add an indentation level
      os << indent << "Host: " << bbdb.host << endl
	 << indent << "Port: " << bbdb.port;
      return os;
    }


    BBSStrategy::BBSStrategy(const ParameterSet& aParSet)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Create a subset of \a aParSet, containing only the relevant keys for
      // a Strategy.
      ParameterSet ps(aParSet.makeSubset("Strategy."));

      // This strategy consists of the following steps.
      vector<string> steps(ps.getStringVector("Steps"));

      // Create a new step for each name in \a steps.
      for (uint i = 0; i < steps.size(); ++i) {
	itsSteps.push_back(BBSStep::create(steps[i], aParSet));
      }

      // ID's of the stations to be used by this strategy.
      itsStations = ps.getUint32Vector("Stations");

      // Get the work domain size for this strategy
      itsDomainSize.bandWidth = ps.getDouble("DomainSize.Freq");
      itsDomainSize.timeInterval = ps.getDouble("DomainSize.Time");

      // Get the names of the Measurement Sets
      itsDataSet = ps.getString("DataSet");

      // Get the correlation product selection (ALL, AUTO, or CROSS)
      string sel = ps.getString("Selection");
      if (sel == "ALL") itsSelection = Selection::ALL;
      else if (sel == "AUTO") itsSelection = Selection::AUTO;
      else if (sel == "CROSS") itsSelection = Selection::CROSS;
      else THROW(BBSControlException, 
		 "Invalid correlation selection " << sel);

      // Get the hostname/ipaddr and portnr of the Blackboard database
      itsBBDB.host = ps.getString("BBDB.Host");
      itsBBDB.port = ps.getUint16("BBDB.Port");

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
      os << indent << "Strategy:" << endl;
      Indent id;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	itsSteps[i]->print(os);
      }
      os << indent << "Stations: " << itsStations << endl
	 << itsDomainSize << endl
	 << indent << "Data set: " << itsDataSet << endl
	 << itsSelection << endl
	 << itsBBDB << endl;
    }


//     void BBSStrategy::addStep(const BBSStep*& aStep)
//     {
//       itsSteps.push_back(aStep);
//     }

    ostream& operator<<(ostream& os, const BBSStrategy& bs)
    {
      bs.print(os); 
      return os;
    }


  } // namespace BBS

} // namespace LOFAR
