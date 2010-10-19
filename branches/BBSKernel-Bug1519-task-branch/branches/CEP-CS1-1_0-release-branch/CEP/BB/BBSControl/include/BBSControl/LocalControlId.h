//#  LocalControlId.h: Identifier for a local controller
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

#ifndef LOFAR_BBSCONTROL_LOCALCONTROLID_H
#define LOFAR_BBSCONTROL_LOCALCONTROLID_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Identifier for a local controller

//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace BBS {

    // \addtogroup BBSControl
    // @{

    // Class representing the identifier for a local controller. It is stored
    // internally as a string and an integer, but this may change in the future.
    class LocalControlId
    {
    public:
      // Construct a local controller id.
      explicit LocalControlId(const string& node, int pid)
        :   itsNode(node),
            itsProcessId(pid) 
      {}

      // Get the local controller id as a string.
      string asString() const
      {
        ostringstream oss;
        oss << itsNode << ":" << itsProcessId;
        return oss.str();
      }

    private:
      // Internal id.
      string    itsNode;
      int       itsProcessId;

      // Operator "less-than". Required when using LocalControlId key in a map.
      friend bool operator<(const LocalControlId& lhs, 
                            const LocalControlId& rhs);

      // Write LocalControlId \a id in human readable format to output stream
      // \a os.
      friend ostream& operator<<(ostream& os, const LocalControlId& id);
    };

    // @}

    inline bool operator<(const LocalControlId& lhs, 
                          const LocalControlId& rhs)
    {
      return (lhs.itsNode < rhs.itsNode) || (lhs.itsNode == rhs.itsNode
        && lhs.itsProcessId < rhs.itsProcessId);
    }

    inline ostream& operator<<(ostream& os, const LocalControlId& id)
    {
      return os << id.itsNode << ":" << id.itsProcessId;
    }
    
  } // namespace BBS

} // namespace LOFAR

#endif
