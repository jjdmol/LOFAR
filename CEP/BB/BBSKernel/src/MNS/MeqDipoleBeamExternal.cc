//# MeqDipoleBeamExternal.cc: Dipole voltage beam based on external functions.
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

#include <BBSKernel/MNS/MeqDipoleBeamExternal.h>
#include <Common/lofar_complex.h>

#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS 
{

MeqDipoleBeamExternal::MeqDipoleBeamExternal(const string &moduleTheta,
    const string &modulePhi, MeqExpr azel, MeqExpr orientation,
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


MeqJonesResult MeqDipoleBeamExternal::getJResult(const MeqRequest &request)
{
    // Evaluate children.
    MeqResultVec res_azel;
    MeqResult res_orientation;
    
    const MeqResultVec &azel = getChild(IN_AZEL).getResultVecSynced(request,
        res_azel);

    const MeqResult &orientation =
        getChild(IN_ORIENTATION).getResultSynced(request, res_orientation);

    // Create result.
    MeqJonesResult result(request.nspid());
    MeqResult &result11 = result.result11();
    MeqResult &result12 = result.result12();
    MeqResult &result21 = result.result21();
    MeqResult &result22 = result.result22();
    
    // Evaluate main value.
    evaluate(request,
        azel[0].getValue(), azel[1].getValue(),
        orientation.getValue(),
        result11.getValueRW(), result12.getValueRW(),
        result21.getValueRW(), result22.getValueRW());

    // Evaluate perturbed values.  
    const MeqParmFunklet *perturbedParm;
    for(int i = 0; i < request.nspid(); ++i)
    {
        // Find out if this perturbed value needs to be computed.
    	if(azel[0].isDefined(i))
    	{
    	    perturbedParm = azel[0].getPerturbedParm(i);
    	}
    	else if(azel[1].isDefined(i))
    	{
    	    perturbedParm = azel[1].getPerturbedParm(i);
    	}
    	else if(orientation.isDefined(i))
    	{
    	    perturbedParm = orientation.getPerturbedParm(i);
    	}
    	else
    	{
    	    continue;
    	}

        evaluate(request,
            azel[0].getPerturbedValue(i), azel[1].getPerturbedValue(i),
            orientation.getPerturbedValue(i),
            result11.getPerturbedValueRW(i), result12.getPerturbedValueRW(i),
            result21.getPerturbedValueRW(i), result22.getPerturbedValueRW(i));

        result11.setPerturbedParm(i, perturbedParm);
        result12.setPerturbedParm(i, perturbedParm);
        result21.setPerturbedParm(i, perturbedParm);
        result22.setPerturbedParm(i, perturbedParm);
    }
    
    return result;
}


void MeqDipoleBeamExternal::evaluate(const MeqRequest &request,
        const MeqMatrix &in_az, const MeqMatrix &in_el,
        const MeqMatrix &in_orientation,
        MeqMatrix &out_E11, MeqMatrix &out_E12,
        MeqMatrix &out_E21, MeqMatrix &out_E22)
{        
    // Check preconditions.
    ASSERT(in_az.nelements() == request.ny());
    ASSERT(in_el.nelements() == request.ny());
    ASSERT(in_orientation.nelements() == 1);
    
    const double *az = in_az.doubleStorage();
    const double *el = in_el.doubleStorage();

    double *E11_re, *E11_im;
    out_E11.setDCMat(request.nx(), request.ny());
    out_E11.dcomplexStorage(E11_re, E11_im);
    
    double *E12_re, *E12_im;
    out_E12.setDCMat(request.nx(), request.ny());
    out_E12.dcomplexStorage(E12_re, E12_im);

    double *E21_re, *E21_im;
    out_E21.setDCMat(request.nx(), request.ny());
    out_E21.dcomplexStorage(E21_re, E21_im);

    double *E22_re, *E22_im;
    out_E22.setDCMat(request.nx(), request.ny());
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
    for(int t = 0; t < request.ny(); ++t)
    {
        const double freq = request.domain().startX() + request.stepX() / 2.0;
        xParms[1] = makedcomplex(freq, 0.0);
        yParms[1] = makedcomplex(freq, 0.0);

        // TODO: Where does the -pi/4 term in azimuth come from (see
        // global_model.py in EJones_HBA)? Is this just the default dipole
        // orientation? If so, the term should be removed in favor of setting
        // a correct dipole orientation in the parameter database (such that
        // the orientation in the parameter database corresponds 1:1 with the
        // real orientation).
//        xParms[2] = yParms[2] = makedcomplex(az[t] - casa::C::pi_4, 0.0);
        xParms[2] = yParms[2] = makedcomplex(az[t], 0.0);
        xParms[3] = yParms[3] = makedcomplex(el[t], 0.0);
                    
        for(int f = 0; f < request.nx(); ++f)
        {
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

            // Update frequency.
            xParms[1] += request.stepX();
            yParms[1] += request.stepX();
            
            // Move to next sample.
            ++sample;
        }
    }
}


} //# namespace BBS
} //# namespace LOFAR
