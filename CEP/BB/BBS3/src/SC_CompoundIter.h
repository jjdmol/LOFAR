//#  SC_CompoundIter.h: A calibration strategy which sends compound workorders.
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

#ifndef LOFAR_BBS3_SC_COMPOUNDITER_H
#define LOFAR_BBS3_SC_COMPOUNDITER_H

// \file SC_CompoundIter
// A calibration strategy which sends compound workorders (multiple iterations).// NB. This strategy only works correctly when Prediffer and Solver run in 
// separate processes.

//# Includes
#include <BBS3/StrategyController.h>
#include <Common/lofar_string.h>
#include <APS/ParameterSet.h>

namespace LOFAR
{

// \addtogroup BBS3
// @{

//# Forward Declarations

using ACC::APS::ParameterSet;

class SC_CompoundIter : public StrategyController
{
public:
  SC_CompoundIter(Connection* inSolConn, Connection* outWOPDConn, 
	    Connection* outWOSolveConn, int nrPrediffers,
	    const ParameterSet& args);

  virtual ~SC_CompoundIter();

  /// Execute the strategy
  virtual bool execute();

  /// Postprocess
  virtual void postprocess();
    
  /// Get strategy type
  virtual string getType() const;

 private:
  SC_CompoundIter(const SC_CompoundIter&);
  SC_CompoundIter& operator=(const SC_CompoundIter&);

  void readFinalSolution();

  bool         itsFirstCall;
  int          itsPrevWOID;
  ParameterSet itsArgs;
  int          itsMaxIterations;
  double       itsFitCriterion;
  int          itsCurIter;
  double       itsCurStartTime;
  bool         itsControlParmUpd;    // Does this Controller update the parameters?
  bool         itsWriteParms;        // Write the parameters in the parmtable at the end of each interval?
  double       itsStartTime;
  double       itsTimeLength;
  double       itsStartFreq;
  double       itsFreqLength;
};

inline string SC_CompoundIter::getType() const
{ return "CompoundIter"; }

// @}

} // namespace LOFAR

#endif
