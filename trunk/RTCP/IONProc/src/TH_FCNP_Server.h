//#  TH_FCNP_Server.h: TransportHolder that implements FCNP protocol
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

#ifndef LOFAR_IONPROC_FCNP_SERVER_H
#define LOFAR_IONPROC_FCNP_SERVER_H

#if defined HAVE_FCNP && defined __PPC__

// \file
// TransportHolder that does nothing

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Transport/TransportHolder.h>
#include <vector>

namespace LOFAR {
namespace RTCP {  

class TH_FCNP_Server : public TransportHolder
{
  public:
#if 0
    static  void	     createAllTH_FCNP_Servers(unsigned nrCoresPerPset);
    static  void	     deleteAllTH_FCNP_Servers();
#endif

    virtual bool	     init();

    // if doCopy == 0, Zoid's zero-copy protocol is used; memory must be
    // obtained through __zoid_alloc and must be a multiple of 32
    virtual bool	     recvBlocking(void *ptr, int size, int doCopy, int, DataHolder *);
    virtual bool	     sendBlocking(void *ptr, int size, int doCopy, DataHolder *);

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

  //private:
    // create via createAllTH_FCNP_Servers(...)
			     TH_FCNP_Server(unsigned core);
    virtual		     ~TH_FCNP_Server();

    // Copying is not allowed
			     TH_FCNP_Server(const TH_FCNP_Server& that);
			     TH_FCNP_Server& operator=(const TH_FCNP_Server& that);

    static std::vector<TH_FCNP_Server *> theirTHs;

  private:
    unsigned		     itsCore;
};

} // namespace RTCP
} // namespace LOFAR

#endif
#endif
