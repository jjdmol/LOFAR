//#  RSPConfig.h: configuration constants read from config file
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

#include "RSPConfig.h"
#include <stdlib.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
using namespace LOFAR;

using namespace RSP;

#define GET_VALUE(name, var, conv)		\
{						\
  const char* valstr = m_config(name);	\
  if (valstr) var = conv(m_config(name)lstr);		\
}

#if 0
#define DEFAULT_N_RSPBOARDS   (24)
#define DEFAULT_N_RCU         (8 * DEFAULT_N_RSPBOARDS)
#define DEFAULT_SYNC_INTERVAL (1.0)
#endif

RSPConfig* RSPConfig::m_instance = 0;

RSPConfig::RSPConfig()
#if 0
 :
  m_n_rspboards(DEFAULT_N_RSPBOARDS),
  m_n_rcu(DEFAULT_N_RCU),
  m_sync_interval(DEFAULT_SYNC_INTERVAL)
#endif
{
}

RSPConfig::~RSPConfig()
{
}

RSPConfig& RSPConfig::getInstance()
{
  if (!m_instance)
  {
    m_instance = new RSPConfig;
  }

  return *m_instance;
}

void RSPConfig::load(const char* filename)
{
  m_config.init(filename, '=', 1);
#if 0
  GET_VALUE("N_RSPBOARDS",   m_n_rspboards,   atoi);
  GET_VALUE("N_RCU",         m_n_rcu,         atoi);
  GET_VALUE("SYNC_INTERVAL", m_sync_interval, atof);
#endif
}
