//#  DataHolderSelector.cc: Enables a poll on a collection of dataholders.
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
#include <ACC/DataHolderSelector.h>
#include <Transport/TH_Socket.h>

namespace LOFAR {
  namespace ACC {

DataHolderSelector::DataHolderSelector()
{}

DataHolderSelector::~DataHolderSelector()
{}

DataHolderSelector::DataHolderSelector(const DataHolderSelector&	that)
{
	operator= (that);
}

DataHolderSelector& DataHolderSelector::operator=(const DataHolderSelector& that)
{
	if (this != &that) {
		itsDHpool     = that.itsDHpool;
		itsNrElements = that.itsNrElements;
		itsCurElement = that.itsCurElement;
	}

	return (*this);
}

void DataHolderSelector::add   (DataHolder*	aDataHolder)
{
	itsDHpool.insert (itsDHpool.begin(), aDataHolder);
	++itsNrElements;
}

void DataHolderSelector::remove(DataHolder*	aDataHolder) throw(Exception)
{
	iterator	iter = itsDHpool.begin();

	while (iter != itsDHpool.end()) {				// search dataholder
		if (*iter == aDataHolder) {
			itsDHpool.erase(iter);					// remove from pool
			--itsNrElements;						// update size
			setCurElement(itsCurElement);			// boundary check
			return;									// ready
		}
		++iter;
	}
	THROW(Exception, "DataHolder " << aDataHolder << "not in pool");
}

	
// loop over all dataholder to see if data is ready.
// Start scan where we stopped last time.
DataHolder*	DataHolderSelector::poll()
{
	for (int i = itsCurElement; i < itsNrElements; ++i) {
		cout << "poll at " << i << endl;
		if (itsDHpool.at(i)->read()) {
			setCurElement (++i);
			return (itsDHpool.at(i));
		}
	}
	itsCurElement = 0;
	
	return (0);
}

// cleanup DH's that lost connection
// When one is found the DH it removed from the pool, the search is stopped
// and the DataHolder is returned to the user to do additional cleanup.
// !
// ! Unfortunately the TransportHolder does not have a function 'isConnected'
// ! so we have to specialize this class with TH_Socket knowledge. 
// !
DataHolder* 	DataHolderSelector::cleanup()
{
	for (int i = 0; i < itsNrElements; ++i) {
		DataHolder*	aDH = itsDHpool.at(i);
		TH_Socket*	aTHS = dynamic_cast<TH_Socket*>
								(aDH->getTransporter().getTransportHolder());
		if (!aTHS->getDataSocket()) {		// Socket was removed?
			LOG_DEBUG_STR("DataHolderSelector:cleanup " << i);
			remove(aDH);					// remove it from the pool also
			return(aDH);					// let user to other cleanup
		}
	}
	return (0);								// nothing deleted.
}


  } // namespace ACC
} // namespace LOFAR
