//  CorbaTransportIn.h: Corba transport mechanism
//
//  Copyright (C) 2000-2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_CORBA_TRANSPORT_IN_H
#define CEPFRAME_CORBA_TRANSPORT_IN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "CEPFrame/Corba/CorbaTransport.h"
#include "CEPFrame/Corba/CorbaTransportInterface_c.hh"

class CorbaTransportIn: public CorbaTransport
{
public:
  CorbaTransportIn(int aID); 
  virtual ~CorbaTransportIn();
  
  bool getBuffer(void* bufferptr, unsigned long len);

 private:
  PortableServer::ObjectId_var     itsTransporterId;
  CorbaTransportI::Transporter_ptr itsTransporter;
};

#endif
