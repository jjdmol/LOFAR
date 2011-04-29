//#  Cancellation.h:
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

#ifndef LOFAR_LCS_THREAD_CANCELLATION_H
#define LOFAR_LCS_THREAD_CANCELLATION_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <pthread.h>

namespace LOFAR {


class Cancellation {
public:
  static bool set( bool enable ) {
    int oldState;

    pthread_setcanceltype( enable ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE, &oldState );

    return oldState == PTHREAD_CANCEL_ENABLE;
  }

  static bool disable() {
    return set( false );
  }

  static bool enable() {
    return set( true );
  }
};


class ScopedDelayCancellation {
public:
  ScopedDelayCancellation() {
   itsOldState = Cancellation::disable();
  };

  ~ScopedDelayCancellation() {
    Cancellation::set( itsOldState );
  }
private:
  bool itsOldState;
};



} // namespace LOFAR

#endif
