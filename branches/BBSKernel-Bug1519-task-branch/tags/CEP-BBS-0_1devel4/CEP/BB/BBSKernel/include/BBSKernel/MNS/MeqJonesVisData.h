//# MeqJonesVisData.h: 
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

#ifndef MNS_MEQJONESVISDATA_H
#define MNS_MEQJONESVISDATA_H

#include <BBSKernel/MNS/MeqJonesExpr.h>
#include <BBSKernel/VisData.h>

namespace LOFAR
{
namespace BBS
{

class MeqJonesVisData: public MeqJonesExprRep
{
public:
    MeqJonesVisData(VisData::Pointer vdata, baseline_t baseline);

    ~MeqJonesVisData()
    {}

    // Get the result of the expression for the given domain.
    virtual MeqJonesResult getJResult (const MeqRequest&);

private:
    void copy(double *re, double *im,
        const boost::multi_array<sample_t, 4>::const_array_view<2>::type &src);

    VisData::Pointer    itsVisData;
    size_t              itsBaselineIndex;
};

} // namespace BBS
} // namespace LOFAR

#endif
