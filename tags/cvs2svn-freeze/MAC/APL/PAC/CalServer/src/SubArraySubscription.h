//#  -*- mode: c++ -*-
//#  SubArraySubscription.h: class definition for the SubArraySubscription class
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

#ifndef SUBARRAYSUBSCRIPTION_H_
#define SUBARRAYSUBSCRIPTION_H_

#include <APL/RTCCommon/Observer.h>
#include <APL/CAL_Protocol/CAL_Protocol.ph>
#include <GCF/TM/GCF_Control.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_bitset.h>

namespace LOFAR {
  class RTC::Subject; // forward declaration
  namespace CAL {
    class SubArray; // forward declaration

class SubArraySubscription : public RTC::Observer
{
public:
	SubArraySubscription(SubArray*							subarray,
						 bitset<MEPHeader::N_SUBBANDS>		subbandset,
						 GCFPortInterface&					port) :
		m_subarray(subarray),
		m_subbandset(subbandset),
		m_port(port) {}

	virtual ~SubArraySubscription() {}

	/**
	 * Override Observer::update method.
	 */
	virtual void update(RTC::Subject* subject);

private:
	SubArray* m_subarray; // the subject of this observer

	bitset<MEPHeader::N_SUBBANDS>	m_subbandset; // subset of subbands which to update
	GCFPortInterface&				m_port;       // port on which to send updates
};

  }; // namespace CAL
}; // namespace LOFAR

#endif /* SUBARRAYSUBSCRIPTION_H_ */

