//#
//#  netraw.cc: change capabilities to allow raw ethernet access by non-root user.
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

#if defined(ENABLE_CAP_NET_RAW)

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/capability.h>

bool enable_cap_net_raw()
{
  // must be effectively uid root
  if (0 != geteuid() || 0 == getuid())
  {
    fprintf(stderr, "This program must be run as setuid-root.");
    return false;  /** RETURN **/
  }

  // convert string to cap_t
  cap_t newcap = cap_from_text("= cap_net_raw+ep");
  if (!newcap)
  {
    perror("cap_from_text");
    return false;         /** RETURN **/
  }
  
  // set new capabilities
  if (cap_set_proc(newcap) < 0)
  {
    cap_free(newcap);
    perror("cap_set_proc");
    return false;                    /** RETURN **/
  }

  // setuid to effective user
  setuid(getuid());
  
  // set new capabilities
  if (cap_set_proc(newcap) < 0)
  {
    cap_free(newcap);
    perror("cap_set_proc");
    return false;                    /** RETURN **/
  }
  cap_free(newcap);

  return true;
}
#endif
