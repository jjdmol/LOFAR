//#  SC_WritePredData.h: A strategy to make prediffers write predicted data
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

#ifndef LOFAR_BBS3_SC_WRITEPREDDATA_H
#define LOFAR_BBS3_SC_WRITEPREDDATA_H

// \file
// A strategy to make prediffers write predicted data

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

class SC_WritePredData : public StrategyController
{
public:
  SC_WritePredData(Connection* inSolConn, Connection* outWOPDConn, 
	    Connection* outWOSolveConn, int nrPrediffers,
	    const ParameterSet& args);

  virtual ~SC_WritePredData();

  /// Execute the strategy
  virtual bool execute();

  /// Postprocess
  virtual void postprocess();
    
  /// Get strategy type
  virtual string getType() const;

 private:
  SC_WritePredData(const SC_WritePredData&);
  SC_WritePredData& operator=(const SC_WritePredData&);

  bool         itsFirstCall;
  ParameterSet itsArgs;
  double       itsCurStartTime;
  bool         itsWriteInDataCol;
  double       itsStartTime;
  double       itsTimeLength;
  double       itsStartFreq;
  double       itsFreqLength;
};

inline string SC_WritePredData::getType() const
{ return "WritePredData"; }

// @}

} // namespace LOFAR

#endif
