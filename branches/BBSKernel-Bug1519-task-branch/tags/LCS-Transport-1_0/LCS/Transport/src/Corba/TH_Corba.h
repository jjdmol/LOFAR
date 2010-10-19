//  TH_Corba.h: Corba transport mechanism
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
////
//
//////////////////////////////////////////////////////////////////////

#ifndef LIBTRANSPORT_TH_CORBA_H
#define LIBTRANSPORT_TH_CORBA_H

#include <TransportHolder.h>
#include <Corba/CorbaTransportOut.h>
#include <Corba/CorbaTransportIn.h>

/**
   This class defines the transport mechanism between data holders
   when both data holder reside on the same node.
   It uses memcpy to transport the data.
*/

class TH_Corba: public TransportHolder
{
public:
  TH_Corba();
  virtual ~TH_Corba();

  virtual TH_Corba* make() const;

  /// Read the data.
  bool recvBlocking(void* buf, int nbytes, int tag);
  bool recvNonBlocking(void* buf, int nbytes, int tag);
  /// Wait for the data to be received
  bool waitForReceived(void* bug, int nbytes, int tag);
  
  /** Write the data.
      It does not really write, because the read is doing the memcpy.
      The only thing it does is setting the status.
  */
  bool sendBlocking(void* buf, int nbytes, int tag);  
  bool sendNonBlocking(void* buf, int nbytes, int tag);  
  /// Wait for the data to be sent
  bool waitForSent(void* bug, int nbytes, int tag);

  /// Get the type of transport.
  virtual string getType() const;

  /// Declare a TH_Corba prototype that
  /// can be used in functions requiring
  /// a TransportHolder prototype.
  static TH_Corba proto;
  static void init(int argc, char *argv[]);
  static void finalize();
  
private:
  CorbaTransportOut* itsCorbaOut;
  CorbaTransportIn*  itsCorbaIn;
};


#endif
