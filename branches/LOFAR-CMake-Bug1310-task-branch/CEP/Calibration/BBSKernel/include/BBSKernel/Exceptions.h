//#  Exceptions.h: definition of the BBSKernel specific exception classes.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_BBSKERNEL_EXCEPTIONS_H
#define LOFAR_BBSKERNEL_EXCEPTIONS_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Definition of the BBSKernel specific exception classes

#include <Common/Exception.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// @{

// This exception is thrown when an error occurs in the BBS kernel part.
EXCEPTION_CLASS(BBSKernelException, LOFAR::Exception);

// @}

} // namespace BBS
} // namespace LOFAR

#endif
