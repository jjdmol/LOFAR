//# Constant.cc: Real or complex constant
//#
//# Copyright (C) 2002
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

#include "Constant.h"
#include "Request.h"
#include "VellSet.h"
#include "Cells.h"
#include "MeqVocabulary.h"
#include <Common/Debug.h>
#include <Common/Lorrays.h>
#include <aips/Mathematics/Math.h>

namespace Meq {

Constant::Constant (double value)
: Function (),
  itsValue (value, false)
{}

Constant::Constant (const dcomplex& value)
: Function (),
  itsValue (value, false)
{}

Constant::~Constant()
{}

void Constant::init (DataRecord::Ref::Xfer& initrec, Forest* frst)
{
  // do parent init (this will call our setStateImpl())
  Node::init (initrec, frst);
}

int Constant::getResult (Result::Ref& resref,
			 const std::vector<Result::Ref>&,
			 const Request& request, bool)
{
  // Create result object and attach to the ref that was passed in
  Result& result = resref <<= new Result(1,request); // result has one vellset
  VellSet& vs = result.setNewVellSet(0);
  vs.setValue (itsValue);
  return 0;
}

void Constant::setStateImpl (DataRecord& rec, bool initializing)
{
  Function::setStateImpl (rec,initializing);
  // Get value.
  if (rec[FValue].exists()) {
    if (rec[FValue].type() == Tpdouble) {
      itsValue = Vells(rec[FValue].as<double>());
    } else {
      itsValue = Vells(rec[FValue].as<dcomplex>());
    }
    Assert (itsValue.isScalar());
  }
}

string Constant::sdebug (int detail, const string &prefix,const char* nm) const
{
  return Node::sdebug(detail,prefix,nm);
}

} // namespace Meq
