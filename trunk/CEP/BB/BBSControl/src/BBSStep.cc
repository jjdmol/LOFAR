//#  BBSStep.cc: 
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

#include <BBSControl/BBSStep.h>
#include <BBSControl/BBSSingleStep.h>
#include <BBSControl/BBSMultiStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/Exceptions.h>
#include <Common/StreamUtil.h>
#include <APS/ParameterSet.h>
#include <APS/Exceptions.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  using ACC::APS::ParameterSet;
  using ACC::APS::APSException;

  namespace BBS
  {
    using LOFAR::operator<<;

    //##--------   P u b l i c   m e t h o d s   --------##//

    BBSStep::~BBSStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    string BBSStep::fullName() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      string name;
      if (itsParent) name = itsParent->fullName() + ".";
      name += itsName;
      return name;
    }


    vector<const BBSStep*> BBSStep::getAllSteps() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      vector<const BBSStep*> steps;
      doGetAllSteps(steps);
      return steps;
    }


    BBSStep* BBSStep::create(const string& name,
			     const ParameterSet& parset,
			     const BBSStep* parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // If \a parset contains a key <tt>Step.<em>name</em>.Steps</tt>, then
      // \a name is a BBSMultiStep, otherwise it is a SingleStep.
      if (parset.isDefined("Step." + name + ".Steps")) {
	LOG_TRACE_COND_STR(name << " is a MultiStep");
	return new BBSMultiStep(name, parset, parent);
      } else {
	LOG_TRACE_COND_STR(name << " is a SingleStep");
	// We'll have to figure out what kind of SingleStep we must
	// create. The key "Operation" contains this information.
	string oper = toUpper(parset.getString("Step." + name + ".Operation"));
	LOG_TRACE_COND_STR("Creating a " << oper << " step ...");
	if      (oper == "SOLVE")
	  return new BBSSolveStep(name, parset, parent);
	else if (oper == "SUBTRACT")
	  return new BBSSubtractStep(name, parset, parent);
	else if (oper == "CORRECT")
	  return new BBSCorrectStep(name, parset, parent);
	else if (oper == "PREDICT")
	  return new BBSPredictStep(name, parset, parent);
	else if (oper == "SHIFT")
	  return new BBSShiftStep(name, parset, parent);
	else if (oper == "REFIT")
	  return new BBSRefitStep(name, parset, parent);
	else THROW (BBSControlException, "Operation \"" << oper << 
		    "\" is not a valid Step operation");
      }
    }


    //##--------   P r o t e c t e d   m e t h o d s   --------##//

    BBSStep::BBSStep(const string& name, 
		     const ParameterSet& parset,
		     const BBSStep* parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Copy the data members from the parent, if present, so that they have
      // sensible default values.
      if (parent) *this = *parent;

      // We must reset these values, because they were overwritten by the
      // previous data copy.
      itsName = name;
      itsParent = parent;

      // Overrride default values for data members of the current BBSStep, if
      // they're specified in \a parset.
      setParms(parset.makeSubset("Step." + name + "."));

    }


    void BBSStep::read(BlobIStream& bis)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      bis >> itsName
	  >> itsBaselines
	  >> itsCorrelation
 	  >> itsIntegration
 	  >> itsSources
 	  >> itsExtraSources
 	  >> itsInstrumentModels;
    }


    void BBSStep::write(BlobOStream& bos) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      bos << itsName
 	  << itsBaselines
 	  << itsCorrelation
 	  << itsIntegration
 	  << itsSources
 	  << itsExtraSources
 	  << itsInstrumentModels;
    }


    void BBSStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      os << "Step: " << itsName;
      Indent id;  // add an extra indentation level
      os << endl << indent << "Full name: " << fullName()
	 << endl << indent << itsBaselines
	 << endl << indent << itsCorrelation
	 << endl << indent << itsIntegration
	 << endl << indent << "Sources: " << itsSources
	 << endl << indent << "Extra sources: " << itsExtraSources
	 << endl << indent << "Instrument models: " << itsInstrumentModels;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void BBSStep::setParms(const ParameterSet& ps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // If defined, get the baseline selection for this step.
      try {
	itsBaselines.station1 = ps.getStringVector("Baselines.Station1");
	itsBaselines.station2 = ps.getStringVector("Baselines.Station2");
      } catch (APSException&) {}

      // If defined, get the correlation selection (ALL, AUTO, or CROSS), and
      // type (e.g., ["XX", "XY", "YX", "YY"]
      try {
	string sel = ps.getString("Correlation.Selection");
	if      (sel == "ALL")   itsCorrelation.selection = Correlation::ALL;
	else if (sel == "AUTO")  itsCorrelation.selection = Correlation::AUTO;
	else if (sel == "CROSS") itsCorrelation.selection = Correlation::CROSS;
	else THROW(BBSControlException, 
		   "Invalid correlation selection " << sel);
	itsCorrelation.type = ps.getStringVector("Correlation.Type");
      } catch (APSException&) {}

      // If defined, get the integration intervals in frequency (Hz) and
      // time (s).
      try {
	itsIntegration.deltaFreq = ps.getDouble("Integration.Freq");
	itsIntegration.deltaTime = ps.getDouble("Integration.Time");
      } catch (APSException&) {}

      // If defined, get the sources for the current patch.
      try {
	itsSources = ps.getStringVector("Sources");
      } catch (APSException&) {}

      // If defined, get the extra source, outside the current patch.
      try {
	itsExtraSources = ps.getStringVector("ExtraSources");
      } catch (APSException&) {}

      // If defined, get the instrument model(s) used.
      try {
	itsInstrumentModels = ps.getStringVector("InstrumentModel");
      } catch (APSException&) {}

    }


    void BBSStep::doGetAllSteps(vector<const BBSStep*>& steps) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      steps.push_back(this);
    }
    

    //##--------   G l o b a l   m e t h o d s   --------##//

    ostream& operator<<(ostream& os, const BBSStep& bs)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      bs.print(os);
      return os;
    }


  } // namespace BBS

} // namespace LOFAR
