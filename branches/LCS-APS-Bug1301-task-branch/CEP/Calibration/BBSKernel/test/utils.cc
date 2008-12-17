//# utils.cc: Test utility functions.
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
#include <utils.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{

bool compare(const Matrix &lhs, const dcomplex &rhs, double tol)
{
    if(!lhs.isComplex())
    {
        LOG_ERROR("Type mismatch.");
        return false;
    }
    
    const double *lhsRe, *lhsIm;
    lhs.dcomplexStorage(lhsRe, lhsIm);
       
    for(size_t i = 0; i < static_cast<size_t>(lhs.nelements()); ++i)
    {
        if(!near(lhsRe[i], real(rhs), tol) || !near(lhsIm[i], imag(rhs), tol))
        {
            LOG_ERROR_STR("tol: " << tol << " mismatch: " << dcomplex(lhsRe[i],
                lhsIm[i]) << " <> " << rhs);
            return false;
        }
    }
    
    return true;
}

bool compare(const Matrix &lhs, const Matrix &rhs, double tol)
{
    if(lhs.isComplex() != rhs.isComplex() || lhs.nelements() != rhs.nelements())
    {
        LOG_ERROR("Type or shape mismatch.");
        return false;
    }

    if(lhs.isComplex())
    {
        const double *lhsRe, *lhsIm;
        lhs.dcomplexStorage(lhsRe, lhsIm);
        const double *rhsRe, *rhsIm;
        rhs.dcomplexStorage(rhsRe, rhsIm);
        
        for(size_t i = 0; i < static_cast<size_t>(lhs.nelements()); ++i)
        {
            if(!near(lhsRe[i], rhsRe[i], tol) || !near(lhsIm[i], rhsIm[i], tol))
            {
                LOG_ERROR_STR("tol: " << tol << " mismatch: "
                    << setprecision(16)
                    << dcomplex(lhsRe[i], lhsIm[i]) << " <> "
                    << dcomplex(rhsRe[i], rhsIm[i]));
                return false;
            }
        }
    }
    else
    {
        const double *lhsVal = lhs.doubleStorage();
        const double *rhsVal = rhs.doubleStorage();
        
        for(size_t i = 0; i < static_cast<size_t>(lhs.nelements()); ++i)
        {
            if(!near(lhsVal[i], rhsVal[i], tol))
            {
                LOG_ERROR_STR("tol: " << tol << " mismatch: " << lhsVal[i]
                    << " <> " << rhsVal[i]);
                return false;
            }
        }
    }
    
    return true;
}

void log(const string &name, bool result)
{
    if(result)
    {
        LOG_DEBUG_STR(name << "... OK");
    }
    else
    {
        LOG_ERROR_STR(name << "... FAIL");
    }
}

JNodeStub::JNodeStub(const Matrix &v11, const Matrix &v12, const Matrix &v21,
    const Matrix &v22, dcomplex perturbation, PValueKey key)
    :   itsV11(v11), 
        itsV12(v12), 
        itsV21(v21), 
        itsV22(v22), 
        itsPerturbation(perturbation),
        itsKey(key)
{
}
    
JNodeStub::~JNodeStub()
{
}

JonesResult JNodeStub::getJResult(const Request &request)
{
    JonesResult result;
    result.init();
    
    Result &result11 = result.result11();
    Result &result12 = result.result12();
    Result &result21 = result.result21();
    Result &result22 = result.result22();
    
    result11.setValue(itsV11.clone());
    result12.setValue(itsV12.clone());
    result21.setValue(itsV21.clone());
    result22.setValue(itsV22.clone());

    if(request.getPValueFlag())
    {
        result11.setPerturbedValue(itsKey, itsV11 + itsPerturbation);
        result12.setPerturbedValue(itsKey, itsV12 + itsPerturbation);
        result21.setPerturbedValue(itsKey, itsV21 + itsPerturbation);
        result22.setPerturbedValue(itsKey, itsV22 + itsPerturbation);
    }
    
    return result;
}

} //# namespace LOFAR
} //# namespace BBS


