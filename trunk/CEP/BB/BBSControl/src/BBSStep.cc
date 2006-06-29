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
#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>

namespace LOFAR
{
  using ACC::APS::ParameterSet;

  namespace BBS
  {
    using LOFAR::operator<<;

    BBSStep::BBSStep(const string& name, const ParameterSet& parSet) :
      itsName(name)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Create a subset of \a aParSet, containing only the relevant keys for
      // the current BBSStep.
      ParameterSet ps(parSet.makeSubset("Step." + name + "."));

      // Get the baseline selection for this step.
      itsSelection.station1 = ps.getUint32Vector("Selection.Station1");
      itsSelection.station2 = ps.getUint32Vector("Selection.Station2");

      // Get the sources for the current source model.
      itsSources = ps.getStringVector("Sources");
    }


    BBSStep::~BBSStep()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }

//     void BBSStep::addStep(const BBSStep*&)
//     {
//       ASSERTSTR(false, "A BBSStep can only be added to a BBSMultiStep");
//     }


    BBSStep* BBSStep::create(const string& name,
			     const ParameterSet& parSet)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // If \a parSet contains a key <tt>Step.<em>name</em>.Steps</tt>, then
      // \a name is a BBSMultiStep, otherwise it is a SingleStep.
      if (parSet.isDefined("Step." + name + ".Steps")) {
	LOG_TRACE_COND_STR(name << " is a MultiStep");
	return new BBSMultiStep(name, parSet);
      } else {
	LOG_TRACE_COND_STR(name << " is a SingleStep");
	return new BBSSingleStep(name, parSet);
      }
    }


    void BBSStep::print(ostream& os) const
    {
      os << indent << "Step: " << itsName << endl;
      Indent id;  // add an extra indentation level
      os << itsSelection << endl
	 << indent << "Sources: " << itsSources << endl;
    }


    ostream& operator<<(ostream& os, const BBSStep& bs)
    {
      bs.print(os);
      return os;
    }

    ostream& operator<<(ostream& os, const BBSStep::Selection& sel)
    {
      os << indent << "Selection:" << endl;
      Indent id;  // add an extra indentation level
      os << indent << "Station1: " << sel.station1 << endl
	 << indent << "Station2: " << sel.station2;
      return os;
    }


  } // namespace BBS

} // namespace LOFAR
