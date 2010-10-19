//#  BBSStrategy.cc: 
//#
//#  Copyright (C) 2002-2007
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
#include <BBSControl/BBSStructs.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/Exceptions.h>
#include <APS/ParameterSet.h>
#include <APS/Exceptions.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  using ACC::APS::ParameterSet;
  using ACC::APS::APSException;

  namespace BBS
  {
    using LOFAR::operator<<;

    // Register BBSStrategy with the CommandFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
      bool dummy = CommandFactory::instance().
	registerClass<BBSStrategy>("BBSStrategy");
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    BBSStrategy::BBSStrategy(const ParameterSet& aParSet) :
      itsWriteSteps(false)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Get the name of the Measurement Set.
      itsDataSet = aParSet.getString("DataSet");

      // Retrieve the parameter database related key/value pairs.
      itsParmDB.instrument = aParSet.getString("ParmDB.Instrument");
      itsParmDB.localSky = aParSet.getString("ParmDB.LocalSky");
      itsParmDB.history = aParSet.getString("ParmDB.History");

      // Create a subset of \a aParSet, containing only the relevant keys for
      // a Strategy.
      ParameterSet ps(aParSet.makeSubset("Strategy."));

      // ID's of the stations to be used by this strategy.
      itsStations = ps.getStringVector("Stations");

      // Get the name of the MS input data column
      itsInputData = ps.getString("InputData");

      // Get the region of interest (optional)
      try {
        itsRegionOfInterest.frequency = 
          ps.getInt32Vector("RegionOfInterest.Freq");          
        itsRegionOfInterest.time  = 
          ps.getStringVector("RegionOfInterest.Time");
      } catch (APSException&) {}
      
      // Get the work domain size for this strategy
      itsDomainSize.bandWidth = ps.getDouble("WorkDomainSize.Freq");      
      itsDomainSize.timeInterval = ps.getDouble("WorkDomainSize.Time");
      
      // Get the correlation product selection (ALL, AUTO, or CROSS)
      itsCorrelation.selection = ps.getString("Correlation.Selection");
      itsCorrelation.type = ps.getStringVector("Correlation.Type");

      // Get the integration intervals in frequency (Hz) and time (s).
      try {
        itsIntegration.deltaFreq = ps.getDouble("Integration.Freq");
        itsIntegration.deltaTime = ps.getDouble("Integration.Time");
      } catch (APSException&) {}

      // This strategy consists of the following steps.
      try {
        vector<string> steps(ps.getStringVector("Steps"));

        // Create a new step for each name in \a steps.
        for (uint i = 0; i < steps.size(); ++i) {
          itsSteps.push_back(BBSStep::create(steps[i], aParSet, 0));
        }
      } catch (APSException&) {}
    }


    BBSStrategy::~BBSStrategy()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Clean up all steps.
      for (uint i = 0; i < itsSteps.size(); ++i) {
        delete itsSteps[i];
      }
      itsSteps.clear();
    }


    void BBSStrategy::accept(CommandVisitor &visitor) const
    {
      visitor.visit(*this);
    }


    void BBSStrategy::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      os << endl << indent << "Measurement Set: " << itsDataSet
	 << endl << indent << itsParmDB
	 << endl << indent << "Strategy:";
      Indent id;
      os << endl << indent << "Input data: " << itsInputData
	 << endl << indent << "Region of interest: " << itsRegionOfInterest
	 << endl << indent << itsDomainSize
	 << endl << indent << itsCorrelation
	 << endl << indent << itsIntegration
	 << endl << indent << "Stations: " << itsStations;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	os << endl << indent << *itsSteps[i];
      }
    }
 

    vector<const BBSStep*> BBSStrategy::getAllSteps() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      vector<const BBSStep*> steps;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	vector<const BBSStep*> substeps = itsSteps[i]->getAllSteps();
	steps.insert(steps.end(), substeps.begin(), substeps.end());
      }
      return steps;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void BBSStrategy::write(ACC::APS::ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      ostringstream oss;
      oss << endl << "DataSet = " 
          << itsDataSet 
          << endl << "ParmDB.Instrument = " 
          << itsParmDB.instrument
          << endl << "ParmDB.LocalSky = " 
          << itsParmDB.localSky
          << endl << "ParmDB.History = " 
          << itsParmDB.history
          << endl << "Strategy.Stations = " 
          << itsStations
          << endl << "Strategy.InputData = "
          << itsInputData
        //           << endl << "Strategy.RegionOfInterest = "
        //           << itsRegionOfInterest
          << endl << "Strategy.WorkDomainSize.Freq = " 
          << itsDomainSize.bandWidth
          << endl << "Strategy.WorkDomainSize.Time = "
          << itsDomainSize.timeInterval
          << endl << "Strategy.Correlation.Selection = "
          << itsCorrelation.selection
          << endl << "Strategy.Correlation.Type = "
          << itsCorrelation.type
          << endl << "Strategy.Integration.Freq = "
          << itsIntegration.deltaFreq
          << endl << "Strategy.Integration.Time = "
          << itsIntegration.deltaTime;
      ps.adoptBuffer(oss.str());

      LOG_TRACE_COND_STR("Write the BBSStep objects as well?  " <<
                         (itsWriteSteps ? "Yes" : "No"));
      if (itsWriteSteps) writeSteps(ps);
    }


    void BBSStrategy::read(const ACC::APS::ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      itsDataSet                 = ps.getString("DataSet");
      itsParmDB.instrument       = ps.getString("ParmDB.Instrument");
      itsParmDB.localSky         = ps.getString("ParmDB.LocalSky");
      itsParmDB.history          = ps.getString("ParmDB.History");
      itsStations                = ps.getStringVector("Strategy.Stations");
      itsInputData               = ps.getString("Strategy.InputData");
      //       itsRegionOfInterest        = ps.getXXX();
      itsDomainSize.bandWidth    = 
        ps.getDouble("Strategy.WorkDomainSize.Freq");
      itsDomainSize.timeInterval = 
        ps.getDouble("Strategy.WorkDomainSize.Time");
      itsCorrelation.selection   = 
        ps.getString("Strategy.Correlation.Selection");
      itsCorrelation.type        = 
        ps.getStringVector("Strategy.Correlation.Type");
      itsIntegration.deltaFreq   = ps.getDouble("Strategy.Integration.Freq");
      itsIntegration.deltaTime   = ps.getDouble("Strategy.Integration.Time");

      // Read back the BBSStep objects? Set \c itsWriteSteps to \c false, if
      // no steps were specified in the parameter set.
      itsWriteSteps = readSteps(ps);
      LOG_TRACE_COND_STR("Read the BBSStep objects as well?  " <<
                         (itsWriteSteps ? "Yes" : "No"));
    }


    const string& BBSStrategy::type() const
    {
      static const string theType("Strategy");
      return theType;
    }


    bool BBSStrategy::readSteps(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      vector<string> steps;
      try {
        steps = ps.getStringVector("Strategy.Steps");
        LOG_TRACE_COND_STR("Strategy.Steps = " << steps);
        for (uint i = 0; i < steps.size(); ++i) {
          itsSteps.push_back(BBSStep::create(steps[i], ps, 0));
        }
        return true;
      } catch (APSException&) {
        return false;
      }
    }


    void BBSStrategy::writeSteps(ParameterSet& ps) const
    {
      ostringstream oss;

      // Write the "Steps" key/value pair
      oss << "Strategy.Steps = [ ";
      for (uint i = 0; i < itsSteps.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << itsSteps[i]->getName();
      }
      oss << " ]";
      ps.adoptBuffer(oss.str());

      // Write the BBSStep objects, one by one.
      for (uint i = 0; i < itsSteps.size(); ++i) {
        itsSteps[i]->write(ps);
      }
    }


    //##--------   G l o b a l   m e t h o d s   --------##//

    ostream& operator<<(ostream& os, const BBSStrategy& bs)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      bs.print(os); 
      return os;
    }


//     ParameterSet& operator<<(ParameterSet& ps, const BBSStrategy& bs)
//     {
//       LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
//       bs.write(ps); 
//       return ps;
//     }


//     ParameterSet& operator>>(ParameterSet& ps, BBSStrategy& bs)
//     {
//       LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
//       bs.read(ps); 
//       return ps;
//     }


  } // namespace BBS

} // namespace LOFAR
