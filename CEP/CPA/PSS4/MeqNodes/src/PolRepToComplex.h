//# PolRepToComplex.h: Make Complex value of 2 nodes
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

#ifndef MEQ_POLREPTOCOMPLEX_H
#define MEQ_POLREPTOCOMPLEX_H
    
#include <MEQ/Function.h>

#include <MeqNodes/TID-MeqNodes.h>
#pragma aidgroup MeqNodes
#pragma types #Meq::PolRepToComplex

namespace Meq {    


//##ModelId=400E530400B3
class PolRepToComplex : public Function2
{
public:
    //##ModelId=400E535502BF
  PolRepToComplex();

    //##ModelId=400E535502C0
  virtual ~PolRepToComplex();

    //##ModelId=400E535502C2
    virtual TypeId objectType() const
    { return TpMeqPolRepToComplex; }

  // Evaluate the value for the given request.
    //##ModelId=400E535502C4
  virtual Vells evaluate (const Request&,const LoShape &,
			  const vector<const Vells*>& values);
};


} // namespace Meq

#endif
