//# ExternalFunction.cc: Dynamically loaded function.
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

#include <lofar_config.h>

#include <BBSKernel/Expr/ExternalFunction.h>
#include <BBSKernel/Exceptions.h>

#include <casa/OS/Path.h>
#include <dlfcn.h>

namespace LOFAR
{
namespace BBS
{

ExternalFunction::ExternalFunction(const casa::Path &module, const string &name)
    :   itsModule(0)
{
    casa::String path = module.expandedName();
    itsModule = dlopen(path.c_str(), RTLD_LAZY);
    if(!itsModule)
    {
        THROW(BBSKernelException, "Unable to open module: " << path);
    }

    itsNX = *static_cast<int*>(getSymbol("Nx_" + name));
    itsNPar = *static_cast<int*>(getSymbol("Npar_" + name));
    itsFunction =
        reinterpret_cast<signature_t>(getSymbol(name + "_complex"));
}

ExternalFunction::~ExternalFunction()
{
    if(itsModule)
    {
        dlclose(itsModule);
    }
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
