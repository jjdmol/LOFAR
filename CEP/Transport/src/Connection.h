//#  Connection.h: 
//#
//#  Copyright (C) 2002-2003
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

#ifndef LIBTRANSPORT_CONNECTION_H
#define LIBTRANSPORT_CONNECTION_H

#include <lofar_config.h>
#include <Transporter.h>

// I should not have to include this, right? -- CB
#include <Common/Debug.h>

namespace LOFAR
{
  class Connection
  {
    
  public:
    
    Connection();
    
    virtual ~Connection();

    bool connectTo(Transporter* sourceTP,
		   Transporter* targetTP);
    
    bool connectFrom(Transporter* sourceTP, 
		     Transporter* targetTP);

  private:

    bool connectData(Transporter* sourceTP, 
		     Transporter* targetTP);

  };

} // namespace LOFAR

#endif
