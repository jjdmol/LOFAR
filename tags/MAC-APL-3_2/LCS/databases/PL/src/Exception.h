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

#ifndef LOFAR_PL_EXCEPTION_H
#define LOFAR_PL_EXCEPTION_H

// \file Exception.h
// The base exception class for the Persistence Layer.

//# Includes
#include <Common/Exception.h>

namespace LOFAR {

  namespace PL {

    // \addtogroup PL
    // @{
    
    //
    // This is the base exception class for the Persistence Layer.
    //
    EXCEPTION_CLASS(Exception, LOFAR::Exception);


    // 
    // This exception is thrown when an error occurs during a delete
    // operation on the database.
    //
    EXCEPTION_CLASS(EraseError, Exception);

    //
    // This exception is thrown when an error occurs during an insert
    // operation on the database.
    //
    EXCEPTION_CLASS(InsertError, Exception);

    //
    // This exception is thrown when an error occurs during a select
    // operation on the database.
    //
    EXCEPTION_CLASS(RetrieveError, Exception);

    //
    // This exception is thrown when an error occurs during an update
    // operation on the database.
    //
    EXCEPTION_CLASS(UpdateError, Exception);

    //
    // This exception is thrown when an error occurs during processing
    // of an SQL query.
    //
    EXCEPTION_CLASS(QueryError, Exception);

    //
    // This exception is thrown when an error occurs within the 
    // PersistenceBroker class.
    //
    EXCEPTION_CLASS(BrokerException, Exception);

    //
    // This exception is thrown when an error occurs within the 
    // Collection class.
    //
    EXCEPTION_CLASS(CollectionException, Exception);

    // @}

  }

}    

#endif
