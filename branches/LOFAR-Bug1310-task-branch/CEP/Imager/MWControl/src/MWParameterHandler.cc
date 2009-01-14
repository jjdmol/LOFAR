//# MWParameterHandler.cc: Handle the master-worker part of a .parset file
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <MWControl/MWParameterHandler.h>
#include <MWControl/MWStrategySpec.h>
#include <MWControl/MWMultiSpec.h>

using namespace std;

namespace LOFAR { namespace CEP {

  MWParameterHandler::MWParameterHandler (const ParameterSet& parSet)
    : ParameterHandler (parSet)
  {}

  string MWParameterHandler::getDataSetName() const
  {
    return getString ("DataSet");
  }

  int MWParameterHandler::getNParts() const
  {
    return getUint ("NNode", 1);
  }

  vector<MWStrategySpec> MWParameterHandler::getStrategies() const
  {
    // Get all strategy names.
    // Default name is 'Strategy'.
    vector<string> defName(1, "Strategy");
    vector<string> strategyNames(getStringVector("Strategies", defName));
    // Create a new strategy specification object for each name.
    vector<MWStrategySpec> specs;
    for (unsigned i=0; i<strategyNames.size(); ++i) {
      specs.push_back (MWStrategySpec (strategyNames[i], itsParms));
    }
    return specs;
  }

  MWMultiSpec MWParameterHandler::getSteps (const string& name) const
  {
    // Get all step names.
    vector<string> stepNames(itsParms.getStringVector(name+".Steps"));
    // Create a new step specification object for each name.
    MWMultiSpec specs;
    for (unsigned i=0; i<stepNames.size(); ++i) {
      specs.push_back (MWSpec::createSpec (stepNames[i], itsParms, 0));
    }
    return specs;
  }

}} // end namespaces
