//#  Cancellation.cc:
//#
//#  Copyright (C) 2009
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
//#  $Id: Thread.h 16592 2010-10-22 13:04:23Z mol $

#include <lofar_config.h>
#include <Common/Thread/Cancellation.h>

namespace LOFAR {

#ifdef USE_THREADS

pthread_mutex_t Cancellation::mutex = PTHREAD_MUTEX_INITIALIZER;

Cancellation::thread_states_t& Cancellation::getThreadStates() {
  // Cancellation needs to be available _always_. This means that we
  // a) need to construct it on demand, to avoid race conditions during global static initialisation
  // b) need to leak it, to avoid race conditions during global static destruction
  static thread_states_t *thread_states = new thread_states_t;

  return *thread_states;
}

#endif

} // namespace LOFAR

