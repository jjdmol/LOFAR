//# ExternalFunction.cc: Dynamically loaded function.
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

#include <BBSKernel/Expr/ExternalFunction.h>
#include <BBSKernel/Exceptions.h>

#include <dlfcn.h>

namespace LOFAR
{
namespace BBS
{

ExternalFunction::ExternalFunction(const string &module, const string &name)
{
    itsModule = dlopen(module.c_str(), RTLD_LAZY);
    if(!itsModule)
    {
        THROW(BBSKernelException, "Unable to open module: " << module);
    }

    itsNX = *static_cast<int*>(getSymbol("Nx_" + name));
    itsNPar = *static_cast<int*>(getSymbol("Npar_" + name));
    itsFunction =
        reinterpret_cast<signature_t>(getSymbol(name + "_complex"));
}

ExternalFunction::~ExternalFunction()
{
    ASSERT(itsModule);
    dlclose(itsModule);
}

void *ExternalFunction::getSymbol(const string &name) const
{
    ASSERT(itsModule);

    // Clear error flag.
    dlerror();

    // Attempt to get symbol.
    void *ptr = dlsym(itsModule, name.c_str());

    // If an error occurred, throw an exception.
    const char *dlsymError = dlerror();
    if(dlsymError)
    {
        THROW(BBSKernelException, "Undefined symbol: " << name << " ("
            << dlsymError << ")");
    }

    return ptr;
}

} // namespace BBS
} // namespace LOFAR
