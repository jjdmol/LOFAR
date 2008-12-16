//# MWLocalSpec.cc: A local step in the MWSpec composite pattern
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
#include <MWControl/MWLocalSpec.h>
#include <MWCommon/MWStepFactory.h>

using namespace LOFAR;
using namespace std;

namespace LOFAR { namespace CEP {

  MWLocalSpec::MWLocalSpec()
  {}

  MWLocalSpec::MWLocalSpec(const string& name,
			   const ParameterSet& parset,
			   const MWSpec* parent)
    : itsSpec(name, parset, parent)
  {}

  MWLocalSpec::~MWLocalSpec()
  {}

  MWStep::ShPtr MWLocalSpec::create()
  {
    return MWStep::ShPtr (new MWLocalSpec());
  }

  void MWLocalSpec::registerCreate()
  {
    MWStepFactory::push_back ("MWLocalSpec", &create);
  }

  MWLocalSpec* MWLocalSpec::clone() const
  {
    return new MWLocalSpec(*this);
  }

  std::string MWLocalSpec::className() const
  {
    return "MWLocalSpec";
  }

  ParameterSet MWLocalSpec::getParms() const
  {
    return itsSpec.getFullParSet();
  }

  void MWLocalSpec::toBlob (LOFAR::BlobOStream& bos) const
  {
    bos.putStart (className(), 0);
    itsSpec.toBlob (bos);
    bos.putEnd();
  }
  void MWLocalSpec::fromBlob (LOFAR::BlobIStream& bis)
  {
    bis.getStart (className());
    itsSpec.fromBlob (bis);
    bis.getEnd();
  }

  void MWLocalSpec::print (ostream& os, const string& indent) const
  {
    itsSpec.printSpec (os, indent, "MWLocalSpec");
  }

  void MWLocalSpec::visit (MWStepVisitor& visitor) const
  {
    visitor.visitLocal (*this);
  }

}} // end namespaces
