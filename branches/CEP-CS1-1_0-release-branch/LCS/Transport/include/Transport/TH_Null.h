//#  TH_Null.h: TransportHolder that does nothing
//#
//#  Copyright (C) 2005
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

#ifndef LOFAR_TRANSPORTTH_NULL_H
#define LOFAR_TRANSPORTTH_NULL_H

// \file
// TransportHolder that does nothing

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Transport/TransportHolder.h>

namespace LOFAR 
{
    // \addtogroup Transport
    // @{

    //# Forward Declarations

    // Description of class.
    class TH_Null : public TransportHolder
    {
    public:
      TH_Null(){};
      virtual ~TH_Null(){};

      virtual bool init() {return true;};

      virtual bool recvBlocking (void*, int, int, int, DataHolder*) 
      { return true; }

      virtual bool sendBlocking (void*, int, int, DataHolder*)
      { return true; }

      virtual int32 recvNonBlocking (void*, int32, int, int32, DataHolder*)
      { return true; }

      virtual void waitForReceived(void*, int, int)
      {}

      virtual bool sendNonBlocking (void*, int, int, DataHolder*)
      { return true; }

      virtual void waitForSent(void*, int, int)
      {}

      virtual string getType() const
      { return "TH_Null"; }

      virtual bool isClonable() const
      { return true; }

      virtual TransportHolder* clone() const
      { return new TH_Null(); }

      virtual void reset()
      {}

    private:
      // Copying is not allowed
      TH_Null(const TH_Null& that);
      TH_Null& operator=(const TH_Null& that);

      //# Datamembers
    };

    // @}
} // namespace LOFAR

#endif
