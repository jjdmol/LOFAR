//#  WH_Solve.h: predicts visibilities and determines difference to measured
//#                data.
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

#ifndef BBS3_WH_SOLVE_H
#define BBS3_WH_SOLVE_H


//# Includes
#include <tinyCEP/WorkHolder.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{
//# Forward Declarations
class Solver;

// This workholder class solves for a certain domain

class WH_Solve : public LOFAR::WorkHolder
{
 public:
  // Construct the workholder and give it a name
  explicit WH_Solve(const string& name, int nPrediffInputs);
  
  // Destructor
  virtual ~WH_Solve();
  
  // Make a fresh copy of the WH object.
  virtual WH_Solve* make (const string& name);

  // Preprocess
  virtual void preprocess();

  // Do a process step.
  virtual void process();
  
  // Show the workholder on stdout.
  virtual void dump();

 private:
  // Forbid copy constructor
  WH_Solve(const WH_Solve&);

  // Forbid assignment
  WH_Solve& operator= (const WH_Solve&);

  // Create a Solver object
  void createSolver(const KeyValueMap& args);

  // Read all Prediffer inputs
  void readInputs();
  
  // Read all Prediffer inputs and set solvable parameter data.
  void readInputsAndSetParmData();

  int         itsNPrediffers;// Number of Prediffer inputs
  KeyValueMap itsArgs;       // Arguments
  Solver*     itsSolver;     // Object actually calculating results

};

} // namespace LOFAR

#endif
