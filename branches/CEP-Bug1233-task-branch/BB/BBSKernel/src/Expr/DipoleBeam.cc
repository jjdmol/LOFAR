//# DipoleBeam.cc: Dipole voltage beam (analytic)
//#
//# Copyright (C) 2007
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
#include <BBSKernel/Expr/DipoleBeam.h>
#include <Common/lofar_complex.h>
#include <Common/lofar_math.h>

namespace LOFAR
{
namespace BBS 
{
using LOFAR::sin;
using LOFAR::cos;
using LOFAR::tan;
using LOFAR::dcomplex;
using LOFAR::conj;


DipoleBeam::DipoleBeam(Expr azel, double height, double length,
    double slant, double orientation)
    :   itsHeight(height),
        itsLength(length),
        itsSlant(slant),
        itsOrientation(orientation)
{
    addChild(azel);
}


JonesResult DipoleBeam::getJResult(const Request &request)
{
    // Evaluate children.
    ResultVec res_azel;    
    const ResultVec &azel =
        getChild(DipoleBeam::IN_AZEL).getResultVecSynced(request, res_azel);

    // Create result.
    JonesResult result(request.nspid());
    Result& result11 = result.result11();
    Result& result12 = result.result12();
    Result& result21 = result.result21();
    Result& result22 = result.result22();
    
    // Evaluate main value.
    // Evaluate X polarization.
    evaluate(request, azel[0].getValue(), azel[1].getValue(),
        result11.getValueRW(), result12.getValueRW(), itsHeight, itsLength,
        itsSlant, itsOrientation);
    // Evaluate Y polarization.
    evaluate(request, azel[0].getValue(), azel[1].getValue(),
        result21.getValueRW(), result22.getValueRW(), itsHeight, itsLength,
        itsSlant, itsOrientation - casa::C::pi_2);

    // Evaluate perturbed values.  
    const ParmFunklet *perturbedParm;
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
    	else
    	{
    	    continue;
    	}

        // Evaluate X polarization.
        evaluate(request,
            azel[0].getPerturbedValue(i), azel[1].getPerturbedValue(i),
            result11.getPerturbedValueRW(i), result12.getPerturbedValueRW(i),
            itsHeight, itsLength, itsSlant, itsOrientation);
                
        // Evaluate Y polarization.
        evaluate(request,
            azel[0].getPerturbedValue(i), azel[1].getPerturbedValue(i),
            result21.getPerturbedValueRW(i), result22.getPerturbedValueRW(i),
            itsHeight, itsLength, itsSlant, itsOrientation - casa::C::pi_2);
        
        result11.setPerturbedParm(i, perturbedParm);
        result12.setPerturbedParm(i, perturbedParm);
        result21.setPerturbedParm(i, perturbedParm);
        result22.setPerturbedParm(i, perturbedParm);
    }
    
    return result;
}


void DipoleBeam::evaluate(const Request &request, const Matrix &in_az,
    const Matrix &in_el, Matrix &out_E_theta, Matrix &out_E_phi, 
    double height, double length, double slant, double orientation)
{
    const double *az = in_az.doubleStorage();
    const double *el = in_el.doubleStorage();

    double *E_theta_re, *E_theta_im;
    out_E_theta.setDCMat(request.nx(), request.ny());
    out_E_theta.dcomplexStorage(E_theta_re, E_theta_im);
    
    double *E_phi_re, *E_phi_im;
    out_E_phi.setDCMat(request.nx(), request.ny());
    out_E_phi.dcomplexStorage(E_phi_re, E_phi_im);
    
    for(int t = 0; t < request.ny(); ++t)
    {
        double phi = orientation + (*az);
        double theta = casa::C::pi_2 - (*el);
        
        if(theta >= casa::C::_2pi)
        {
            // Below the horizon.
            for(int f = 0; f < request.nx(); ++f)
            {
                *E_theta_re = 0.0;
                *E_theta_im = 0.0;
                E_theta_re++;
                E_theta_im++;

                *E_phi_re = 0.0;
                *E_phi_im = 0.0;
                E_phi_re++;
                E_phi_im++;
            }
        }
        else
        {
            // Common terms that depend on direction (and therefore on time).
            double sin_theta = sin(theta);
            double cos_theta = cos(theta);
            double sin_phi = sin(phi);
            double cos_phi = cos(phi);
            double sin_alpha = sin(slant);
            double cos_alpha = cos(slant);
            double tan_alpha = tan(slant);
            double inv_sin_alpha = 1.0 / sin_alpha;
            double inv_sin_alpha_sqr = 1.0 / (sin_alpha * sin_alpha);

            double A = sin_theta * cos_phi;
            double B = cos_theta / tan_alpha;
            double F = A + B;
            double G = A - B;

            double I = sin_alpha * sin_phi;
            double J = cos_alpha * sin_theta;
            double K = sin_alpha * cos_theta * cos_phi;
            double M = J + K;
            double N = J - K;

            double freq = request.domain().startX() + request.stepX() / 2.0;
                
            for(int f = 0; f < request.nx(); ++f)
            {
                // Common terms that depend on frequency.
                double omega = casa::C::_2pi * freq;
                double k = omega / casa::C::c;

                double C = k * length;
                double D = inv_sin_alpha * cos(C);
                double E = k * height * cos_theta;

                dcomplex term1 = makedcomplex(cos(E), sin(E));

                double term2_F_phase = F * C * sin_alpha;
                double term2_F_re = cos(term2_F_phase) * inv_sin_alpha - D;
                double term2_F_im =
                    sin(term2_F_phase) * inv_sin_alpha - F * sin(C);
                dcomplex term2_F = makedcomplex(term2_F_re, term2_F_im);
                // k-term vanishes when computing E_phi/E_theta.
                double term3_F = F * F - inv_sin_alpha_sqr;

                double term2_G_phase = G * C * sin_alpha;
                double term2_G_re = cos(term2_G_phase) * inv_sin_alpha - D;
                double term2_G_im =
                    sin(term2_G_phase) * inv_sin_alpha - G * sin(C);
                dcomplex term2_G = makedcomplex(term2_G_re, term2_G_im);
                // k-term vanishes when computing E_phi/E_theta.
                double term3_G = G * G - inv_sin_alpha_sqr;

                dcomplex Gamma1 = (-1.0 * term1 * term2_G) / term3_G;
                dcomplex Gamma2 = (-1.0 * term1 * term2_F) / term3_F;
                dcomplex Gamma3 = (-1.0 * conj(term1) * term2_G) / term3_G;
                dcomplex Gamma4 = (-1.0 * conj(term1) * term2_F) / term3_F;

                // mu = 1e-7 * 4.0 * pi --> mu / (4 * pi) = 1e-7
                double H = (1e-7 * inv_sin_alpha) * omega;
                dcomplex E_phi =
                    H * (N * Gamma1 - M * Gamma2 - N * Gamma3 + M * Gamma4);
                dcomplex E_theta = H * I * (Gamma1 + Gamma2 - Gamma3 - Gamma4);

                *(E_theta_re) = real(E_theta);
                *(E_theta_im) = imag(E_theta);
                ++E_theta_re;
                ++E_theta_im;

                *(E_phi_re) = real(E_phi);
                *(E_phi_im) = imag(E_phi);
                ++E_phi_re;
                ++E_phi_im;
                
                freq += request.stepX();
            }
        }
        ++az; ++el;
    }
}


} //# namespace BBS
} //# namespace LOFAR
