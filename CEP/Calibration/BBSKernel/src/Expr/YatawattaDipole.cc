//# YatawattaDipole.cc: Dipole voltage beam based on external functions.
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

#include <BBSKernel/Expr/YatawattaDipole.h>
#include <BBSKernel/Expr/PValueIterator.h>

#include <Common/lofar_complex.h>

#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS 
{

YatawattaDipole::YatawattaDipole(const string &moduleTheta,
    const string &modulePhi, const Expr &azel, const Expr &orientation,
        double scaleFactor)
    :   itsThetaFunction(moduleTheta, "test"),
        itsPhiFunction(modulePhi, "test"),
        itsScaleFactor(scaleFactor)
{
    ASSERT(itsThetaFunction.getParameterCount() == 8);
    ASSERT(itsPhiFunction.getParameterCount() == 8);
    
    addChild(azel);
    addChild(orientation);
}

JonesResult YatawattaDipole::getJResult(const Request &request)
{
    // Evaluate children.
    ResultVec tmpAzel;
    Result tmpOrientation;
    
    const ResultVec &resAzel = getChild(0).getResultVecSynced(request, tmpAzel);
    const Result &resOrientation =
        getChild(1).getResultSynced(request, tmpOrientation);

    const Matrix &az = resAzel[0].getValue();
    const Matrix &el = resAzel[1].getValue();
    const Matrix &orientation = resOrientation.getValue();

    // Create result.
    JonesResult result;
    result.init();
    
    Result &resXX = result.result11();
    Result &resXY = result.result12();
    Result &resYX = result.result21();
    Result &resYY = result.result22();
    
    // Compute main value.
    evaluate(request, az, el, orientation, resXX.getValueRW(),
        resXY.getValueRW(), resYX.getValueRW(), resYY.getValueRW());

    // Compute the perturbed values.  
    enum PValues
    {
        PV_AZ, PV_EL, PV_ORIENTATION, N_PValues
    };
    
    const Result *pvSet[N_PValues] = {&(resAzel[0]), &(resAzel[1]),
        &resOrientation};
    PValueSetIterator<N_PValues> pvIter(pvSet);
    
    while(!pvIter.atEnd())
    {
        const Matrix &pvAz = pvIter.value(PV_AZ);
        const Matrix &pvEl = pvIter.value(PV_EL);
        const Matrix &pvOrientation = pvIter.value(PV_ORIENTATION);

        evaluate(request, pvAz, pvEl, pvOrientation,
            resXX.getPerturbedValueRW(pvIter.key()),
            resXY.getPerturbedValueRW(pvIter.key()),
            resYX.getPerturbedValueRW(pvIter.key()),
            resYY.getPerturbedValueRW(pvIter.key()));

        pvIter.next();
    }

    return result;
}

void YatawattaDipole::evaluate(const Request &request,
        const Matrix &in_az, const Matrix &in_el,
        const Matrix &in_orientation,
        Matrix &out_E11, Matrix &out_E12,
        Matrix &out_E21, Matrix &out_E22)
{        
    const size_t nChannels = request.getChannelCount();
    const size_t nTimeslots = request.getTimeslotCount();

    // Check preconditions.
    ASSERT(static_cast<size_t>(in_az.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_el.nelements()) == nTimeslots);
    ASSERT(static_cast<size_t>(in_orientation.nelements()) == 1);
    
    const double *az = in_az.doubleStorage();
    const double *el = in_el.doubleStorage();

    double *E11_re, *E11_im;
    out_E11.setDCMat(nChannels, nTimeslots);
    out_E11.dcomplexStorage(E11_re, E11_im);
    
    double *E12_re, *E12_im;
    out_E12.setDCMat(nChannels, nTimeslots);
    out_E12.dcomplexStorage(E12_re, E12_im);

    double *E21_re, *E21_im;
    out_E21.setDCMat(nChannels, nTimeslots);
    out_E21.dcomplexStorage(E21_re, E21_im);

    double *E22_re, *E22_im;
    out_E22.setDCMat(nChannels, nTimeslots);
    out_E22.dcomplexStorage(E22_re, E22_im);
    
    //  Parameters for external functions:
    //      0: time
    //          (NOTE: ignored)
    //      1: frequency
    //      2: az
    //      3: el
    //          (NOTE: incorrectly labelled zenith angle in implementation!)
    //      4: height
    //          (NOTE: ignored)
    //      5: projected arm length
    //          (NOTE: ignored)
    //      6: slant
    //          (NOTE: ignored)
    //      7: orientation (phi0)

    // Create parameter vectors for the X and Y dipole (used for calling
    // external functions).
    vector<dcomplex> xParms(8, makedcomplex(0.0, 0.0));
    vector<dcomplex> yParms(8, makedcomplex(0.0, 0.0));

    // TODO: Inside external function, these parameters are added to the
    // azimuth. The resulting azimuth is therefore:
    //
    // az = az + orientation (- pi / 2.0)
    //
    // Whereas it seems to me that the orientation should be subtracted
    // instead of added. It probably does not matter much, because the
    // beam pattern is symmetric with respect to azimuth.
    xParms[7] = makedcomplex(in_orientation.getDouble(0, 0), 0.0);
    yParms[7] =
        makedcomplex(in_orientation.getDouble(0, 0) - casa::C::pi_2, 0.0);
        
    // Evaluate beam.
    uint sample = 0;
    Axis::ShPtr freqAxis(request.getGrid()[FREQ]);
    for(size_t t = 0; t < nTimeslots; ++t)
    {
        // TODO: Where does the -pi/4 term in azimuth come from (see
        // global_model.py in EJones_HBA)? Is this just the default dipole
        // orientation? If so, the term should be removed in favor of setting
        // a correct dipole orientation in the parameter database (such that
        // the orientation in the parameter database corresponds 1:1 with the
        // real orientation).
//        xParms[2] = yParms[2] = makedcomplex(az[t] - casa::C::pi_4, 0.0);
        xParms[2] = yParms[2] = makedcomplex(az[t], 0.0);
        xParms[3] = yParms[3] = makedcomplex(el[t], 0.0);
                    
        for(size_t f = 0; f < nChannels; ++f)
        {
            // Update frequency.
            xParms[1] = yParms[1] = makedcomplex(freqAxis->center(f), 0.0);

            // Compute dipole beam value.
            const dcomplex xTheta = itsThetaFunction(xParms) * itsScaleFactor;
            const dcomplex xPhi = itsPhiFunction(xParms) * itsScaleFactor;
            E11_re[sample] = real(xTheta);
            E11_im[sample] = imag(xTheta);
            E12_re[sample] = real(xPhi);
            E12_im[sample] = imag(xPhi);

            const dcomplex yTheta = itsThetaFunction(yParms) * itsScaleFactor;
            const dcomplex yPhi = itsPhiFunction(yParms) * itsScaleFactor;
            E21_re[sample] = real(yTheta);
            E21_im[sample] = imag(yTheta);
            E22_re[sample] = real(yPhi);
            E22_im[sample] = imag(yPhi);
            
            // Move to next sample.
            ++sample;
        }
    }
}


} //# namespace BBS
} //# namespace LOFAR
