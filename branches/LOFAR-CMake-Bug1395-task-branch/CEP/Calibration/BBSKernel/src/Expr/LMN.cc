//# LMN.cc: LMN-coordinates of a direction on the sky.
//#
//# Copyright (C) 2005
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

#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/Source.h>
#include <BBSKernel/Expr/PhaseRef.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/PValueIterator.h>
#include <Common/LofarLogger.h>


namespace LOFAR
{
namespace BBS
{

LMN::LMN(const Source::Pointer &source,
    const PhaseRef::ConstPointer &phaseRef)
    :   itsSource(source),
        itsPhaseRef(phaseRef)
{
    addChild(itsSource->getRa());
    addChild(itsSource->getDec());
}

LMN::~LMN()
{
}

ResultVec LMN::getResultVec(const Request &request)
{
    ResultVec result(3);
    Result &resL = result[0];
    Result &resM = result[1];
    Result &resN = result[2];
    Result raRes, deRes;

    const Result &rak = getChild(0).getResultSynced(request, raRes);
    const Result &deck = getChild(1).getResultSynced(request, deRes);
    const double refRa = itsPhaseRef->getRa();
    const double refDec = itsPhaseRef->getDec();
    const double refSinDec = itsPhaseRef->getSinDec();
    const double refCosDec = itsPhaseRef->getCosDec();

    Matrix cosdec = cos(deck.getValue());
    Matrix radiff = rak.getValue() - refRa;
    Matrix lk = cosdec * sin(radiff);
    Matrix mk = sin(deck.getValue()) * refCosDec - cosdec * refSinDec
        * cos(radiff);
    MatrixTmp nks = 1. - sqr(lk) - sqr(mk);

    // Although an N-coordinate of 0.0 is valid as long as the length of the
    // LMN vector equals 1.0, it will cause problems when dividing by it.
    // However, this should be checked where the division takes place.
    ASSERTSTR(min(nks).getDouble() >= 0.0, "Source " << itsSource->getName()
        << " too far from phase reference " << refRa << ", " << refDec);

    Matrix nk = sqrt(nks);
    resL.setValue(lk);
    resM.setValue(mk);
    resN.setValue(nk);

//    cout << itsSource->getName() << " L: " << lk.getDouble(0, 0) << " M: "
//        << mk.getDouble(0, 0) << " N: " << nk.getDouble(0, 0) << endl;

    // Compute perturbed values.
    const Result *pvSet[2] = {&rak, &deck};
    PValueSetIterator<2> pvIter(pvSet);

    while(!pvIter.atEnd())
    {
        const Matrix &pvRa = pvIter.value(0);
        const Matrix &pvDec = pvIter.value(1);
        Matrix pradiff = pvRa - refRa;
        Matrix pcosdec = cos(pvDec);
        Matrix plk = pcosdec * sin(pradiff);
        Matrix pmk = sin(pvDec) * refCosDec - pcosdec * refSinDec * cos(pradiff);
        MatrixTmp nks = MatrixTmp(1.) - sqr(plk) - sqr(pmk);
        ASSERTSTR(min(nks).getDouble() >= 0.0, "Perturbed source "
            << itsSource->getName() << " too far from phase reference "
            << refRa << ", " << refDec);
        Matrix pnk = sqrt(nks);

        resL.setPerturbedValue(pvIter.key(), plk);
        resM.setPerturbedValue(pvIter.key(), pmk);
        resN.setPerturbedValue(pvIter.key(), pnk);
        
        pvIter.next();
    }

    return result;
}


} // namespace BBS
} // namespace LOFAR
