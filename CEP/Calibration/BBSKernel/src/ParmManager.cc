//# ParmManager.cc: Manages model parameters from multiple ParmDBs.
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

#include <lofar_config.h>

#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Exceptions.h>
#include <ParmDB/Grid.h>

#include <casa/Utilities/Regex.h>
#include <casa/Exceptions/Error.h>


namespace LOFAR
{
namespace BBS
{

ParmManagerImpl::ParmManagerImpl()
    : itsParmCache(itsParmSet)
{
}

void ParmManagerImpl::initCategory(uint category, const ParmDB &db)
{
    // It is assumed that constructing multiple ParmDB objects for the same
    // ParmDB on disk is supported by the ParmDB implementation.
    ASSERTSTR(itsCategories.find(category) == itsCategories.end(),
        "Category already bound to a ParmDB.");
    itsCategories.insert(make_pair(category, db));
}

double ParmManagerImpl::getDefaultValue(uint category, const string &name,
    double value)
{
    ParmDB &parmDb = getParmDbForCategory(category);

    ParmValueSet valueSet = parmDb.getDefValue(name, ParmValue(value));
    ASSERT(valueSet.empty() && valueSet.getType() == ParmValue::Scalar);
    const casa::Array<double> &values =
        valueSet.getFirstParmValue().getValues();
    ASSERT(values.size() == 1);
    return values(casa::IPosition(values.ndim(), 0));
}

ParmProxy::Pointer ParmManagerImpl::get(uint category, const string &name)
{
    ParmDB &parmDb = getParmDbForCategory(category);

    pair<map<string, pair<uint, uint> >::const_iterator, bool> status =
        itsParmMap.insert(make_pair(name,
            make_pair(category, itsParms.size())));

    const uint parmCat = status.first->second.first;
    const uint parmId = status.first->second.second;

    // Verify that the parameter belong to the requested category.
    ASSERTSTR(parmCat == category, "Category mismatch for parameter " << name);

    if(status.second)
    {
        // This is the first reference to this parameter.
//        cout << "Fetching parameter from db: " << name << " [" << parmId << "]"
//            << endl;

        const ParmId internalId = itsParmSet.addParm(parmDb, name);
        Parm parm(itsParmCache, internalId);
        itsParms.push_back(ParmProxy::Pointer(new ParmProxy(parmId, name,
            parm)));
        itsParmCache.cacheValues();
    }

    return itsParms[parmId];
}

ParmProxy::Pointer ParmManagerImpl::get(uint category, const string &name,
    ParmGroup &group)
{
    ParmProxy::Pointer proxy(get(category, name));
    group.insert(proxy->getId());
    return proxy;
}

void ParmManagerImpl::setDomain(const Box &domain)
{
    itsParmCache.reset(domain);
}

void ParmManagerImpl::setGrid(const Grid &grid)
{
    vector<ParmProxy::Pointer>::const_iterator it = itsParms.begin();
    while(it != itsParms.end())
    {
        (*it)->setGrid(grid);
        ++it;
    }
}

void ParmManagerImpl::setGrid(const Grid &grid, const ParmGroup &group)
{
    ParmGroup::const_iterator it = group.begin();
    while(it != group.end())
    {
        itsParms[*it]->setGrid(grid);
        ++it;
    }
}

void ParmManagerImpl::flush()
{
    itsParmCache.flush();
}

ParmGroup ParmManagerImpl::makeSubset(const vector<string> &include,
    const vector<string> &exclude, const ParmGroup &group) const
{
    vector<casa::Regex> includeRegex(include.size());
    vector<casa::Regex> excludeRegex(exclude.size());

    try
    {
        transform(include.begin(), include.end(), includeRegex.begin(),
            ptr_fun(casa::Regex::fromPattern));

        transform(exclude.begin(), exclude.end(), excludeRegex.begin(),
            ptr_fun(casa::Regex::fromPattern));
    }
    catch(casa::AipsError &ex)
    {
        THROW(BBSKernelException, "Error parsing include/exclude pattern"
            " (exception: " << ex.what() << ")");
    }

    ParmGroup result;
    if(group.empty())
    {
        for(size_t i = 0; i < itsParms.size(); ++i)
        {
            if(isIncluded(itsParms[i]->getName(), includeRegex, excludeRegex))
            {
                result.insert(itsParms[i]->getId());
            }
        }
    }
    else
    {
        ParmGroup::const_iterator it = group.begin();
        while(it != group.end())
        {
            if(isIncluded(itsParms[*it]->getName(), includeRegex, excludeRegex))
            {
                result.insert(itsParms[*it]->getId());
            }
            ++it;
        }
    }

    return result;
}

bool ParmManagerImpl::isIncluded(const string &candidate,
    const vector<casa::Regex> &include, const vector<casa::Regex> &exclude)
    const
{
    casa::String name(candidate);

    bool flag = false;
    vector<casa::Regex>::const_iterator inc_it = include.begin();
    while(inc_it != include.end())
    {
        if(name.matches(*inc_it))
        {
            flag = true;
            break;
        }
        ++inc_it;
    }

    if(flag)
    {
        vector<casa::Regex>::const_iterator exc_it = exclude.begin();
        while(exc_it != exclude.end())
        {
            if(name.matches(*exc_it))
            {
                flag = false;
                break;
            }
            ++exc_it;
        }
    }

    return flag;
}

const ParmDB &ParmManagerImpl::getParmDbForCategory(uint category) const
{
    map<uint, ParmDB>::const_iterator catIt = itsCategories.find(category);
    ASSERTSTR(catIt != itsCategories.end(), "No ParmDB instance bound to"
        " category: " << category);
    return catIt->second;
}

ParmDB &ParmManagerImpl::getParmDbForCategory(uint category)
{
    map<uint, ParmDB>::iterator catIt = itsCategories.find(category);
    ASSERTSTR(catIt != itsCategories.end(), "No ParmDB instance bound to"
        " category: " << category);
    return catIt->second;
}

} //# namespace BBS
} //# namespace LOFAR
