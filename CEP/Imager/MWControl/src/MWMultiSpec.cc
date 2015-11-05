//# MWMultiSpec.cc: A multi step in the MWSpec composite pattern
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
