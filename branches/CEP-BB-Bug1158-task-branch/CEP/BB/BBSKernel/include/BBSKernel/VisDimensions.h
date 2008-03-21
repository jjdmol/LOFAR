//# VisDimensions.h: 
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

#ifndef LOFAR_BBS_BBSKERNEL_VISDIMENSIONS_H
#define LOFAR_BBS_BBSKERNEL_VISDIMENSIONS_H

#include <BBSKernel/Axis.h>
#include <BBSKernel/Grid.h>
#include <BBSKernel/Types.h>

#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

class VisDimensions
{
public:
    VisDimensions()
    {}
    
    void setGrid(const Grid<double> &grid);
    void setBaselines(const vector<baseline_t> &baselines);
    void setPolarizations(const vector<string> &polarizations);
    
    const Grid<double> &getGrid() const
    { return itsGrid; }
    const vector<baseline_t> &getBaselines() const
    { return itsBaselines; }
    const vector<string> &getPolarizations() const
    { return itsPolarizations; }

    size_t getChannelCount() const
    { return itsGrid[FREQ]->size(); }
    size_t getTimeSlotCount() const
    { return itsGrid[TIME]->size(); }
    size_t getBaselineCount() const
    { return itsBaselines.size(); }
    size_t getPolarizationCount() const
    { return itsPolarizations.size(); }

    bool hasBaseline(baseline_t baseline) const;
    size_t getBaselineIndex(baseline_t baseline) const;

    bool hasPolarization(const string &polarization) const;
    size_t getPolarizationIndex(const string &polarization) const;

private:
    Grid<double>            itsGrid;
    vector<baseline_t>      itsBaselines;
    vector<string>          itsPolarizations;

    // Indexes.
    map<baseline_t, size_t> itsBaselineIndex;
    map<string, size_t>     itsPolarizationIndex;
};

} //# namespace BBS
} //# namespace LOFAR

#endif

