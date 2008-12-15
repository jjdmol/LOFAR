//#  MWMultiSpec.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <MWControl/MWMultiSpec.h>

using namespace LOFAR;
using namespace std;

namespace LOFAR { namespace CEP {

  MWMultiSpec::MWMultiSpec()
  {}

  MWMultiSpec::MWMultiSpec(const string& name,
			   const ParameterSet& parset,
			   const MWSpec* parent)
    : itsSpec(name, parset, parent)
  {
    // Get the names of the specs it consists of.
    vector<string> specs(parset.getStringVector("Step." + name + ".Steps"));
    // Create a new spec for each name in \a specs.
    for (uint i=0; i<specs.size(); ++i) {
      if (parent) {
	parent->infiniteRecursionCheck (specs[i]);
      }
      push_back (MWSpec::createSpec(specs[i], parset, &itsSpec));
    }
  }

  MWMultiSpec::~MWMultiSpec()
  {}

  MWStep::ShPtr MWMultiSpec::create()
  {
    return MWStep::ShPtr (new MWMultiSpec());
  }

  MWMultiSpec* MWMultiSpec::clone() const
  {
    return new MWMultiSpec(*this);
  }

  void MWMultiSpec::print (ostream& os, const string& indent) const
  {
    itsSpec.printSpec (os, indent, "MWMultiSpec");
    MWMultiStep::print (os, indent);
  }

}} // end namespaces
