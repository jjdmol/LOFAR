//
//  tPAPerformance.h: Test program to the the performance of the PA stuff.
//
//  Copyright (C) 2007
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

#ifndef _TPAPERFORMANCE_H_
#define _TPAPERFORMANCE_H_

#include <boost/shared_ptr.hpp>
#include "GSA_Service.h"		// NOTE: THIS A COPY FROM PAL/SAL/src !!
#include <Common/lofar_vector.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/PAL/GCF_PropertySetAnswerHandlerInterface.h>
#include <GCF/PAL/GCF_PropertySetAnswer.h>
#include <GCF/PAL/GCF_MyPropertySet.h>
#include "PerformanceService.h"

namespace LOFAR {
  namespace GCF {
	namespace PAL {

class tPAPerformance : public TM::GCFTask, GCFPropertySetAnswerHandlerInterface
{
public:

  tPAPerformance (const string& name);
  ~tPAPerformance();

  void handlePropertySetAnswer(GCFEvent& answer);
  GCFEvent::TResult initial  	 (GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult final    	 (GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1cleanup (GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1create	 (GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1setvalue(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1getvalue(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test1delete	 (GCFEvent& e, GCFPortInterface& p);

private:
//   	typedef boost::shared_ptr<GCF::PAL::GCFMyPropertySet> GCFMyPropertySetPtr;
   	typedef GCF::PAL::GCFMyPropertySet*	 GCFMyPropertySetPtr;

	PerformanceService*			itsCoreService;
	GCFPropertySetAnswer		itsPSA;
	vector<GCFMyPropertySetPtr>	itsPropSetVector;
	TM::GCFTimerPort*			itsTimerPort;
};

   } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
