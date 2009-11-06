//#  Exceptions.h: definition of the AMCImpl specific exception classes.
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

#ifndef LOFAR_AMCIMPL_EXCEPTIONS_H
#define LOFAR_AMCIMPL_EXCEPTIONS_H

// \file
// Definition of the AMCImpl specific exception classes

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <AMCBase/Exceptions.h>

namespace LOFAR
{
  namespace AMC
  {

    // \addtogroup AMCImpl
    // @{

    // This exception is thrown when an error occurs within the conversion
    // methods.
    EXCEPTION_CLASS(ConverterError, Exception);

    // This exception is thrown when an error occurs within the AMCServer
    // sub-package.
    EXCEPTION_CLASS(ServerError, Exception);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
