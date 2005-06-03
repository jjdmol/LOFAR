//#  -*- mode: c++ -*-
//#  ACMProxy.h: class definition for the ACMProxy task.
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

#ifndef ACMPROXY_H_
#define ACMPROXY_H_

#include "ACC.h"

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace CAL {

    class ACMProxy : public GCFTask
    {
    public:
      /**
       * The constructor of the ACMProxy task.
       * @param name The name of the task. The name is used for looking
       * up connection establishment information using the GTMNameService and
       * GTMTopologyService classes.
       */
      ACMProxy(string name, ACCs& accs);
      virtual ~ACMProxy();

      /**
       * All necessary connections established?
       */
      bool isEnabled();

      /*@{*/
      /**
       * States
       */
      GCFEvent::TResult initial  (GCFEvent& e, GCFPortInterface& port);
      GCFEvent::TResult idle     (GCFEvent& e, GCFPortInterface& port);
      GCFEvent::TResult receiving(GCFEvent& e, GCFPortInterface& port);
      /*@}*/

      /*@{*/
      /**
       * Handle the CAL_Protocol requests
       */
      GCFEvent::TResult handle_acm_acm (GCFEvent& e, GCFPortInterface &port);
      GCFEvent::TResult handle_acm_done(GCFEvent& e, GCFPortInterface &port);
      /*@}*/

    private:
      ACCs& m_accs; // pointer to ACC buffer collection (front and back)

      /**
       * Port to the ACMServer.
       */
      GCFPort m_acmserver; // connection to the ACM server
    };

  }; // namespace CAL
}; // namespace LOFAR
     
#endif /* ACMPROXY_H_ */
