//#  -*- mode: c++ -*-
//#
//#  RawDispatch.h: dispatch raw EPA events as GCF events.
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

#ifndef RAWDISPATCH_H_
#define RAWDISPATCH_H_

#include <GCF/GCF_Control.h>

namespace EPA_Protocol
{
  class RawEvent
  {
    public:
      static GCFEvent::TResult dispatch(GCFTask& task,
					GCFPortInterface& port);
  };
};

#endif /* RAWDISPATCH_H_ */
