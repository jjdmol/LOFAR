//#  DataHolderSelector.h: one line description
//#
//#  Copyright (C) 2004
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

#ifndef ACC_DATAHOLDERSELECTOR_H
#define ACC_DATAHOLDERSELECTOR_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_vector.h>
#include <Transport/DataHolder.h>

namespace LOFAR {
  namespace ACC {

//# Forward Declarations
//class forward;


// Description of class.
class DataHolderSelector
{
	typedef vector<DataHolder*>				DHlist;
	typedef vector<DataHolder*>::iterator	iterator;
public:
	DataHolderSelector();
	~DataHolderSelector();
	DataHolderSelector(const DataHolderSelector&	that);
	DataHolderSelector& operator=(const DataHolderSelector& that);

	void add   (DataHolder*	aDataHolder);
	void remove(DataHolder*	aDataHolder) throw(Exception);
	
	DataHolder*	poll();
	DataHolder*	cleanup();

private:
	void setCurElement(uint16	aValue);

	DHlist		itsDHpool;
	uint16		itsNrElements;
	uint16		itsCurElement;
};

inline void DataHolderSelector::setCurElement(uint16	aValue)
{
	if (aValue >= itsNrElements) {				// check upper boundary
		itsCurElement = 0;
	}
	else {
		itsCurElement = aValue;
	}
}


  } // namespace ACC
} // namespace LOFAR
#endif
