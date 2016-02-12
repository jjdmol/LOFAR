//# Source.cc: Class holding the expressions defining a  source
//#
//# Copyright (C) 2006
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
#include <BBSKernel/Expr/Source.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/GaussianSource.h>
#include <BBSKernel/Expr/ShapeletSource.h>
#include <BBSKernel/Expr/PointSource.h>
#include <BBSKernel/Expr/Scope.h>
#include <BBSKernel/Expr/SpectralIndex.h>
#include <BBSKernel/Expr/StokesRM.h>

#include <ParmDB/SourceInfo.h>

namespace LOFAR
{
namespace BBS
{

Source::Ptr Source::create(const SourceInfo &source, Scope &scope)
{
    switch(source.getType())
    {
    case SourceInfo::POINT:
        return Source::Ptr(new PointSource(source, scope));
    case SourceInfo::GAUSSIAN:
        return Source::Ptr(new GaussianSource(source, scope));
    case SourceInfo::SHAPELET:
        return Source::Ptr(new ShapeletSource(source, scope));
    default:
        THROW(BBSKernelException, "Unsupported source type: "
            << source.getType() << " for source: " << source.getName());
    }
}

Source::Source(const SourceInfo &source, Scope &scope)
    :   itsName(source.getName())
{
    // Position.
    ExprParm::Ptr ra = scope(SKY, "Ra:" + name());
    ExprParm::Ptr dec = scope(SKY, "Dec:" + name());

    AsExpr<Vector<2> >::Ptr position(new AsExpr<Vector<2> >());
    position->connect(0, ra);
    position->connect(1, dec);
    itsPosition = position;

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

Source::~Source()
{
}

const string &Source::name() const
{
    return itsName;
}

Expr<Vector<2> >::Ptr Source::position() const
{
    return itsPosition;
}

Expr<Vector<4> >::Ptr Source::stokes() const
{
    return itsStokesVector;
}

} // namespace BBS
} // namespace LOFAR
