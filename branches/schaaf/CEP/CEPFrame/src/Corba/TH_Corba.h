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
//
//  $Log$
//  Revision 1.10  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.9  2002/03/26 09:18:35  schaaf
//  Removed unused include
//
//  Revision 1.8  2002/03/14 14:25:12  wierenga
//  Adapted to the new TransportHolder interface.
//
//  Revision 1.7  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.6  2001/11/02 11:29:48  gvd
//  Changed TH_Corba for Transport changes
//
//  Revision 1.4  2001/10/05 11:50:37  gvd
//  Added getType function
//
//  Revision 1.3  2001/09/05 08:07:26  wierenga
//  Use getTransport method instead of private itsTransport.
//  Implement new transport holder interface for prototype pattern.
//
//  Revision 1.2  2001/08/13 12:06:36  schaaf
//  Use BS_Corba for Orb and POA
//
//  Revision 1.1  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_TH_CORBA_H
#define BASESIM_TH_CORBA_H

//#include <pthread.h>
#include "BaseSim/TransportHolder.h"
#include "BaseSim/Corba/CorbaTransportOut.h"
#include "BaseSim/Corba/CorbaTransportIn.h"

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
  bool recv(void* buf, int nbytes, int source, int tag);

  /** Write the data.
      It does not really write, because the read is doing the memcpy.
      The only thing it does is setting the status.
  */
  bool send(void* buf, int nbytes, int destination, int tag);  

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
