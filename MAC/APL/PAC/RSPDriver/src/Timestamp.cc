//#  Timestamp.h: implementation of the Timestamp class
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

#include "Timestamp.h"

#include <math.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace RSP_Protocol;
using namespace std;

void Timestamp::setNow(double delay)
{
  (void)gettimeofday(&m_tv, 0);
  m_tv.tv_sec += (long)trunc(delay);

  /**
   * For future use it may be required to have higher
   * precision than seconds.
   */
#if 0
  m_tv.tv_usec = 0;
#else
  m_tv.tv_usec += (int)(10e6 * (delay - trunc(delay)));
  if (m_tv.tv_usec > (int)(10e6))
  {
      m_tv.tv_usec -= (int)(10e6);
      m_tv.tv_sec++;
  }
#endif
}

