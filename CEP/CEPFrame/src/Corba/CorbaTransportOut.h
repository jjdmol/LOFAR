//  CorbaTransportOut.h: Corba transport mechanism
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
//  $Log$
//  Revision 1.8  2002/06/06 14:10:06  wierenga
//  %[BugId:28]%
//  Revert to versions before change to CyclicBuffer.
//  Move from MTCircularBuffer to CyclicBuffer has been cancelled.
//
//  Revision 1.6  2002/05/22 07:04:29  wierenga
//  Renamed CircularBuffer.h to MTCircularBuffer.h to make room for new
//  circular buffer implementation.
//
//  Revision 1.5  2002/05/07 08:58:42  gvd
//  Added BUILT_SOURCES to Corba makefile.am to force idl build
//  Added BaseSim/Corba to includes of .hh files
//  Move CorbaControlClient from Corba to CorbaExe (for correct build order)
//
//  Revision 1.4  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.3  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.2  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.1  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_CORBA_TRANSPORT_OUT_H
#define BASESIM_CORBA_TRANSPORT_OUT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/Corba/CorbaTransport.h"
#include "BaseSim/Corba/CorbaTransportInterface_s.hh"
#include "BaseSim/Corba/MTCircularBuffer.h"

class CorbaTransportOut: public CorbaTransport,
                         public POA_CorbaTransportI::Transporter
{
public:

  CorbaTransportOut(PortableServer::POA_var        itsRootPOA,
		    PortableServer::POAManager_var itsPOAManager,
		    int aID); 
  virtual ~CorbaTransportOut();
  CorbaBufferType* transportBuffer(void);

  bool putBuffer(void* bufferptr, unsigned long len);

 private:
  MTCircularBuffer<CorbaBufferType*>* itsBuffers;
  CorbaBufferType* itsBuffer;
};

#endif
