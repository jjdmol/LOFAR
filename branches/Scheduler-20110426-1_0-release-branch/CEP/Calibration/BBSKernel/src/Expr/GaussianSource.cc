//# GaussianSource.cc: Class holding the expressions defining a gauss source
//#
//# Copyright (C) 2002
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
#include <BBSKernel/Expr/GaussianSource.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/SpectralIndex.h>
#include <BBSKernel/Expr/GaussianCoherence.h>
#include <BBSKernel/Expr/Scope.h>

#include <Common/lofar_sstream.h>
#include <ParmDB/SourceInfo.h>

namespace LOFAR
{
namespace BBS
{

GaussianSource::GaussianSource(const SourceInfo &source, Scope &scope)
    :   Source(source, scope)
{
    ASSERT(source.getType() == SourceInfo::GAUSSIAN);

    unsigned int degree =
        static_cast<unsigned int>(ParmManager::instance().getDefaultValue(SKY,
            "SpectralIndexDegree:" + name()));

    // Reference frequency.
    ExprParm::Ptr refFreq = scope(SKY, "ReferenceFrequency:" + name());

    // Stokes parameter value at the reference frequency.
    ExprParm::Ptr refStokes = scope(SKY, "I:" + name());

    vector<Expr<Scalar>::Ptr> coeff;
    coeff.reserve(degree + 1);
    for(unsigned int i = 0; i <= degree; ++i)
    {
        ostringstream oss;
        oss << "SpectralIndex:" << i << ":" << name();
        coeff.push_back(scope(SKY, oss.str()));
    }

    Expr<Scalar>::Ptr stokesI = Expr<Scalar>::Ptr(new SpectralIndex(refFreq,
        refStokes, coeff.begin(), coeff.end()));

    ExprParm::Ptr stokesQ = scope(SKY, "Q:" + name());
    ExprParm::Ptr stokesU = scope(SKY, "U:" + name());
    ExprParm::Ptr stokesV = scope(SKY, "V:" + name());

    AsExpr<Vector<4> >::Ptr stokes(new AsExpr<Vector<4> >());
    stokes->connect(0, stokesI);
    stokes->connect(1, stokesQ);
    stokes->connect(2, stokesU);
    stokes->connect(3, stokesV);
    itsStokesVector = stokes;

    ExprParm::Ptr major = scope(SKY, "MajorAxis:" + name());
    ExprParm::Ptr minor = scope(SKY, "MinorAxis:" + name());

    AsExpr<Vector<2> >::Ptr dimensions(new AsExpr<Vector<2> >());
    dimensions->connect(0, major);
    dimensions->connect(1, minor);
    itsDimensions = dimensions;

    itsOrientation = Expr<Scalar>::Ptr(scope(SKY, "Orientation:" + name()));
}

Expr<JonesMatrix>::Ptr
GaussianSource::coherence(const Expr<Vector<3> >::ConstPtr &uvwLHS,
    const Expr<Vector<3> >::ConstPtr &uvwRHS) const
{
    return Expr<JonesMatrix>::Ptr(new GaussianCoherence(itsStokesVector,
        itsDimensions, itsOrientation, uvwLHS, uvwRHS));
}

} // namespace BBS
} // namespace LOFAR
