//# ToComplex.cc: Make Complex value of 2 nodes
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

#include <MEQ/ToComplex.h>

#include <MEQ/Vells.h>

#include <MEQ/Vells.h>

using namespace Meq::VellsMath;

namespace Meq {    

//##ModelId=400E535502BF
ToComplex::ToComplex()
{}

//##ModelId=400E535502C0
ToComplex::~ToComplex()
{}

//##ModelId=400E535502C4
Vells ToComplex::evaluate (const Request&,const LoShape &,
			   const vector<Vells*>& values)
{
  return tocomplex(*(values[0]), *(values[1]));
}

} // namespace Meq
