//#  GSA_WaitForAnswer.cc: 
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
#include <lofar_config.h>

#define LOFARLOGGER_SUBPACKAGE "SAL"

#include "GSA_WaitForAnswer.h"
#include "GSA_Service.h"

namespace LOFAR {
 namespace GCF {
  namespace PAL {

GSAWaitForAnswer::GSAWaitForAnswer(GSAService& service) :
  HotLinkWaitForAnswer(),
  _service(service)
{
}

void GSAWaitForAnswer::hotLinkCallBack(DpMsgAnswer& answer)
{
  _service.handleHotLink(answer, *this);  
}


void GSAWaitForAnswer::hotLinkCallBack(DpHLGroup& group)
{
  _service.handleHotLink(group, *this);
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
