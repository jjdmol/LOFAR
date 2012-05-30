//# SourceDBUtil.cc: Helper functions to extract patch and source information
//# from a SourceDB.
//#
//# Copyright (C) 2012
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

#include <lofar_config.h>
#include <DPPP/SourceDBUtil.h>
#include <ParmDB/SourceDB.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace DPPP
{
using BBS::SourceDB;
using BBS::ParmDB;
using BBS::ParmValue;
using BBS::ParmValueSet;
using BBS::SourceInfo;

namespace
{
// Retrieve the value of the parameter \p name from the default value table of
// \p parmDB. If no default value can be found, \p value will be returned.
double getDefaultParmValue(const ParmDB &parmDB, const string &name,
    const double value = 0.0);
} // Unnamed namespace.

Patch makePatch(SourceDB &sourceDB, const string &name)
{
    ParmDB &parmDB = sourceDB.getParmDB();
    vector<SourceInfo> sources = sourceDB.getPatchSources(name);
    LOG_DEBUG_STR("found: " << sources.size() << " sources for patch: " << name);

    vector<PointSource> components;
    components.reserve(sources.size());
    for(vector<SourceInfo>::const_iterator it = sources.begin(),
        end = sources.end(); it != end; ++it)
    {
        ASSERTSTR(it->getType() == SourceInfo::POINT, "Only point sources are"
            " supported at the moment.");

        PointSource component;

        try
        {
            // Fetch position.
            Position position;
            position[0] = getDefaultParmValue(parmDB, "Ra:" + it->getName());
            position[1] = getDefaultParmValue(parmDB, "Dec:" + it->getName());
            component.setPosition(position);

            // Fetch stokes vector.
            Stokes stokes;
            stokes.I = getDefaultParmValue(parmDB, "I:" + it->getName());
            stokes.V = getDefaultParmValue(parmDB, "V:" + it->getName());
            if(!it->getUseRotationMeasure())
            {
                stokes.Q = getDefaultParmValue(parmDB, "Q:" + it->getName());
                stokes.U = getDefaultParmValue(parmDB, "U:" + it->getName());
            }
            component.setStokes(stokes);

            // Fetch spectral index attributes (if applicable).
            size_t nTerms = it->getSpectralIndexNTerms();
            if(nTerms > 0)
            {
                vector<double> terms;
                terms.reserve(nTerms);

                ostringstream oss;
                for(size_t i = 0; i < nTerms; ++i)
                {
                    oss << "SpectralIndex:" << i << ":" << it->getName();
                    terms.push_back(getDefaultParmValue(parmDB, oss.str()));
                }

                component.setSpectralIndex(it->getSpectralIndexRefFreq(),
                    terms.begin(), terms.end());
            }

            // Fetch rotation measure attributes (if applicable).
            if(it->getUseRotationMeasure())
            {
                component.setPolarizedFraction(getDefaultParmValue(parmDB,
                    "PolarizedFraction:" + it->getName()));
                component.setPolarizationAngle(getDefaultParmValue(parmDB,
                    "PolarizationAngle:" + it->getName()));
                component.setRotationMeasure(getDefaultParmValue(parmDB,
                    "RotationMeasure:" + it->getName()));
            }
        }
        catch(Exception &e)
        {
            continue;
        }

        components.push_back(component);
    }

    return Patch(name, components.begin(), components.end());
}

namespace
{
double getDefaultParmValue(const ParmDB &parmDB, const string &name,
    const double value)
{
    ParmValueSet valueSet = parmDB.getDefValue(name, ParmValue(value));
    ASSERT(valueSet.empty() && valueSet.getType() == ParmValue::Scalar);
    const casa::Array<double> &values = valueSet.getDefParmValue().getValues();
    ASSERT(values.size() == 1);
    return values(casa::IPosition(values.ndim(), 0));
}
} // Unnamed namespace.

} //# namespace DPPP
} //# namespace LOFAR
