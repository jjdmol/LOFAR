//#  WaitForAnswer.h: Our callback class
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

#ifndef  GSA_WAITFORANSWER_H
#define  GSA_WAITFORANSWER_H

#include <HotLinkWaitForAnswer.hxx>   
#include <Common/lofar_string.h>

class DpMsgAnswer;
class DpHLGroup;

namespace LOFAR {
 namespace GCF {
  namespace PVSS {

//# forward
class PVSSservice;

/**
 * This special WaitForAnswer class implements the abstract handleAnswer 
 * methods, which translates the incoming messages to methods in the GSAService 
 * class. Incoming messages are normally responses on requests on PVSS or 
 * indications if values of subscribed properties are changed. Actually this 
 * class is the translator from SCADA termology to PVSS termology and visa 
 * versa (see table below). It also translates received valueobjects (of type 
 * PVSS<pvsstype>) to GCFPV<mactype> valueobjects and visa versa.
 * PVSS               SCADA
 * dpConnect          subscribe
 * dpDisconnect       unsubscribe
 * DpSet              set
 * dpGet              get
 * dpCreate           create
 * dpDelete           delete
 */

class GSAWaitForAnswer : public HotLinkWaitForAnswer
{
public:
    GSAWaitForAnswer (PVSSservice& service) :
		HotLinkWaitForAnswer(), _service(service) {}
    virtual ~GSAWaitForAnswer () {};
    
    void hotLinkCallBack (DpMsgAnswer& answer)
		{ _service.handleHotLink(answer, *this);   }

    const string&	getDpName () const 
		{ return (_dpName); }
    void 			setDpName (const string& dpName) 
		{ _dpName = dpName; }

protected:
    // Answer on conenct
    void hotLinkCallBack (DpHLGroup& group)
		{ _service.handleHotLink(group, *this); }

private:
    PVSSservice& 	_service;
    string			_dpName;
};                                 

  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR

#endif
