//# ExprSet.h: Base class for a set of related expressions, such as the
//# model expressions for all the baselines of a telescope.
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

#ifndef LOFAR_BBSKERNEL_EXPRESSIONSET_H
#define LOFAR_BBSKERNEL_EXPRESSIONSET_H

// \file
// Base class for a set of related expressions, such as the model expressions
// for all the baselines of a telescope.

#include <BBSKernel/ParmManager.h>
#include <ParmDB/Box.h>
#include <Common/lofar_smartptr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

template <typename T_EXPR>
class ExprSet
{
public:
    typedef shared_ptr<ExprSet>         Ptr;
    typedef shared_ptr<const ExprSet>   ConstPtr;

    virtual ~ExprSet();

    // Return the number of expressions in the set.
    virtual unsigned int size() const = 0;

    // Return the domain on which the expressions in the set are defined.
    virtual Box domain() const = 0;

    virtual ParmGroup getParms() const = 0;
    virtual ParmGroup getSolvableParms() const = 0;
    virtual void setSolvableParms(const ParmGroup &solvables) = 0;
    virtual void clearSolvableParms() = 0;

    virtual void setEvalGrid(const Grid &grid) = 0;
    virtual const T_EXPR evaluate(unsigned int i) = 0;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: ExprSet                                          - //
// -------------------------------------------------------------------------- //

template <typename T_EXPR>
ExprSet<T_EXPR>::~ExprSet()
{
}

} //# namespace BBS
} //# namespace LOFAR

#endif
