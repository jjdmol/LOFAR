//#  ConnectionHandler.h: handles connection to the database.
//#
//#  Copyright (C) 2002-2003
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

#ifndef LOFAR_PL_CONNECTIONHANDLER_H
#define LOFAR_PL_CONNECTIONHANDLER_H

// \file ConnectionHandler.h
// Handles connection to the database.

//# Includes
#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace PL
  {
    // \addtogroup PL
    // @{

    // This class is responsible for handling the connection to the database.
    // It is an adapter class, because is decouples the PersistenceBroker from
    // the Database Template Library (DTL). As such, this class will not be of
    // interest to ordinary users of the PL library.
    class ConnectionHandler
    {
    public:
      static void connect(const string& aDsn, const string& aUid, 
                          const string& aPwd);
    private:
      ConnectionHandler();
      ConnectionHandler(const ConnectionHandler&);
      ConnectionHandler& operator=(const ConnectionHandler&);
    };

    // @}

  } // namespace PL

} // namespace LOFAR

#endif
