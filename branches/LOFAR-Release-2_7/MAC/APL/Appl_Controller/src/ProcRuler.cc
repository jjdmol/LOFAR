//#  ProcRuler.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include "ProcRuler.h"

namespace LOFAR {
  namespace ACC {

ProcRuler::ProcRuler()
{}

ProcRuler::~ProcRuler()
{
	itsProcPool.clear();
}

void ProcRuler::add   (const	ProcRule&	aProcRule)
{
	LOG_TRACE_OBJ_STR("ProcRuler add:" << aProcRule.getName());

	itsProcPool.erase(aProcRule.getName());

	itsProcPool.insert(make_pair(aProcRule.getName(), aProcRule.clone()));
}

void ProcRuler::remove(const	string&	    aProcName)
{
	itsProcPool.erase(aProcName);
}

bool ProcRuler::start(const string& aProcName)
{
	iterator iter = itsProcPool.find(aProcName);
	if (iter == itsProcPool.end()) {
		return (false);
	}
	
	return (iter->second->start());
}

bool ProcRuler::stop (const string& aProcName)
{
	iterator iter = itsProcPool.find(aProcName);
	if (iter == itsProcPool.end()) {
		return (false);
	}
	
	return (iter->second->stop());
}

bool ProcRuler::startAll()
{
	bool result = true;

	iterator iter = itsProcPool.begin();
	while (iter != itsProcPool.end()) {
		result &= iter->second->start();
		++iter;
	}
	return (result);
}

bool ProcRuler::stopAll()
{
	bool result = true;

	iterator iter = itsProcPool.begin();
	while (iter != itsProcPool.end()) {
		result &= iter->second->stop();
		++iter;
	}
	return (result);
}

void ProcRuler::markAsStopped(const string& aProcName)
{
	iterator iter = itsProcPool.find(aProcName);
	if (iter != itsProcPool.end()) {
		iter->second->markAsStopped();
	}
}

// @@ TEMP
void ProcRuler::show()
{
	iterator iter = itsProcPool.begin();
	while (iter != itsProcPool.end()) {
		cout << iter->second;
		++iter;
	}
}


  } // namespace ACC
} // namespace LOFAR
