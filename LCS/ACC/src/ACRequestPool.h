//#  ACRequestPool.h: small structure used for comm. with ACDaemon
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
//#  Note: This source is best read with tabstop 4.
//#
//#  $Id$

#ifndef ACC_ACREQUESTPOOL_H
#define ACC_ACREQUESTPOOL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_list.h>
#include <ACC/ACRequest.h>

namespace LOFAR {
  namespace ACC {

// The ACRequestPool is internally used by the ACDeamon to manage the
// resources he owns.
class ACRequestPool
{
public:
	typedef list<ACRequest*>::iterator			iterator;
	typedef list<ACRequest*>::const_iterator	const_iterator;

	ACRequestPool();
	~ACRequestPool();

	// Element maintenance
	void	add    (const ACRequest&		anACR);
	void	remove (const ACRequest&		anACR);

	ACRequest*	find (const string&		anACRName);

	// Store and retrieve whole pool to/from a file.
	bool	save (const string&		filename);
	bool	load (const string&		filename);
	
private:
	// Copying is not allowed
	ACRequestPool(const ACRequestPool&	that);
	ACRequestPool& operator=(const ACRequestPool& that);
	
	list<ACRequest*>		itsPool;
};

  } // namespace ACC
} // namespace LOFAR

#endif
