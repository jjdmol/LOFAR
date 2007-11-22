//#  ConverterStatus.h: Status that will be returned by the Converter server
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

#ifndef LOFAR_AMCBASE_CONVERTERSTATUS_H
#define LOFAR_AMCBASE_CONVERTERSTATUS_H

// \file
// Status that will be returned by the Converter server

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace AMC
  {

    // \addtogroup AMCBase
    // @{

    // Class representing the status that will be returned by the
    // converter. This class is most useful in a client/server configuration,
    // because then, the only way to communicate failure from server to client
    // is through a return value. It encapsulates the return value (an
    // enumerated value), with its associated return value as a string, and an
    // optional additional error message. A ConverterStatus instance can be
    // transported using the BlobIStream and BlobOStream classes.
    class ConverterStatus
    {
    public:
      // Status values that can be set by the Converter server.
      enum Status {
        UNKNOWN = -1,  ///< An unknown error occurred.
        OK,            ///< No errors.
        ERROR,         ///< Something definitely went wrong.
        //# Insert new return values HERE !!
        N_Status       ///< Number of status values.
      };

      // If \a sts matches with one of the enumeration values in Status, then
      // itsStatus is set to \a sts, else itsStatus is set to \c UNKNOWN. The
      // user can optionally supply extra text, providing a more detailed
      // description of the actual error.
      ConverterStatus(Status sts = OK, 
                      const string& txt = string());

      // Get the return value.
      Status get() const
      { return itsStatus; }

      // Return the user-supplied text.
      const string& text() const
      { return itsText; }

      // Return the status as a string.
      const string& asString() const;

      // Return \c true if status is OK, else return \c false.
      operator bool() const
      { return itsStatus == OK; }

    private:
      // The return status
      Status itsStatus;

      // User-supplied optional text.
      string itsText;

    };

    // Output in ASCII format.
    ostream& operator<<(ostream& os, const ConverterStatus& cs);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
