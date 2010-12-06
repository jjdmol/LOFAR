//# MWSpec.cc: Base component class of the MWSpec composite pattern
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
#include <MWControl/MWSpec.h>
#include <MWControl/MWGlobalSpec.h>
#include <MWControl/MWLocalSpec.h>
#include <MWControl/MWMultiSpec.h>
#include <MWCommon/ParameterHandler.h>
#include <MWCommon/MWError.h>
#include <Common/StreamUtil.h>
#include <ostream>

namespace std {
  using LOFAR::operator<<;
}
using namespace std;

namespace LOFAR { namespace CEP {


  MWSpec::MWSpec()
    : itsParent (0)
  {}

  MWSpec::MWSpec (const string& name, 
		  const ParameterSet& parset,
		  const MWSpec* parent)
    : itsName   (name),
      itsParent (parent)
  {
    itsParSet = parset.makeSubset("Step." + name + ".");
    // Merge it with the parset of the parent to inherit unspecified parms
    // from the parent.
    // First get a copy of the parameterset of the parent (if there).
    if (itsParent) {
      itsFullParSet = itsParent->copyFullParSet();
    }
    // Merge its own parset in.
    // Note that this function takes the parm from the second one if occurring
    // in both; that is why the parent parset was copied first.
    itsFullParSet.adoptCollection (itsParSet);
  }

  MWSpec::~MWSpec()
  {}

  string MWSpec::fullName() const
  {
    string name;
    if (itsParent) {
      name = itsParent->fullName() + ".";
    }
    name += itsName;
    return name;
  }

  MWStep::ShPtr MWSpec::createSpec (const string& name,
				    const ParameterSet& parset,
				    const MWSpec* parent)
  {
    MWStep* spec;
    // If \a parset contains a key <tt>Step.<em>name</em>.Steps</tt>, then
    // \a name is a MWMultiSpec, otherwise it is a SingleSpec.
    // If it contains a key <tt>Global</tt> with value True, a global step
    // needs to be executed which requires iteration between worker and master.
    if (parset.isDefined("Step."+name+".Steps")) {
      spec = new MWMultiSpec(name, parset, parent);
    } else {
      bool global = parset.getBool ("Step."+name+".Global", false);
      if (global) {
	spec = new MWGlobalSpec(name, parset, parent);
      } else {
	spec = new MWLocalSpec(name, parset, parent);
      }
    }
    return MWStep::ShPtr (spec);
  }

  void MWSpec::printSpec (ostream& os, const string& indent,
			  const string& type) const
  {
    os << indent << type << " specification: " << itsName;
    string indent2 = indent + " ";
    os << endl << indent2 << "Full name: " << fullName();
    os << endl << itsFullParSet;
  }

  void MWSpec::toBlob (BlobOStream& bos) const
  {
    bos << itsFullParSet;
  }

  void MWSpec::fromBlob (BlobIStream& bis)
  {
    bis >> itsFullParSet;
  }

  ParameterSet MWSpec::copyFullParSet() const
  {
    ParameterSet parset;
    for (ParameterSet::const_iterator iter=itsFullParSet.begin();
	 iter != itsFullParSet.end();
	 ++iter) {
      parset.add (iter->first, iter->second);
    }
    return parset;
  }
  
  void MWSpec::infiniteRecursionCheck (const string& name) const
  {
    if (name == itsName) {
      THROW (MWError, 
	     "Infinite recursion detected in definition of MWSpec \""
	     << name << "\". Please check your ParameterSet file.");
    }
    if (itsParent) {
      itsParent->infiniteRecursionCheck (name);
    }
  }

}} // end namespaces
