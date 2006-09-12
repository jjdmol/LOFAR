//#  BBSSolveStep.cc: 
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

#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/StreamFormatting.h>
#include <APS/ParameterSet.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS
  {
    using ACC::APS::ParameterSet;
    using LOFAR::operator<<;

    BBSSolveStep::BBSSolveStep(const BBSStep* parent) : 
      BBSSingleStep(parent), 
      itsMaxIter(0), itsEpsilon(0), itsMinConverged(0)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    BBSSolveStep::BBSSolveStep(const string& name, 
			       const ParameterSet& parset,
			       const BBSStep* parent) :
      BBSSingleStep(name, parset, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Create a subset of \a parset, containing only the relevant keys for
      // the current BBSSingleStep.
      ParameterSet ps(parset.makeSubset("Step." + name + ".Solve."));

      // Get the relevant parameters from the Parameter Set \a parset. 
      // \note A missing key is considered an error.
      itsMaxIter                 = ps.getUint32("MaxIter");
      itsEpsilon                 = ps.getDouble("Epsilon");
      itsMinConverged            = ps.getDouble("MinConverged");
      itsParms                   = ps.getStringVector("Parms");
      itsExclParms               = ps.getStringVector("ExclParms");
      itsDomainSize.bandWidth    = ps.getDouble("DomainSize.Freq");
      itsDomainSize.timeInterval = ps.getDouble("DomainSize.Time");
    }


    BBSSolveStep::~BBSSolveStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    void BBSSolveStep::read(BlobIStream& bis)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      BBSSingleStep::read(bis);
      bis >> itsMaxIter
	  >> itsEpsilon
	  >> itsMinConverged
	  >> itsParms
	  >> itsExclParms
	  >> itsDomainSize;
    }


    void BBSSolveStep::write(BlobOStream& bos) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      BBSSingleStep::write(bos);
      bos << itsMaxIter
	  << itsEpsilon
	  << itsMinConverged
	  << itsParms
  	  << itsExclParms
	  << itsDomainSize;
    }


    void BBSSolveStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      BBSSingleStep::print(os);
      Indent id;
      os << endl << indent << "Solve: ";
      {
	Indent id;
	os << endl << indent << "Max nr. of iterations: "  << itsMaxIter
	   << endl << indent << "Convergence threshold: "  << itsEpsilon
	   << endl << indent << "Min fraction converged: " << itsMinConverged
	   << endl << indent << "Solvable parameters: "    << itsParms
	   << endl << indent << "Excluded parameters: "    << itsExclParms
	   << endl << indent << itsDomainSize;
      }
    }


    const string& BBSSolveStep::type() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      static const string theType("BBSSolveStep");
      return theType;
    }


  } // namespace BBS

} // namespace LOFAR
