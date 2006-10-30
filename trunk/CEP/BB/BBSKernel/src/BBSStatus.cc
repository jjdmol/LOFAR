//#  BBSStatus.cc: Status that will be returned by the BBS kernel.
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
#include <BBSKernel/BBSStatus.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarTypes.h>

namespace LOFAR
{
  namespace BBS
  {

    //##--------   S t a t i c   i n i t i a l i z a t i o n   --------##//

    // Register BBSStatus with the BBSStreamableFactory. Use an anonymous
    // namespace. This ensures that the variable `dummy' gets its own private
    // storage area and is only visible in this compilation unit.
    namespace
    {
      bool dummy = BlobStreamableFactory::instance().
	registerClass<BBSStatus>("BBSStatus");
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    BBSStatus::BBSStatus(Status sts, const string& txt) :
      itsText(txt)
    {
      if (UNKNOWN < sts && sts < N_Status) itsStatus = sts;
      else itsStatus = UNKNOWN;
    }

    const string& BBSStatus::asString() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          Status that is defined in the header file!
      static const string stsString[N_Status+1] = {
        "Success",
        "BBSKernel error",
        "Unknown error"    //# This line should ALWAYS be last!
      };
      if (itsStatus < 0) return stsString[N_Status];
      else return stsString[itsStatus];
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    void BBSStatus::read(BlobIStream& bis)
    {
      int32 sts;
      bis >> sts >> itsText;
      itsStatus = static_cast<Status>(sts);
    }

    // Write the contents of \c *this into the blob output stream \a bos.
    void BBSStatus::write(BlobOStream& bos) const
    {
      bos << static_cast<int32>(itsStatus)
	  << itsText;
    }

    // Return the class type of \c *this as a string.
    const string& BBSStatus::classType() const
    {
      static const string theType("BBSStatus");
      return theType;
    }
    

    //##--------   G l o b a l   m e t h o d s   --------##//

    ostream& operator<<(ostream& os, const BBSStatus& bs)
    {
      os << bs.asString();
      if (!bs.text().empty()) os << ": " << bs.text();
      return os;
    }

  } // namespace BBS
  
} // namespace LOFAR
