//# ExprValueView.cc: A view on an ExprValue instance of the unbound value or
//# the value bound to a specific (parameter, coefficient) pair.
//#
//# Copyright (C) 2009
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
#include <BBSKernel/Expr/ExprValueView.h>

namespace LOFAR
{
namespace BBS
{

ExprValueView<Scalar>::ExprValueView()
    :   itsBindMask(false)
{
}

ExprValueView<JonesMatrix>::ExprValueView()
{
    itsBindMask[0] = itsBindMask[1] = itsBindMask[2] = itsBindMask[3] = false;
}

} //# namespace BBS
} //# namespace LOFAR
