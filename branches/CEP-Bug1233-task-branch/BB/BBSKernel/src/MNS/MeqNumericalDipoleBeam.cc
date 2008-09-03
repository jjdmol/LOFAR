//# MeqNumericalDipoleBeam.h: Implementation of J.P. Hamaker's memo
//# "Mathematical-physical analysis of the generic dual-dipole antenna"
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
#include <BBSKernel/MNS/MeqNumericalDipoleBeam.h>

#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS 
{

MeqNumericalDipoleBeam::MeqNumericalDipoleBeam(const BeamCoeff &coeff,
    const MeqExpr &azel, const MeqExpr &orientation)
    : itsBeamCoeff(coeff)
{
    ASSERT(itsBeamCoeff.coeff);
    ASSERT(itsBeamCoeff.coeff->size() > 0);
    
    addChild(azel);
    addChild(orientation);
}


MeqJonesResult MeqNumericalDipoleBeam::getJResult(const MeqRequest &request)
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


void MeqNumericalDipoleBeam::evaluate(const MeqRequest &request,
    const MeqMatrix &in_az, const MeqMatrix &in_el,
    const MeqMatrix &in_orientation,
    MeqMatrix &out_E11, MeqMatrix &out_E12,
    MeqMatrix &out_E21, MeqMatrix &out_E22)
{
    // Check preconditions.
    ASSERT(in_az.nelements() == request.ny());
    ASSERT(in_el.nelements() == request.ny());
    ASSERT(in_orientation.nelements() == 1);
    
    // Get pointers to input and output data.
    const double *az = in_az.doubleStorage();
    const double *el = in_el.doubleStorage();
    const double orientation = in_orientation.getDouble(0, 0);

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
    
    // Evaluate beam.
    const size_t nHarmonics = itsBeamCoeff.coeff->shape()[1];
    const size_t nTheta = itsBeamCoeff.coeff->shape()[2];
    const size_t nFreq = itsBeamCoeff.coeff->shape()[3];

    const boost::multi_array<dcomplex, 4> &coeff = *itsBeamCoeff.coeff;

    size_t sample = 0;
    for(size_t t = 0; t < static_cast<size_t>(request.ny()); ++t)
    {
        double freq = request.domain().startX() + 0.5 * request.stepX();

        // Correct azimuth for dipole orientation.
        const double phi = az[t] - orientation;

        // NB: The model is parameterized in terms of zenith angle. The
        // appropriate conversion is taken care of below.
        const double theta = casa::C::pi_2 - el[t];

        for(size_t f = 0; f < static_cast<size_t>(request.nx()); ++f)
        {
            // NB: The model is parameterized in terms of a normalized
            // frequency in the range [-1, 1]. The appropriate conversion is
            // taken care of below.
            const double scaledFreq = (freq - itsBeamCoeff.freqAvg)
                / itsBeamCoeff.freqRange;

            // J-jones matrix (2x2 complex matrix)
            dcomplex J[2][2];
            J[0][0] = J[0][1] = J[1][0] = J[1][1] = makedcomplex(0.0, 0.0);
            
            for(size_t k = 0; k < nHarmonics; ++k)
            {
                // Compute diagonal projection matrix P for the current
                // harmonic.
                dcomplex P[2];
                P[0] = P[1] = makedcomplex(0.0, 0.0);

                dcomplex inner[2];
                for(long i = nTheta - 1; i >= 0; --i)
                {
                    inner[0] = coeff[0][k][i][nFreq - 1];
                    inner[1] = coeff[1][k][i][nFreq - 1];

                    for(long j = nFreq - 2; j >= 0; --j)
                    {
                        inner[0] = inner[0] * scaledFreq + coeff[0][k][i][j];
                        inner[1] = inner[1] * scaledFreq + coeff[1][k][i][j];
                    }
                    P[0] = P[0] * theta + inner[0];
                    P[1] = P[1] * theta + inner[1];
                }

                // Compute J-jonex matrix for this harmonic by rotating
                // P over kappa * phi and add it to the result.
                const double kappa = ((k & 1) == 0 ? 1.0 : -1.0) * (2.0 * k + 1.0);
                const double cphi = std::cos(kappa * phi);
                const double sphi = std::sin(kappa * phi);
                
                J[0][0] += cphi * P[0];
                J[0][1] += -sphi * P[1];
                J[1][0] += sphi * P[0];
                J[1][1] += cphi * P[1];
            }

            E11_re[sample] = real(J[0][0]);
            E11_im[sample] = imag(J[0][0]);
            E12_re[sample] = real(J[0][1]);
            E12_im[sample] = imag(J[0][1]);
            E21_re[sample] = real(J[1][0]);
            E21_im[sample] = imag(J[1][0]);
            E22_re[sample] = real(J[1][1]);
            E22_im[sample] = imag(J[1][1]);

            // Update frequency.
            freq += request.stepX();

            // Move to next sample.
            ++sample;
        }
    }
}


#ifdef EXPR_GRAPH
std::string MeqNumericalDipoleBeam::getLabel()
{
    return std::string("MeqNumericalDipoleBeam\\nDipole voltage beam based on"
        " numerical simulation");
}
#endif

} //# namespace BBS
} //# namespace LOFAR
