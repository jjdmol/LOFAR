//# MWGlobalSpec.cc: A global step in the MWSpec composite pattern
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
#include <MWControl/MWGlobalSpec.h>
#include <MWCommon/MWStepFactory.h>

using namespace LOFAR;
using namespace std;

namespace LOFAR { namespace CEP {

  MWGlobalSpec::MWGlobalSpec()
  {}

  MWGlobalSpec::MWGlobalSpec(const string& name,
			     const ParameterSet& parset,
			     const MWSpec* parent)
    : itsSpec(name, parset, parent)
  {}

  MWGlobalSpec::~MWGlobalSpec()
  {}

  MWStep::ShPtr MWGlobalSpec::create()
  {
    return MWStep::ShPtr (new MWGlobalSpec());
  }

  void MWGlobalSpec::registerCreate()
  {
    MWStepFactory::push_back ("MWGlobalSpec", &create);
  }

  MWGlobalSpec* MWGlobalSpec::clone() const
  {
    return new MWGlobalSpec(*this);
  }

  std::string MWGlobalSpec::className() const
  {
    return "MWGlobalSpec";
  }

  ParameterSet MWGlobalSpec::getParms() const
  {
    return itsSpec.getFullParSet();
  }

  void MWGlobalSpec::toBlob (LOFAR::BlobOStream& bos) const
  {
    bos.putStart (className(), 0);
    itsSpec.toBlob (bos);
    bos.putEnd();
  }
  void MWGlobalSpec::fromBlob (LOFAR::BlobIStream& bis)
  {
    bis.getStart (className());
    itsSpec.fromBlob (bis);
    bis.getEnd();
  }

  void MWGlobalSpec::print (ostream& os, const string& indent) const
  {
    itsSpec.printSpec (os, indent, "MWGlobalSpec");
  }

  void MWGlobalSpec::visit (MWStepVisitor& visitor) const
  {
    visitor.visitGlobal (*this);
  }

}} // end namespaces
