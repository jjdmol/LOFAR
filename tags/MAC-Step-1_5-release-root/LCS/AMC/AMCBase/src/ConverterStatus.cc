//#  ConverterStatus.cc: Status that will be returned by the Converter server
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
#include <AMCBase/ConverterStatus.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace AMC
  {

    ConverterStatus::ConverterStatus(Status sts, const string& txt) :
      itsText(txt)
    {
      if (UNKNOWN < sts && sts < N_Status) itsStatus = sts;
      else itsStatus = UNKNOWN;
    }

    const string& ConverterStatus::asString() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          Status that is defined in the header file!
      static const string stsString[N_Status+1] = {
        "Success",
        "Converter error",
        "Unknown error"    //# This line should ALWAYS be last!
      };
      if (itsStatus < 0) return stsString[N_Status];
      else return stsString[itsStatus];
    }


    ostream& operator<<(ostream& os, const ConverterStatus& cs)
    {
      os << cs.asString();
      if (!cs.text().empty()) os << ": " << cs.text();
      return os;
    }

  } // namespace AMC
  
} // namespace LOFAR
