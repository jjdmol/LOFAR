//#  SC_Simple.h: A simple calibration strategy
//#
//#  Copyright (C) 2002-2003
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

#ifndef LOFAR_BBSCONTROL_SC_SIMPLE_H
#define LOFAR_BBSCONTROL_SC_SIMPLE_H

// \file
// A simple calibration strategy

//# Includes
#include <BBSControl/StrategyController.h>
#include <Common/lofar_string.h>
#include <APS/ParameterSet.h>

namespace LOFAR
{
  namespace BBS
  {

    // \addtogroup BBS
    // @{

    //# Forward Declarations

    class SC_Simple : public StrategyController
    {
    public:
      SC_Simple(Connection* inSolConn, Connection* outWOPDConn, 
		Connection* outWOSolveConn, int nrPrediffers,
		const ACC::APS::ParameterSet& args);

      virtual ~SC_Simple();

      // Execute the strategy
      virtual bool execute();

      // Preprocess
      virtual void preprocess();

      // Postprocess
      virtual void postprocess();
    
      // Get strategy type
      virtual string getType() const;

    private:
      SC_Simple(const SC_Simple&);
      SC_Simple& operator=(const SC_Simple&);

      void readSolution();

      int          itsPrevWOID;
      ACC::APS::ParameterSet itsArgs;
      int          itsNrIterations;
      double       itsFitCriterion;
      int          itsCurIter;
      double       itsCurStartTime;
      // Does this Controller update the parameters?
      bool         itsControlParmUpd;
      // Write the parameters in the parmtable at the end of each interval?
      bool         itsWriteParms;
      double       itsStartTime;
      double       itsEndTime;
      double       itsTimeLength;
      int          itsStartChannel;
      int          itsEndChannel;
      // Flag indicating whether the previous sent workorder was a "do
      // nothing"
      bool         itsSendDoNothingWO;
    };

    inline string SC_Simple::getType() const
    { return "Simple"; }

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
