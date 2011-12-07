//#  -*- mode: c++ -*-
//#
//#  CalTest.h: class definition for the CalTest program
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

#ifndef CALTEST_H_
#define CALTEST_H_

#include <TestSuite/test.h>
#include <GCF/TM/GCF_Control.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPort;
  using GCF::TM::GCFPortInterface;
  namespace CAL {

      class CalTest : public GCFTask, public Test
	{
	public:
	  /**
	   * The constructor of the CalTest task.
	   * @param name The name of the task. The name is used for looking
	   * up connection establishment information using the GTMNameService and
	   * GTMTopologyService classes.
	   */
	  CalTest(string name, string arrayname, string parentname, int nantennas, int nyquistzone, uint32 rcucontrol, int subarrayid);
	  virtual ~CalTest();

	  // state methods

	  /**
	   * The initial and final state.
	   */
	  /*@{*/
	  GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);
	  GCFEvent::TResult final(GCFEvent& e, GCFPortInterface &p);
	  /*@}*/

	  /*@{*/
	  /**
	   * The test scenarios. Each state represents one test scenario.
	   * Each successful test transitions to the next test state.
	   */
	  GCFEvent::TResult test001(GCFEvent& e, GCFPortInterface &p);
	  /*@}*/

	  /**
	   * Run the tests.
	   */
	  void run();

	private:
	  // member variables

	private:
	  // ports
	  GCFPort m_server;

	  memptr_t	m_handle; // subscription handle
	  int 		m_counter1; // general purpose test counter, semantics assigned per test

	  string  m_name;        // name of the current array
	  string  m_arrayname;   // name of the new subarray
	  string  m_parentname;  // name of the parent array of the subarray
	  int     m_nantennas;   // number of antennas in the array
	  int     m_nyquistzone; // nyquistzone of interest
	  uint32  m_rcucontrol;  // value for RCU control register for RCU's of this subarray
	  int     m_subarrayid;  // array 0 is full array, 1 is odd antennas (e.g. 1,3,5,etc), 2 is even antennas (e.g. 0,2,5,etc)
	};

    };
};
     
#endif /* CALTEST_H_ */
