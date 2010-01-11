//# utils.h: Test utility functions.
//#
//# Copyright (C) 2008
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

#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_complex.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace BBS
{

// Test for equivalence up to a specified tolerance.
template <typename T>
bool near(const T &x, const T &y, double tol = 10-8)
{
    if(x == y) return true;
    if(abs(x - y) <= tol) return true;
    if(abs(x) > abs(y)) return (abs((x - y) / x) <= tol);
    else return (abs((x - y) / y) <= tol);
}
    
bool compare(const Matrix &lhs, const dcomplex &rhs, double tol);
bool compare(const Matrix &lhs, const Matrix &rhs, double tol);

// Log the outcome of a named test.
void log(const string &name, bool result);

// Stub class used for testing any node that takes a JonesExpr as input.
class JNodeStub: public JonesExprRep
{
public:
    JNodeStub(const Matrix &v11, const Matrix &v12, const Matrix &v21,
        const Matrix &v22, dcomplex perturbation = dcomplex(1e-6, 1e-6),
        PValueKey key = PValueKey(1, 0));
    ~JNodeStub();

    virtual JonesResult getJResult(const Request &request);

private:
    Matrix      itsV11, itsV12, itsV21, itsV22;
    dcomplex    itsPerturbation;        
    PValueKey   itsKey;
};

} //# namespace LOFAR
} //# namespace BBS

