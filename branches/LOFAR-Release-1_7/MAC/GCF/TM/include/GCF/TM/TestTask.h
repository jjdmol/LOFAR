//#  TestTask.h: Program for testing GCF Tasks
//#
//#  Copyright (C) 2010-2011
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
//#  $Id: TestPC.h 11046 2008-03-21 08:39:08Z overeem $

#ifndef GCF_TM_TESTTASK_H
#define GCF_TM_TESTTASK_H

//# Common Includes
#include <queue>

//# GCF Includes
#include <GCF/TM/GCF_Control.h>

// forward declaration

namespace LOFAR {
  namespace GCF {
	namespace TM {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTask;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFPortInterface;

// Testing a task may be a nasty job especially when this task has several communication ports.
// The TestTask simplifies this process: Write a task (that is derived from TestTask) that will simulate 
// all I/O channels of the task under test (tut). Define message-sequences that prove that the tut
// works correctly and program these in the function \c startTesten \c.
// As soon as your task TRAN's to the state-machine TestFrame all defined test are executed. When the
// tut behaves as defined in the sequences the program terminates normally otherwise it ASSERTs with
// an appropriate message.
class TestTask : public GCFTask
{
public:
	virtual ~TestTask();

	// Define your test sequences here by calling the add-functions and start the sequence with soTestSuite().
	virtual void startTesten() = 0;

	// Add a send command to the sequence that sends 'event' to 'port'.
	void addSendTest(GCFEvent*	event, GCFPortInterface*	port);
	// Add a receive command to the sequence: expect message 'signal' on 'port'.
	void addRecvTest(int	signal,    GCFPortInterface*	port);
	// Add a pause to the sequence, within this period nothing may happen.
	void addPause   (double	period);
	// Start a new test. It prints the given title in a striking way to interprete the logs more easily.
	void newTest	(const string&	title);

	// execute the whole sequence stack build with the add-commands.
	void doTestSuite();

protected:
	// Contructor you have to use.
    TestTask (State initial, const string& name);
	// FSM your have to TRAN to.
   	GCFEvent::TResult TestFrame		(GCFEvent& e, GCFPortInterface& p);

private:
	void _checkTest(GCFEvent& event, GCFPortInterface& port);

	// avoid defaultconstruction and copying
	TestTask();
	TestTask(const TestTask&);
   	TestTask& operator=(const TestTask&);

	enum {	TP_MSG = 0,
			TP_SEND,
			TP_RECV,
			TP_PAUSE,
			TP_OBSERVE,
			TP_DONE
	};

	typedef struct testAction {
		int					type;
		int					signal;
		double				period;
		GCFEvent*			event;
		GCFPortInterface*	port;
		string				message;
		testAction(const string& m) : type(TP_MSG), signal(TP_MSG), event(0), port(0), message(m) {};
		testAction(GCFEvent* e, GCFPortInterface* p) : type(TP_SEND), signal(0), event(e), port(p) {};
		testAction(GCFPortInterface* p, int s) : type(TP_RECV), signal(s), event(0), port(p) {};
		testAction(int t, double w) : type(t), signal(0), period(w), event(0), port(0) {};
	} testAction;

	std::queue<testAction>	itsActionQ;
	GCFTimerPort*			itsTestTimer;			// general port for timers

};

	}; // TM
  };// GCF
};// LOFAR
#endif
