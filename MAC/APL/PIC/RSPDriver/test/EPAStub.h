//#  -*- mode: c++ -*-
//#
//#  EPAStub.h: class definition for the EPA stub task
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

#ifndef EPASTUB_H_
#define EPASTUB_H_

#include <Suite/test.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>

namespace LOFAR {
  namespace RSP_Test {

    class EPAStub : public GCFTask, public Test
    {
    public:
      /**
       * The constructor of the EPAStub task.
       * @param name The name of the task. The name is used for looking
       * up connection establishment information using the GTMNameService and
       * GTMTopologyService classes.
       */
      EPAStub(string name);
      virtual ~EPAStub();

      // state methods

      /**
       * The initial and final state.
       */
      /*@{*/
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);
      GCFEvent::TResult final(GCFEvent& e, GCFPortInterface &p);
      /*@}*/

      /**
       * The stub states.
       */
      GCFEvent::TResult connected(GCFEvent& event, GCFPortInterface& port);

      /*@{*/
      /**
       * Request handlers.
       */
      GCFEvent::TResult read_rsr_status(EPAReadEvent& event, GCFPortInterface& port);
      GCFEvent::TResult read_rsr_version(EPAReadEvent& event, GCFPortInterface& port);
      GCFEvent::TResult read_stats(EPAReadEvent& event, GCFPortInterface& port);
      /*@}*/

      /**
       * Run the tests.
       */
      void run();

    private:
      // member variables

    private:
      // ports
      GCFETHRawPort m_client;

      // lookup for register pointers
      typedef struct
      {
	char*  addr;
	uint16 size;
      } reginfo;
      
      reginfo m_reg[MEPHeader::MAX_PID + 1][MEPHeader::MAX_REGID + 1];
    };

  };
};
     
#endif /* EPASTUB_H_ */
