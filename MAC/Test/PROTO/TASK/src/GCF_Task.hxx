//  GCFTask.hxx: handles all events for a task.
//
//  Copyright (C) 2003
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

#ifndef GCF_TASK_HXX
#define GCF_TASK_HXX

/**
 *
 *
 */

#include <CharString.hxx>
#include "GCF_Defines.hxx"
#include "GCF_Fsm.hxx"
#include <DynPtrArray.cxx>

//#include <vector>

// debug macro
#ifdef DEBUG_SIGNAL
#define F_DEBUG_SIGNAL(e, p) do { debug_signal((e), (p)); } while (0)
#define F_DEBUG_SIGNAL_I(e, p, i) do { debug_signal((e), (p), (i)); } while (0)
#else
#define F_DEBUG_SIGNAL(e, p) do { (e)=(e); (p)=(p); } while (0) // keep compiler happy
#define F_DEBUG_SIGNAL_I(e, p, i) do { (e)=(e); (p)=(p); (i) = (i)} while (0)
#endif

//using namespace std;

// forward declaration
class GCFPort;
class GCFScadaApi;

/**
 * Base class of all application tasks. This class inherits from Fsm through
 * which behaviour of the task in terms of states, events and transitions is
 * defined.
 *
 * @todo * document all code with doxygen style comments
 * @todo * remove all dependencies on ACE from the task/src/ sources.
 * @todo * introduce FASSERT and FDEBUG macros with various logging levels
 * @todo * make robust to network failures, add support for automatic
 * connection re-establishment.
 *
 */


class GCFTask : public GCFFsm
{
 public:

  /**
   * The static run method. This starts the event processing loop.
   * When multiple tasks are declared within the same binary, only
   * one call to GCFTask::run will suffice.
   * A call to this function will NEVER return. It is an error if it
   * does return.
   *
   * @note Should this then not be called GCFTask::mainloop() ?
   *
   * @code
   *
   *     class MyTask : public Task { ... }
   *
   *     int main(int,char**)
   *     {
   *         // create two tasks, a and b.
   *         MyTask a("a");
   *         MyTask b("b");
   *
   *         // start the event processing loop
   *         GCFTask::run();
   *     }
   * 
   * @endcode
   */
  void run();

  /**
   * Register the protocol. This is used for debugging. The name of each event
   * that is part of the protocol is specified. Index 0 should not be used for
   * a signal. Signal numbers should start at 1.
   */
  void registerProtocol(unsigned short protocol_id,
			const char* signal_names[],
			int nr_signals);

  /// Get the name of the task.
  const char* getName() const;

  /**
   * Initialize the task.
   * This is used for two phase construction. After using the default
   * constructor the task needs to be initialized with the init method.
   */

  /**
   * Stop the task; this results in the F_STOP signal being sent to
   * the state machine for the task.
   *
   * @note <b>NOT IMPLEMENTED YET</b>
   */
  virtual void stop();

  /// print debug string of event
  void debug_signal(const GCFEvent& e, GCFPortInterface& p, const char* info = 0);

  /// return the string representation of the specified event
  const char* evtstr(const GCFEvent& e);

  void attachScadaApi(GCFScadaApi *pScadaApi);

 protected:

  GCFTask(State initial, const char* name);
  virtual ~GCFTask();

  virtual void init() = 0;
  virtual void workProc() = 0;
  GCFScadaApi* _pScadaApi;

 private:
  GCFTask();

  typedef struct
  {
    unsigned short protID;
    const char** sigTable;
  } signal_table_entry;

  typedef DynPtrArray<signal_table_entry> signal_name_map;
  signal_name_map _signal_name_map;
  static int sortFunc(const signal_table_entry *e1, const signal_table_entry *e2);


  CharString _name;
  bool _stopped;
  

  //map<unsigned short, const char**> _signal_name_map;
  //vector<signal_name_entry> _signal_name_map;
};

#endif /* GCF_TASK_HXX */
