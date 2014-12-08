//# ExprId.h: Unique ExprBase instance identifier. Placed in its own include
//# file to resolve an include conflict between Expr.h and Cache.h
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXPRID_H
#define LOFAR_BBSKERNEL_EXPR_EXPRID_H

// \file
// Unique ExprBase instance identifier. Placed in its own include file to
// resolve an include conflict between Expr.h and Cache.h

#include <Common/LofarTypes.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// Unique ExprBase instance identifier.
typedef size_t ExprId;

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
