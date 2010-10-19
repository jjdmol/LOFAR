//# MeqPhaseShift.cc: Phase delay due to baseline geometry with respect to
//#     source direction.
//#
//# Copyright (C) 2005
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

#include <BBSKernel/MNS/MeqPhaseShift.h>
#include <BBSKernel/MNS/MeqDFTPS.h>
#include <BBSKernel/MNS/MeqStatUVW.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{
using LOFAR::dcomplex;
using LOFAR::conj;


MeqPhaseShift::MeqPhaseShift (const MeqExpr& left, const MeqExpr& right)
  : itsLeft  (left),
    itsRight (right)
{
    addChild (itsLeft);
    addChild (itsRight);
}


MeqPhaseShift::~MeqPhaseShift()
{
}


MeqResult MeqPhaseShift::getResult (const MeqRequest& request)
{
    MeqResult result(request.nspid());

    // Get N (for the division).
    // Assert it is a scalar value.
//    MeqResultVec lmnRes;
//    const MeqResultVec& lmn = itsLMN.getResultVecSynced (request, lmnRes);
//    const MeqResult& nk = lmn[2];
//    DBGASSERT (nk.getValue().nelements() == request.ny()  ||
//            nk.getValue().nelements() == 1);

    // Calculate the left and right station Jones matrix elements.
    // A delta in the station source predict is only available if multiple
    // frequency channels are used.
    bool multFreq = request.nx() > 1;
    MeqResultVec reslBuf, resrBuf;
    const MeqResultVec& resl = itsLeft.getResultVecSynced (request, resrBuf);
    const MeqResultVec& resr = itsRight.getResultVecSynced (request, reslBuf);
    const MeqResult& left = resl[0];
    const MeqResult& right = resr[0];
    const MeqResult& leftDelta = resl[1];
    const MeqResult& rightDelta = resr[1];

    // It is tried to compute the DFT as efficient as possible.
    // Therefore the baseline contribution is split into its antenna parts.
    // dft = exp(2i.pi(ul+vm+wn)) / n                 (with u,v,w in wavelengths)
    //     = (exp(2i.pi((u1.l+v1.m+w1.n) - (u2.l+v2.m+w2.n))/wvl))/n (u,v,w in m)
    //     = ((exp(i(u1.l+v1.m+w1.n)) / exp(i(u2.l+v2.m+w2.m))) ^ (2.pi/wvl)) / n
    // So left and right return the exp values independent of wavelength.
    // Thereafter they are scaled to the freq domain by raising the values
    // for each time to the appropriate powers.
    // Alas the rule
    //   x^(a*b) = (x^a)^b
    // which is valid for real numbers, is only valid for complex numbers
    // if b is an integer number.
    // Therefore the station calculations (in MeqStatSources) are done as
    // follows, where it should be noted that the frequencies are regularly
    // strided.
    //  f = f0 + k.df   (k = 0 ... nchan-1)
    //  s1 = (u1.l+v1.m+w1.n).2i.pi/c
    //  s2 = (u2.l+v2.m+w2.n).2i.pi/c
    //  dft = exp(s1(f0+k.df)) / exp(s2(f0+k.df)) / n
    //      = (exp(s1.f0)/exp(s2.f0)) . (exp(s1.k.df)/exp(s2.k.df)) / n
    //      = (exp(s1.f0)/exp(s2.f0)) . (exp(s1.df)/exp(s2.df))^k / n
    // In principle the power is expensive, but because the frequencies are
    // regularly strided, it is possible to use multiplication.
    // So it gets
    // dft(f0) = (exp(s1.f0)/exp(s2.f0)) / n
    // dft(fj) = dft(fj-1) * (exp(s1.df)/exp(s2.df))
    // Using a python script (tuvw.py) is is checked that this way of
    // calculation is accurate enough.
    // Another optimization can be achieved in the division of the two
    // complex numbers which can be turned into a cheaper multiplication.
    //  exp(x)/exp(y) = (cos(x) + i.sin(x)) / (cos(y) + i.sin(y))
    //                = (cos(x) + i.sin(x)) * (cos(y) - i.sin(y))
    
    /*
    {

        MeqDFTPS *dftpsl = dynamic_cast<MeqDFTPS*>(itsLeft.rep());
        MeqDFTPS *dftpsr = dynamic_cast<MeqDFTPS*>(itsRight.rep());

        const MeqResult& ul = dftpsl->uvw()->getU(request);
        const MeqResult& vl = dftpsl->uvw()->getV(request);
        const MeqResult& wl = dftpsl->uvw()->getW(request);
        const MeqResult& ur = dftpsr->uvw()->getU(request);
        const MeqResult& vr = dftpsr->uvw()->getV(request);
        const MeqResult& wr = dftpsr->uvw()->getW(request);
        cout << "U: " << setprecision(20) << ur.getValue() - ul.getValue()
            << endl;
        cout << "V: " << setprecision(20) << vr.getValue() - vl.getValue()
            << endl;
        cout << "W: " << setprecision(20) << wr.getValue() - wl.getValue()
            << endl;
        LOG_TRACE_FLOW ("U: " << ur.getValue() - ul.getValue());
        LOG_TRACE_FLOW ("V: " << vr.getValue() - vl.getValue());
        LOG_TRACE_FLOW ("W: " << wr.getValue() - wl.getValue());
    }
    */
    
    MeqMatrix res(makedcomplex(0,0), request.nx(), request.ny(), false);
    for(int iy=0; iy<request.ny(); ++iy)
    {
        dcomplex tmpl = left.getValue().getDComplex(0,iy);
        dcomplex tmpr = right.getValue().getDComplex(0,iy);
        
        // We have to divide by N.
        // However, we divide by 2N to get the factor 0.5 needed in (I+Q)/2, etc.
        // in MeqBaseLinPS.
        //    double tmpnk = 2. * nk.getValue().getDouble(0,iy);
//        double tmpnk = 2.0;
        dcomplex factor;
        if(multFreq)
        {
            dcomplex deltal = leftDelta.getValue().getDComplex(0,iy);
            dcomplex deltar = rightDelta.getValue().getDComplex(0,iy);
            factor = deltar * conj(deltal);
        }
//        res.fillRowWithProducts (tmpr * conj(tmpl) / tmpnk, factor, iy);
        res.fillRowWithProducts (tmpr * conj(tmpl), factor, iy);
    }
    result.setValue (res);

    //  cout << "DFT:" << endl;
    //  cout << setprecision(20) << res << endl;

    // Evaluate (if needed) for the perturbed parameter values.
    // Note that we do not have to test for perturbed values in nk,
    // because the left and right value already depend on nk.
    const MeqParmFunklet* perturbedParm;
    
    for(int spinx=0; spinx<request.nspid(); spinx++)
    {
        bool eval = false;
        if(left.isDefined(spinx))
        {
            eval = true;
            perturbedParm = left.getPerturbedParm(spinx);
        }
        else if(right.isDefined(spinx))
        {
            eval = true;
            perturbedParm = right.getPerturbedParm(spinx);
        }
        
        if(eval)
        {
            MeqMatrix pres(makedcomplex(0,0), request.nx(), request.ny(), false);
            for(int iy=0; iy<request.ny(); ++iy)
            {
                dcomplex tmpl = left.getPerturbedValue(spinx).getDComplex(0,iy);
                dcomplex tmpr = right.getPerturbedValue(spinx).getDComplex(0,iy);
                
                // double tmpnk = 2. * nk.getPerturbedValue(spinx).getDouble(0,iy);
//                double tmpnk = 2.0;
                dcomplex factor;
                if(multFreq)
                {
                    dcomplex deltal = leftDelta.getPerturbedValue(spinx).getDComplex(0,iy);
                    dcomplex deltar = rightDelta.getPerturbedValue(spinx).getDComplex(0,iy);
                    factor = deltar * conj(deltal);
                }
//                pres.fillRowWithProducts(tmpr * conj(tmpl) / tmpnk, factor, iy);
                pres.fillRowWithProducts(tmpr * conj(tmpl), factor, iy);
                result.setPerturbedValue (spinx, pres);
                result.setPerturbedParm (spinx, perturbedParm);
            }
        }
    }
    
    return result;
}

#ifdef EXPR_GRAPH
std::string MeqPhaseShift::getLabel()
{
    return std::string("MeqPhaseShift\\nPhase delay due to baseline geometry.");
}
#endif

} // namespace BBS
} // namespace LOFAR
