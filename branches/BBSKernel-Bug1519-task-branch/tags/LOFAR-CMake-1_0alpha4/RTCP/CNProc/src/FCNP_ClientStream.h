//#  FCNP_ClientStream.h: Stream that implements FCNP protocol
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

#ifndef LOFAR_CNPROC_FCNP_CLIENT_STREAM_H
#define LOFAR_CNPROC_FCNP_CLIENT_STREAM_H

#if defined HAVE_FCNP && defined HAVE_BGP_CN

#include <Stream/Stream.h>

namespace LOFAR {
namespace RTCP {  

class FCNP_ClientStream : public Stream
{
  public:
    virtual	 ~FCNP_ClientStream();

    virtual void read(void *ptr, size_t size);
    virtual void write(const void *ptr, size_t size);
};

} // namespace RTCP
} // namespace LOFAR

#endif
#endif
