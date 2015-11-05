//# PointSource.cc: Class holding the expressions defining a point source.
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
#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/PointCoherence.h>
#include <BBSKernel/Expr/Scope.h>
#include <BBSKernel/Expr/SpectralIndex.h>
#include <BBSKernel/Expr/StokesRM.h>

#include <Common/lofar_sstream.h>
#include <ParmDB/SourceInfo.h>

namespace LOFAR
{
namespace BBS
{

PointSource::PointSource(const SourceInfo &source, Scope &scope)
    :   Source(source, scope)
{
    ASSERT(source.getType() == SourceInfo::POINT);

    // Stokes vector.
    const unsigned int nCoeff = source.getSpectralIndexNTerms();

    vector<Expr<Scalar>::Ptr> coeff;
    coeff.reserve(nCoeff);
    for(unsigned int i = 0; i < nCoeff; ++i)
    {
        ostringstream oss;
        oss << "SpectralIndex:" << i << ":" << name();
        coeff.push_back(scope(SKY, oss.str()));
    }

    const double refFreq = source.getSpectralIndexRefFreq();
    ExprParm::Ptr refStokes = scope(SKY, "I:" + name());
    Expr<Scalar>::Ptr stokesI = Expr<Scalar>::Ptr(new SpectralIndex(refFreq,
        refStokes, coeff.begin(), coeff.end()));
    ExprParm::Ptr stokesV = scope(SKY, "V:" + name());

    if(source.getUseRotationMeasure())
    {
        ExprParm::Ptr polFraction = scope(SKY, "PolarizedFraction:" + name());
        ExprParm::Ptr polAngle = scope(SKY, "PolarizationAngle:" + name());
        ExprParm::Ptr rm = scope(SKY, "RotationMeasure:" + name());

        itsStokesVector = StokesRM::Ptr(new StokesRM(stokesI, stokesV,
            polFraction, polAngle, rm));
    }
    else
    {
        ExprParm::Ptr stokesQ = scope(SKY, "Q:" + name());
        ExprParm::Ptr stokesU = scope(SKY, "U:" + name());

        AsExpr<Vector<4> >::Ptr stokes(new AsExpr<Vector<4> >());
        stokes->connect(0, stokesI);
        stokes->connect(1, stokesQ);
        stokes->connect(2, stokesU);
        stokes->connect(3, stokesV);

        itsStokesVector = stokes;
    }
}

Expr<JonesMatrix>::Ptr
PointSource::coherence(const baseline_t&, const Expr<Vector<3> >::ConstPtr&,
    const Expr<Vector<3> >::ConstPtr&) const
{
    if(!itsCoherence)
    {
	    itsCoherence =
            Expr<JonesMatrix>::Ptr(new PointCoherence(itsStokesVector));
    }

    return itsCoherence;
}

} // namespace BBS
} // namespace LOFAR
