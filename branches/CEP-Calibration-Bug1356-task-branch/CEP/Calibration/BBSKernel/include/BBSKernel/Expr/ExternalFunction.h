//# ExternalFunction.h: Dynamically loaded function.
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

#ifndef LOFAR_BBSKERNEL_EXPR_EXTERNALFUNCTION_H
#define LOFAR_BBSKERNEL_EXPR_EXTERNALFUNCTION_H

#include <Common/lofar_complex.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class ExternalFunction
{
public:
    ExternalFunction(const string &module, const string &name);
    ~ExternalFunction();

    unsigned int nArguments() const;
    dcomplex operator()(const vector<dcomplex> &args) const;

private:
    //# Define the signature of the external function.
    typedef dcomplex (*signature_t)(const dcomplex *par, const dcomplex *x);

    // Try to find a specific symbol in the module.
    void *getSymbol(const string &name) const;

    void            *itsModule;
    signature_t     itsFunction;
    int             itsNX, itsNPar;
};

inline unsigned int ExternalFunction::nArguments() const
{
    return itsNX + itsNPar;
}

inline dcomplex ExternalFunction::operator()(const vector<dcomplex> &args) const
{
    DBGASSERT(itsFunction);
    DBGASSERT(args.size() == static_cast<size_t>(nArguments()));
    return itsFunction(&args[itsNX], &args[0]);
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
