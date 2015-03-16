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

typedef set<unsigned int> ParmGroup;

class ParmManagerImpl
{
public:
    typedef shared_ptr<ParmManagerImpl>         Ptr;
    typedef shared_ptr<const ParmManagerImpl>   ConstPtr;

    void initCategory(unsigned int category, const ParmDB &db);

    vector<string> find(unsigned int category, const string &pattern) const;

    double getDefaultValue(unsigned int category, const string &name,
        double value = 0.0);

    ParmProxy::Ptr get(unsigned int category, const string &name);
    ParmProxy::Ptr get(unsigned int category, const string &name,
        ParmGroup &group);

    ParmProxy::Ptr get(unsigned int id);
    ParmProxy::ConstPtr get(unsigned int id) const;

    Box domain() const;
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

    const ParmDB &getParmDBForCategory(unsigned int category) const;
    ParmDB &getParmDBForCategory(unsigned int category);

    // TODO: Create domain() method on ParmCache instead of keeping a copy
    // of the domain as a member here.
    Box                                             itsDomain;
    ParmSet                                         itsParmSet;
    ParmCache                                       itsParmCache;
    map<unsigned int, ParmDB>                       itsCategories;
    map<string, pair<unsigned int, unsigned int> >  itsParmMap;
    vector<ParmProxy::Ptr>                          itsParms;
};

typedef Singleton<ParmManagerImpl>  ParmManager;

// @}

// -------------------------------------------------------------------------- //
// - ParmManagerImpl implementation                                         - //
// -------------------------------------------------------------------------- //

inline ParmProxy::Ptr ParmManagerImpl::get(unsigned int id)
{
    DBGASSERT(id < itsParms.size());
    return itsParms[id];
}

inline ParmProxy::ConstPtr ParmManagerImpl::get(unsigned int id) const
{
    DBGASSERT(id < itsParms.size());
    return itsParms[id];
}

} //# namespace BBS
} //# namespace LOFAR

#endif
