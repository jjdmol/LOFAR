//#  Exception.h: the base exception class for the Persistence Layer.
//#
//#  Copyright (C) 2002
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

#ifndef LCS_PL_EXCEPTION_H
#define LCS_PL_EXCEPTION_H

//# Includes
#include <Common/Exception.h>

namespace LCS {

  namespace PL {

    //
    // This is the base exception class for the Persistence Layer.
    //
    EXCEPTION_CLASS(PLException, LCS::Exception);


    // 
    // This exception is thrown when an error occurs during a delete
    // operation on the database.
    //
    EXCEPTION_CLASS(DeleteError, PLException);

    //
    // This exception is thrown when an error occurs during an insert
    // operation on the database.
    //
    EXCEPTION_CLASS(InsertError, PLException);

    //
    // This exception is thrown when an error occurs during a select
    // operation on the database.
    //
    EXCEPTION_CLASS(SelectError, PLException);

    //
    // This exception is thrown when an error occurs during an update
    // operation on the database.
    //
    EXCEPTION_CLASS(UpdateError, PLException);

    //
    // This exception is thrown when an error occurs during processing
    // of an SQL query.
    //
    EXCEPTION_CLASS(QueryError, PLException);

    //
    // This exception is thrown when a called method is not (yet) implemented.
    // One common situation for this exception is the use of a template
    // method for which a specialization is required.
    //
    EXCEPTION_CLASS(NotImplemented, PLException);

    //
    // This exception is thrown when an error occurs within the 
    // PersistenceBroker.
    //
    EXCEPTION_CLASS(BrokerException, PLException);
  }

}    

#endif
