//#  Strategy.cc: 
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

#include <BBSControl/Strategy.h>
#include <BBSControl/Step.h>
#include <BBSControl/Types.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/Exceptions.h>
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <BBSControl/StreamUtil.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

  namespace BBS
  {
    using LOFAR::operator<<;

    // Register Strategy with the CommandFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
      bool dummy = CommandFactory::instance().
        registerClass<Strategy>("Strategy");
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    Strategy::Strategy(const ParameterSet& aParSet) :
      itsWriteSteps(false)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Get the name of the Measurement Set.
      itsDataSet = aParSet.getString("DataSet");

      // Retrieve the parameter database related key/value pairs.
      itsPDB.instrument = aParSet.getString("ParmDB.Instrument");
      itsPDB.sky = aParSet.getString("ParmDB.Sky");
      itsPDB.history = aParSet.getString("ParmDB.History");

      // Create a subset of \a aParSet, containing only the relevant keys for
      // a Strategy.
      ParameterSet ps(aParSet.makeSubset("Strategy."));

      // ID's of the stations to be used by this strategy.
      itsStations = ps.getStringVector("Stations");

      // Get the name of the MS input column
      itsInputColumn = ps.getString("InputColumn");

      // Get the region of interest (optional)
      itsRegionOfInterest.freq =
        ps.getUint32Vector("RegionOfInterest.Freq", vector<uint32>());
      itsRegionOfInterest.time =
        ps.getStringVector("RegionOfInterest.Time", vector<string>());

      // Get the chunk size for this strategy
      itsChunkSize = ps.getUint32("ChunkSize", 0);

      // Get the correlation product selection (ALL, AUTO, or CROSS)
      itsCorrelation.selection = 
        ps.getString("Correlation.Selection");
      itsCorrelation.type = 
        ps.getStringVector("Correlation.Type", vector<string>());

      // This strategy consists of the following steps.
      vector<string> steps(ps.getStringVector("Steps"));

      // Create a new step for each name in \a steps.
      for (uint i = 0; i < steps.size(); ++i) {
        itsSteps.push_back(Step::create(steps[i], aParSet, 0));
      }
    }


    Strategy::~Strategy()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    void Strategy::accept(CommandVisitor &visitor) const
    {
      visitor.visit(*this);
    }


    void Strategy::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      os << endl << indent << "Measurement Set: " << itsDataSet
	 << endl << indent << itsPDB
	 << endl << indent << "Strategy:";
      Indent id;
      os << endl << indent << "Input column: " << itsInputColumn
	 << endl << indent << itsRegionOfInterest
	 << endl << indent << "Chunk size: " << itsChunkSize
	 << endl << indent << itsCorrelation
	 << endl << indent << "Stations: " << itsStations;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	os << endl << indent << *itsSteps[i];
      }
    }
 

    vector< shared_ptr<const Step> > Strategy::getAllSteps() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      vector< shared_ptr<const Step> > steps;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	vector< shared_ptr<const Step> > substeps =
          itsSteps[i]->getAllSteps();
	steps.insert(steps.end(), substeps.begin(), substeps.end());
      }
      return steps;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void Strategy::write(ParameterSet& ps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      ps.add("DataSet", itsDataSet);
      ps.add("ParmDB.Instrument", itsPDB.instrument);
      ps.add("ParmDB.Sky", itsPDB.sky);
      ps.add("ParmDB.History", itsPDB.history);
      ps.add("Strategy.Stations", toString(itsStations));
      ps.add("Strategy.InputColumn", itsInputColumn);
      ps.add("Strategy.ChunkSize", toString(itsChunkSize));
      ps.add("Strategy.RegionOfInterest.Freq", 
             toString(itsRegionOfInterest.freq));
      ps.add("Strategy.RegionOfInterest.Time",
             toString(itsRegionOfInterest.time));
      ps.add("Strategy.Correlation.Selection", itsCorrelation.selection);
      ps.add("Strategy.Correlation.Type", toString(itsCorrelation.type));
      LOG_TRACE_COND_STR("Write the Step objects as well?  " <<
                         (itsWriteSteps ? "Yes" : "No"));
      if (itsWriteSteps) writeSteps(ps);
    }


    void Strategy::read(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      itsDataSet                 = ps.getString("DataSet");
      itsPDB.instrument          = ps.getString("ParmDB.Instrument");
      itsPDB.sky                 = ps.getString("ParmDB.Sky");
      itsPDB.history             = ps.getString("ParmDB.History");
      itsStations                = ps.getStringVector("Strategy.Stations");
      itsInputColumn             = ps.getString("Strategy.InputColumn");
      itsChunkSize               = ps.getUint32("Strategy.ChunkSize");
      itsRegionOfInterest.freq   =
        ps.getUint32Vector("Strategy.RegionOfInterest.Freq");
      itsRegionOfInterest.time   =
        ps.getStringVector("Strategy.RegionOfInterest.Time");
      itsCorrelation.selection   =
        ps.getString("Strategy.Correlation.Selection");
      itsCorrelation.type        = 
        ps.getStringVector("Strategy.Correlation.Type");

      // Read back the Step objects? Set \c itsWriteSteps to \c false, if
      // no steps were specified in the parameter set.
      itsWriteSteps = readSteps(ps);
      LOG_TRACE_COND_STR("Read the Step objects as well?  " <<
                         (itsWriteSteps ? "Yes" : "No"));
    }


    const string& Strategy::type() const
    {
      static const string theType("Strategy");
      return theType;
    }


    bool Strategy::readSteps(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      vector<string> steps;
      try {
        steps = ps.getStringVector("Strategy.Steps");
        LOG_TRACE_COND_STR("Strategy.Steps = " << steps);
        for (uint i = 0; i < steps.size(); ++i) {
          itsSteps.push_back(Step::create(steps[i], ps, 0));
        }
        return true;
      } catch (APSException&) {
        return false;
      }
    }


    void Strategy::writeSteps(ParameterSet& ps) const
    {
      ostringstream oss;

      // Write the "Steps" key/value pair
      oss << "Strategy.Steps = [ ";
      for (uint i = 0; i < itsSteps.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << itsSteps[i]->name();
      }
      oss << " ]";
      ps.adoptBuffer(oss.str());

      // Write the Step objects, one by one.
      for (uint i = 0; i < itsSteps.size(); ++i) {
        itsSteps[i]->write(ps);
      }
    }


    //##--------   G l o b a l   m e t h o d s   --------##//

    ostream& operator<<(ostream& os, const Strategy& bs)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      bs.print(os); 
      return os;
    }

  } // namespace BBS

} // namespace LOFAR
