//#  SenderId.h: Identifier for sender of messages.
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

#ifndef LOFAR_BBSCONTROL_SENDERID_H
#define LOFAR_BBSCONTROL_SENDERID_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Identifier for sender of messages.

//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>
#include <Common/StringUtil.h>

namespace LOFAR
{
  namespace BBS {

    // \addtogroup BBSControl
    // @{

    // Identifier for the sender of messages. The sender is identified by its
    // type (e.g. \e kernel of \e solver) and its unique ID. 
    class SenderId
    {
    public:
      // Type of sender.
      enum Type {
        UNKNOWN = -1,
        KERNEL,
        SOLVER,
        //# Insert new types HERE !!
        N_Types
      };

      // Construct a sender ID.
      SenderId(Type type = UNKNOWN, int id = -1);

      // This constructor was added to make the life of the programmer a
      // little brighter. The job of converting the \c int \a type to an \c
      // enum Type is now done by the constructor.
      SenderId(int itype, int id = -1);

      // Is this a valid ID? For example, a default constructed SenderId is
      // invalid.
      bool isValid() const;

      // Return the type of sender.
      Type type() const { return itsType; }

      // Return the sender ID.
      int id() const { return itsId; }

      // Return the sender type as a string.
      const string& typeAsString() const;

    private:
      // Set sender type and ID.
      void set(Type type, int id);

      // Check to see if \a type and \a id are valid (i.e. in range).
      bool isValid(Type type, int id) const;

      // Sender type.
      Type itsType;

      // Sender ID.
      int itsId;
    };

    // Comparison operator. Needed when storing SenderId objects in an
    // associative STL container.
    bool operator<(const SenderId& lhs, const SenderId& rhs);
    
    // Write SenderId \a id in human readable form to output stream \a os.
    ostream& operator<<(ostream& os, const SenderId& id);

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
