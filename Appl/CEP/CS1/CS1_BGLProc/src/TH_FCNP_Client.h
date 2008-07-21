//#  TH_FCNP_Client.h: TransportHolder that implements FCNP protocol
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

#ifndef LOFAR_TRANSPORTTH_FCNP_CLIENT_H
#define LOFAR_TRANSPORTTH_FCNP_CLIENT_H

#if defined HAVE_FCNP && defined HAVE_BGP

// \file
// TransportHolder that does nothing

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Transport/TransportHolder.h>

namespace LOFAR {
namespace CS1 {  

class TH_FCNP_Client : public TransportHolder
{
  public:
  			     TH_FCNP_Client() {}
    virtual		     ~TH_FCNP_Client();


    virtual bool	     init();

    virtual bool	     recvBlocking(void *ptr, int size, int unaligned, int, DataHolder *);
    virtual bool	     sendBlocking(void *ptr, int size, int unaligned, DataHolder *);

    static  void	     sendCompleted(void *ptr, void *arg);

    // functions below are not supported
    virtual int32	     recvNonBlocking(void *, int32, int, int32, DataHolder *);
    virtual void	     waitForReceived(void *, int, int);
    virtual bool	     sendNonBlocking (void *, int, int, DataHolder *);
    virtual void	     waitForSent(void *, int, int);
    virtual string	     getType() const;
    virtual bool	     isClonable() const;
    virtual TransportHolder* clone() const;
    virtual void	     reset();

    // Copying is not allowed
			     TH_FCNP_Client(const TH_FCNP_Client& that);
			     TH_FCNP_Client& operator=(const TH_FCNP_Client& that);
};

} // namespace CS1
} // namespace LOFAR

#endif
#endif
