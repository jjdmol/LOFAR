//#  MWParameterHandler.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

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
