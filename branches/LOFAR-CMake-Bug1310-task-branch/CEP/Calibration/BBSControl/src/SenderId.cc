//#  SenderId.cc: Identifier for sender of messages.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/SenderId.h>

namespace LOFAR
{
  namespace BBS
  {
    SenderId::SenderId(Type type, int id) : itsType(UNKNOWN), itsId(-1)
    {
      if (isValid(type, id)) set(type, id);
    }



    SenderId::SenderId(int itype, int id) : itsType(UNKNOWN), itsId(-1)
    {
      Type type(static_cast<Type>(itype));
      if (isValid(type, id)) set(type, id);
    }


    bool SenderId::isValid() const
    {
      return isValid(itsType, itsId);
    }


    void SenderId::set(Type type, int id)
    {
      if (isValid(type, id)) {
        itsType = type;
        itsId = id;
      } else {
        itsType = UNKNOWN;
        itsId = -1;
      }
    }


    bool SenderId::isValid(Type type, int id) const
    {
      return UNKNOWN < type && type < N_Types && id >= 0;
    }


    const string& SenderId::typeAsString() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          Type that is defined in the header file!
      static const string typeString[N_Types+1] = {
        "Kernel",
        "Solver",
        "<Unknown>"  //# This line should ALWAYS be last!
      };
      return (UNKNOWN < itsType && itsType < N_Types) ?
        typeString[itsType] : typeString[N_Types];
    }


    bool operator<(const SenderId& lhs, const SenderId& rhs)
    { 
      return 
        (lhs.isValid() && rhs.isValid()) && 
        (lhs.type() < rhs.type() ||
         (lhs.type() == rhs.type() && lhs.id() < rhs.id()));
    }


    ostream& operator<<(ostream& os, const SenderId& si)
    {
      return os << si.typeAsString() << " " << si.id();
    }


  } // namespace BBS

} // namespace LOFAR
