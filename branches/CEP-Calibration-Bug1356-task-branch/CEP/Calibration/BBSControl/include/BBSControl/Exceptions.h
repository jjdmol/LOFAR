//#  Exceptions.h: definition of the BBSControl specific exception classes.
//#
//#  Copyright (C) 2002-2007
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

#ifndef LOFAR_BBSCONTROL_EXCEPTIONS_H
#define LOFAR_BBSCONTROL_EXCEPTIONS_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Definition of the BBSControl specific exception classes

//# Includes
#include <Common/Exceptions.h>

namespace LOFAR
{
  namespace BBS
  {
    // \ingroup BBSControl
    // @{

    // Top-level exception class for the BBSControl package.
    EXCEPTION_CLASS(BBSControlException, LOFAR::Exception);

    // Exception class for the global controller.
    EXCEPTION_CLASS(GlobalControlException, BBSControlException);

    // Exception class for the kernel controller.
    EXCEPTION_CLASS(KernelControlException, BBSControlException);

    // Exception class for the (global) solver controller.
    EXCEPTION_CLASS(SolverControlException, BBSControlException);

    // Exception class for the solve task.
    EXCEPTION_CLASS(SolveTaskException, BBSControlException);

    // Exception class for the database access layer.
    EXCEPTION_CLASS(DatabaseException, BBSControlException);

    // Exception class to signal failed data translation to / from the database
    // access layer.
    EXCEPTION_CLASS(TranslationException, DatabaseException);

    // Exception class for the shared state.
    EXCEPTION_CLASS(CalSessionException, BBSControlException);

    //  @}

  } // namespace BBS
  
} // namespace LOFAR

#endif
