//#  WH_Prediff.h: predicts visibilities and determines difference to measured
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

#ifndef BBS3_WH_PREDIFF_H
#define BBS3_WH_PREDIFF_H


//# Includes
#include <tinyCEP/WorkHolder.h>
#include <Common/KeyValueMap.h>

namespace LOFAR
{
//# Forward Declarations
class Prediffer;
 class ParmData;

  // This workholder class predicts (calculates) visibilities based on current values
  // of the parameters and determines the difference to the measured data for a 
  // certain domain.
class WH_Prediff : public LOFAR::WorkHolder
{
 public:
  // Construct the workholder and give it a name
  explicit WH_Prediff(const string& name, int id);
  
  // Destructor
  virtual ~WH_Prediff();
  
  // Make a fresh copy of the WH object.
  virtual WH_Prediff* make (const string& name);

  // Preprocess
  virtual void preprocess();

  // Do a process step.
  virtual void process();
  
  // Show the workholder on stdout.
  virtual void dump();

 private:
  typedef map<int, Prediffer*> PrediffMap;

  // Forbid copy constructor
  WH_Prediff(const WH_Prediff&);

  // Forbid assignment
  WH_Prediff& operator= (const WH_Prediff&);

  // Create a Prediffer object or get it from the Map
  Prediffer* getPrediffer(int id, 
			  const KeyValueMap& args, 
			  const vector<int>& antNrs);
  // Read the next workorder
  void readWorkOrder();

  // Read a parameter solution
  void readSolution(int id, vector<ParmData>& solVec);

  int         itsID;         // Identification number
  KeyValueMap itsArgs;       // Arguments
  PrediffMap  itsPrediffs;   // Map of Prediffer objects, each associated
                                   // with a strategy (controller)
};

} // namespace LOFAR

#endif
