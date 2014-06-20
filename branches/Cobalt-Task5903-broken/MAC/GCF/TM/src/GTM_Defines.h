//#  GTM_Defines.h: preprocessor definitions of various constants
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

#ifndef GTM_DEFINES_H
#define GTM_DEFINES_H

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

#define PARAM_PORT_PROT_TYPE "mac.ns.%s.%s.type"
#define PARAM_TCP_HOST "mac.ns.%s.%s.host"
#define PARAM_TCP_PORTNR "mac.ns.%s.%s.port"
#define PARAM_SERVER_SERVICE_NAME "mac.top.%s.%s.remoteservice"
#define PARAM_ETH_IFNAME "mac.ns.%s.%s.ifname"
#define PARAM_ETH_MACADDR "mac.ns.%s.%s.macAddr"
#define PARAM_ETH_ETHERTYPE "mac.ns.%s.%s.ethertype"

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
