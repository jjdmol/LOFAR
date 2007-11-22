//# MeqParmExpr.h: A parm expression
//#
//# Copyright (C) 2006
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

#include <BBSKernel/MNS/MeqParmExpr.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>

namespace LOFAR
{
namespace BBS
{

  MeqParmExpr::MeqParmExpr (const string& expr, MeqParmGroup& group,
			    LOFAR::ParmDB::ParmDB* table)
  {
    // Only the multiplication or subtraction of 2 parms is supported.
    string::size_type idx = expr.find('-');
    itsType = 1;
    if (idx == string::npos) {
      idx = expr.find('*');
      itsType = 2;
      ASSERTSTR (idx != string::npos,
		 "Currently parmexpr only supports subtraction or "
		 "multiplication of two parms");
    }
    string p1, p2;
    for (string::size_type i=0; i<idx; ++i) {
      if (expr[i] != ' ') {
	p1.push_back (expr[i]);
      }
    }
    for (string::size_type i=idx+1; i<expr.size(); ++i) {
      if (expr[i] != ' ') {
	p2.push_back (expr[i]);
      }
    }
    ASSERTSTR (!p1.empty()  &&  !p2.empty(),
	       "Currently parmexpr only supports multiplication of two parms");
    itsExpr1 = MeqParmFunklet::create (p1, group, table);
    itsExpr2 = MeqParmFunklet::create (p2, group, table);
    addChild (itsExpr1);
    addChild (itsExpr2);
  }

  MeqParmExpr::~MeqParmExpr()
  {}

  MeqMatrix MeqParmExpr::getResultValue (const vector<const MeqMatrix*>& v)
  {
    if (itsType == 1) {
      return *v[0] - *v[1];
    }
    return *v[0] * *v[1];
  }

} // namespace BBS
} // namespace LOFAR
