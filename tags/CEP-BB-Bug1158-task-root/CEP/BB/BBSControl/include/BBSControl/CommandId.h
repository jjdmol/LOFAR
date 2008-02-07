//#  CommandId.h: Command identifier
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

#ifndef LOFAR_BBSCONTROL_COMMANDID_H
#define LOFAR_BBSCONTROL_COMMANDID_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Command identifier

//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>
#include <Common/StringUtil.h>

namespace LOFAR
{
  namespace BBS {

    // \addtogroup BBSControl
    // @{

    // Command identifier. Encapsulates the identifier that is used to
    // uniquely identify a command. Internally, this ID is an integer, but
    // this may change in the future. 
    class CommandId
    {
    public:
      // Construct a command-id.
      CommandId(int id = -1) : itsId(id) {}

      // Return the command-id as an integer.
      // \note This method is provided so that we can later easily change the
      // internal format of the ID to something different than \c int.
      int asInt() const { return itsId; }

    private:
      // Unique id.
      int itsId;

      // Comparison operators.
      // @{
      friend bool operator<(const CommandId& lhs, const CommandId& rhs);
      friend bool operator>(const CommandId& lhs, const CommandId& rhs);
      friend bool operator<=(const CommandId& lhs, const CommandId& rhs);
      friend bool operator>=(const CommandId& lhs, const CommandId& rhs);
      friend bool operator==(const CommandId& lhs, const CommandId& rhs);
      friend bool operator!=(const CommandId& lhs, const CommandId& rhs);
      // @}

      // Write CommandId \a id in human readable form to output stream \a os.
      friend ostream& operator<<(ostream& os, const CommandId& id);
    };

    // @}

    inline bool operator<(const CommandId& lhs, const CommandId& rhs)
    { 
      return lhs.itsId < rhs.itsId; 
    }

    inline bool operator>(const CommandId& lhs, const CommandId& rhs)
    { 
      return lhs.itsId > rhs.itsId; 
    }

    inline bool operator<=(const CommandId& lhs, const CommandId& rhs)
    { 
      return lhs.itsId <= rhs.itsId; 
    }

    inline bool operator>=(const CommandId& lhs, const CommandId& rhs)
    { 
      return lhs.itsId >= rhs.itsId; 
    }

    inline bool operator==(const CommandId& lhs, const CommandId& rhs)
    { 
      return lhs.itsId == rhs.itsId; 
    }

    inline bool operator!=(const CommandId& lhs, const CommandId& rhs)
    { 
      return lhs.itsId != rhs.itsId; 
    }

    inline ostream& operator<<(ostream& os, const CommandId& id)
    {
      return os << id.itsId;
    }

  } // namespace BBS

} // namespace LOFAR

#endif
