//# ParmProxy.h: Wrapper class that stores information related to solving.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BB_BBS_PARMPROXY_H
#define LOFAR_BB_BBS_PARMPROXY_H

// \file
// Wrapper class that stores information related to solving.

#include <Common/lofar_smartptr.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h> // May be removed after ParmDB rework?
#include <Common/lofar_string.h> // May be removed after ParmDB rework?

#include <ParmDB/Grid.h>
#include <ParmDB/Parm.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// @{

class ParmProxy
{
public:
    typedef shared_ptr<ParmProxy>       Pointer;
    typedef shared_ptr<const ParmProxy> ConstPointer;

    ParmProxy(uint id, const string &name, const Parm &parm);
    ~ParmProxy();

    size_t getId() const
    { return itsId; }
    
    const string &getName() const
    { return itsName; }

    size_t getCoeffCount() const
    { return itsParm.getCoeffSize(); }
    
    vector<double> getCoeff(const Location &loc, bool useMask = true) const
    { return itsParm.getCoeff(loc, useMask); }

    void setCoeff(const Location &loc, const double* values, uint nvalues,
        bool useMask = true)
    { itsParm.setCoeff(loc, values, nvalues, 0, useMask); }

    void revertCoeff()
    { itsParm.revertCoeff(); }

    const vector<double> &getPerturbations() const
    { return itsParm.getPerturbations(); }
    
    double getPerturbation(uint index) const
    { return itsParm.getPerturbation(index); }
    
    void setGrid(const Grid &grid)
    { itsParm.setSolveGrid(grid); }

    void getResult(casa::Array<double> &result, const Grid &grid) const
    { itsParm.getResult(result, grid); }

    void getResult(vector<casa::Array<double> > &result, const Grid &grid,
        bool perturbed = false) const
    { itsParm.getResult(result, grid, perturbed); }

private:
    uint                itsId;
    string              itsName;
    mutable Parm        itsParm;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
