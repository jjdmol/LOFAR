//# Solver.h: 
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BB_BBSKERNEL_SOLVER_H
#define LOFAR_BB_BBSKERNEL_SOLVER_H

#include <BBSKernel/CoefficientIndex.h>
#include <BBSKernel/Messages.h>
#include <Common/LofarTypes.h>

#include <scimath/Fitting/LSQFit.h>

namespace LOFAR
{
namespace BBS
{
    class Solver
    {
    public:
        Solver()
        {}

        void setCoefficientIndex(CoeffIndexMsg::Pointer msg);
        CoeffIndexMsg::Pointer getCoefficientIndex() const;

        void setCoefficients(CoefficientMsg::Pointer msg);

        void setEquations(EquationMsg::Pointer msg);

        bool iterate(SolutionMsg::Pointer msg);

    private:
        struct Cell
        {
            casa::LSQFit    solver;
            vector<double>  coeff;
        };

        map<uint32, Cell>                           itsCells;

        map<string, uint32>                         itsParameters;

        CoefficientIndex                            itsCoefficientIndex;
        map<uint32, map<uint32, vector<uint32> > >  itsCoeffMapping;
  };

//  ostream &operator<<(ostream &out, const casa::LSQFit &eq);


} //# namespace BBS
} //# namespace LOFAR

#endif
