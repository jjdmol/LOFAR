//#  AVTStub.h: class definition for the Virtual Telescope test stub.
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

#ifndef AVTSTUB_H_
#define AVTSTUB_H_

#include <APL/BS_Protocol/BS_Protocol.ph>
#include <Suite/test.h>

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>

namespace LOFAR {
  namespace BS {

    class AVTStub : public GCFTask, public Test {
    public:
      /**
       * The constructor of the AVTStub task.
       * @param name The name of the task. The name is used for looking
       * up connection establishment information using the GTMNameService and
       * GTMTopologyService classes.
       */
      AVTStub(string name);
      virtual ~AVTStub();

      // state methods

      /**
       * The initial state. In this state the beam_server port
       * is opened.
       */
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

      //@{
      /**
       * All the test states
       */
      GCFEvent::TResult test001(GCFEvent& e, GCFPortInterface &p);
      GCFEvent::TResult test002(GCFEvent& e, GCFPortInterface &p);
      GCFEvent::TResult test003(GCFEvent& e, GCFPortInterface &p);
      GCFEvent::TResult test004(GCFEvent& e, GCFPortInterface &p);
      GCFEvent::TResult test005(GCFEvent& e, GCFPortInterface &p);
      GCFEvent::TResult done(GCFEvent& e, GCFPortInterface &p);
      //@}

      /**
       * The test run method. This should start the task
       */
      void run();

    private:
      // member variables

    private:
      // ports
      GCFPort       beam_server;

    };

  };
};
     
#endif /* AVTSTUB_H_ */
