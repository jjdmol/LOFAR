//#  -*- mode: c++ -*-
//#
//#  daemonize.h: function to put a daemon process in the background
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

#ifndef DAEMONIZE_H_
#define DAEMONIZE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Function to put a daemon process in the background.
 *
 * Call this function to safely make the current process a daemon
 * detects whether we were started from init - NOTE: this test is
 * unreliable, if the parent dies VERY quickly then we will mistakenly
 * think we started from init
 * @param nofork if != 0 then do not try to fork at all
 * @returns 0 on success, -1 or errno on failure
 */
int daemonize(int nofork);

#ifdef __cplusplus
}
#endif

#endif /* DAEMONIZE_H_ */
