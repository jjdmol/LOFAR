//# Cos.cc: Take cosine of a node
//#
//# Copyright (C) 2003
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

#include <MEQ/Cos.h>
#include <MEQ/VellsTmp.h>

namespace Meq {    

Cos::Cos()
{}

Cos::~Cos()
{}

Vells Cos::evaluate (const Request&,
		     const vector<Vells*>& values)
{
  return cos(*(values[0]));
}

///void Cos::evaluateVells (Vells& result, const Request&,
///			 const vector<Vells*>& values)
///  if (result.isReal()) {
///    result.getRealArray() = cos(values[0]->getRealArray());
///  } else {
///    result.getComplexArray() = cos(values[0]->getComplexArray());
///  }

} // namespace Meq
