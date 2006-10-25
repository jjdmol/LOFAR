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
#include <BBSControl/BBSStructs.h>
#include <BBSControl/Exceptions.h>
#include <APS/ParameterSet.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  using ACC::APS::ParameterSet;

  namespace BBS
  {
    using LOFAR::operator<<;

    // Register BBSStrategy with the BBSStreamableFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
      bool dummy = BlobStreamableFactory::instance().
	registerClass<BBSStrategy>("BBSStrategy");
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    BBSStrategy::BBSStrategy(const ParameterSet& aParSet) :
      itsWriteSteps(false)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

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
      itsParmDB.history = aParSet.getString("ParmDB.History");
      
      // Create a subset of \a aParSet, containing only the relevant keys for
      // a Strategy.
      ParameterSet ps(aParSet.makeSubset("Strategy."));

      // ID's of the stations to be used by this strategy.
      itsStations = ps.getStringVector("Stations");

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

      // This strategy consists of the following steps.
      vector<string> steps(ps.getStringVector("Steps"));

      // Create a new step for each name in \a steps.
      for (uint i = 0; i < steps.size(); ++i) {
	itsSteps.push_back(BBSStep::create(steps[i], aParSet, 0));
      }
    }


    BBSStrategy::~BBSStrategy()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Clean up all steps.
      for (uint i = 0; i < itsSteps.size(); ++i) {
	delete itsSteps[i];
      }
      itsSteps.clear();
    }


    void BBSStrategy::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
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


    vector<const BBSStep*> BBSStrategy::getAllSteps() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      vector<const BBSStep*> steps;
      for (uint i = 0; i < itsSteps.size(); ++i) {
	vector<const BBSStep*> substeps = itsSteps[i]->getAllSteps();
	steps.insert(steps.end(), substeps.begin(), substeps.end());
      }
      return steps;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void BBSStrategy::read(BlobIStream& bis)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      bis >> itsDataSet
	  >> itsBBDB
	  >> itsParmDB
	  >> itsStations
	  >> itsInputData
	  >> itsDomainSize
	  >> itsCorrelation
	  >> itsIntegration;
      // Do we also need to deserialize the BBSStep objects?
      bis >> itsWriteSteps;
      LOG_TRACE_COND_STR("Deserialize the BBSStep objects as well?  " <<
			 (itsWriteSteps ? "Yes" : "No"));
      if (itsWriteSteps) readSteps(bis);
    }

    
    void BBSStrategy::write(BlobOStream& bos) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      bos << itsDataSet
	  << itsBBDB
	  << itsParmDB
	  << itsStations
	  << itsInputData
	  << itsDomainSize
	  << itsCorrelation
	  << itsIntegration;
      // Do we also need to serialize the BBSStep objects?
      bos << itsWriteSteps;
      LOG_TRACE_COND_STR("Serialize the BBSStep objects as well?  " <<
		    (itsWriteSteps ? "Yes" : "No"));
      if (itsWriteSteps) writeSteps(bos);
    }


    const string& BBSStrategy::classType() const
    {
      static const string theType("BBSStrategy");
      return theType;
    }


    void BBSStrategy::readSteps(BlobIStream& bis)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // How many BBSStep objects does this BBSStrategy contain?
      uint32 sz;
      bis >> sz;
      LOG_TRACE_VAR_STR("This BBSStrategy contains " << sz << 
			" BBSStep objects.");
      
      // Create the new BBSSteps by reading the blob input stream.
      for (uint i = 0; i < sz; ++i) {
	BBSStep* step;
	ASSERT(step = dynamic_cast<BBSStep*>(BlobStreamable::deserialize(bis)));
	itsSteps.push_back(step);
      }
    }


    void BBSStrategy::writeSteps(BlobOStream& bos) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Write the number of BBSStep objects that this BBSStrategy contains.
      bos << static_cast<uint32>(itsSteps.size());

      // Write the BBSStep objects, one by one.
      for (uint i = 0; i < itsSteps.size(); ++i) {
	itsSteps[i]->serialize(bos);
      }
    }


    //##--------   G l o b a l   m e t h o d s   --------##//

    ostream& operator<<(ostream& os, const BBSStrategy& bs)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      bs.print(os); 
      return os;
    }


  } // namespace BBS

} // namespace LOFAR
