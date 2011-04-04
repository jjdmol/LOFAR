//# ExprSet.h: Base class for a set of related expressions, such as the model
//# expressions for all the baselines of a telescope.
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

#ifndef LOFAR_BBSKERNEL_EXPRSET_H
#define LOFAR_BBSKERNEL_EXPRSET_H

// \file
// Base class for a set of related expressions, such as the model expressions
// for all the baselines of a telescope.

#include <Common/lofar_smartptr.h>
#include <ParmDB/Box.h>
#include <BBSKernel/ParmManager.h>

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

    // Get the set of parameters this set of expressions depends on.
    virtual ParmGroup parms() const = 0;

    // Get the set of parameters for which partial derivatives will be computed.
    virtual ParmGroup solvables() const = 0;

    // Set the parameters for which partial derivatives will be computed.
    // Parameters that the expressions in this set do not depend on are silently
    // ingnored. Use parms() to find out beforehand which parameters the
    // expressions in this set depend on, or use solvables() after calling this
    // method to get the set of parameters for which partial derivatives will be
    // computed.
    virtual void setSolvables(const ParmGroup &solvables) = 0;

    // Clear the set of parameters for which partial derivatives will be
    // computed, i.e. do not compute any partial derivatives.
    virtual void clearSolvables() = 0;

    // Notify ExprSet of external updates to the value of the solvables.
    virtual void solvablesChanged() = 0;

    // Set the grid on which the expressions in the set will be evaluated.
    virtual void setEvalGrid(const Grid &grid) = 0;

    // Evaluate the expression with index i on the evaluation grid and return
    // the result.
    virtual const T_EXPR evaluate(unsigned int i) = 0;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: ExprSet                                                - //
// -------------------------------------------------------------------------- //

template <typename T_EXPR>
ExprSet<T_EXPR>::~ExprSet()
{
}

} //# namespace BBS
} //# namespace LOFAR

#endif
