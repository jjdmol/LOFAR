//#  ACRequestPool.cc: one line description
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
//#  Note: this source is best read with tabstop 4.
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include<Common/LofarLogger.h>
#include<Common/lofar_fstream.h>
#include<ACC/ACRequestPool.h>

namespace LOFAR {
  namespace ACC {

ACRequestPool::ACRequestPool()
{}

ACRequestPool::~ACRequestPool()
{}

void	ACRequestPool::add    (const ACRequest&		anACR)
{

}

void	ACRequestPool::remove (const ACRequest&		anACR)
{

}


ACRequest*	ACRequestPool::find (const string&		anACRName)
{
	iterator	iter = itsPool.begin();

	while (iter != itsPool.end()) {
		// Does name match?
		if (!strcmp(anACRName.c_str(), (*iter)->itsRequester)) {
			return (*iter);
		}
		iter++;
	}

	return (0);		// No match found
}

bool	ACRequestPool::save (const string&		filename)
{
#if 0
	ofstream	oFile(filename, ios::trunc);

	oFile << itsPool.size();

	iterator	iter = itsPool.begin();
	while (iter != itsPool.end()) {
		oFile << (*iter)->itsRequester;
		oFile << (*iter)->itsNrProcs;
		oFile << (*iter)->itsActivityLevel;
		oFile << (*iter)->itsArchitecture;
		oFile << (*iter)->itsLifeTime;
		oFile << (*iter)->itsAddr;
		oFile << (*iter)->itsPort;
		iter++;
	}

	oFile.close();
#endif
	return (false);
}

bool	ACRequestPool::load (const string&		filename)
{
		
	return (false);
}




  } // namespace ACC
} // namespace LOFAR
