//# ParmManager.h: Manages model parameters from multiple ParmDBs.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BBSKERNEL_PARMMANAGER_H
#define LOFAR_BBSKERNEL_PARMMANAGER_H

// \file
// Manages model parameters from multiple ParmDBs.

#include <BBSKernel/ParmProxy.h>

#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmSet.h>
#include <ParmDB/ParmCache.h>

#include <Common/lofar_set.h>
#include <Common/lofar_string.h>
#include <Common/lofar_smartptr.h>
#include <Common/LofarTypes.h>
#include <Common/Singleton.h>

namespace casa
{
    class Regex;
}

namespace LOFAR
{
namespace BBS
{
class Box;
class Grid;

// \addtogroup BBSKernel
// @{

typedef set<uint> ParmGroup;

class ParmManagerImpl
{
public:
    typedef shared_ptr<ParmManagerImpl>         Ptr;
    typedef shared_ptr<const ParmManagerImpl>   ConstPtr;

    void initCategory(uint category, const ParmDB &db);

    double getDefaultValue(uint category, const string &name,
        double value = 0.0);

    ParmProxy::Ptr get(uint category, const string &name);
    ParmProxy::Ptr get(uint category, const string &name, ParmGroup &group);

    ParmProxy::Ptr get(uint id)
    { return itsParms[id]; }
    ParmProxy::ConstPtr get(uint id) const
    { return itsParms[id]; }

    void setDomain(const Box &domain);

    void setGrid(const Grid &grid);
    void setGrid(const Grid &grid, const ParmGroup &group);

    void flush();

    ParmGroup makeSubset(const vector<string> &include,
        const vector<string> &exclude,
        const ParmGroup &group = ParmGroup()) const;

private:
    ParmManagerImpl();
    ParmManagerImpl(const ParmManagerImpl &other);
    ParmManagerImpl &operator=(const ParmManagerImpl &other);

    friend class Singleton<ParmManagerImpl>;

    bool isIncluded(const string &candidate, const vector<casa::Regex> &include,
        const vector<casa::Regex> &exclude) const;

    const ParmDB &getParmDbForCategory(uint category) const;
    ParmDB &getParmDbForCategory(uint category);

    ParmSet                         itsParmSet;
    ParmCache                       itsParmCache;
    map<uint, ParmDB>               itsCategories;
    map<string, pair<uint, uint> >  itsParmMap;
    vector<ParmProxy::Ptr>      itsParms;
};

typedef Singleton<ParmManagerImpl>  ParmManager;

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
