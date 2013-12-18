//#  CEPKeyValueLogger.h: Interface for CEP to use the KeyValueLogger
//#
//#  Copyright (C) 2002-2003
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

#ifndef CEP_KEYVALUELOGGER_H
#define CEP_KEYVALUELOGGER_H

#include <GCF/GCF_Defines.h>
#include <GCF/TM/EventPort.h>

namespace LOFAR {
  namespace GCF {
    namespace Common {
      class GCFPValue;
    }
    namespace LogSys {


//
// CEPKeyValueLogger
//
class CEPKeyValueLogger
{
public:
	CEPKeyValueLogger ();
	virtual ~CEPKeyValueLogger () {}

	// member functions
	void logKeyValue(const string& key, const Common::GCFPValue& value, 
					Common::TKVLOrigin origin, const timeval& timestamp, 
					const string& description = "");
	void logKeyValue(const string& key, const Common::GCFPValue& value, 
					Common::TKVLOrigin origin, const string& description = "");

	void addAction(const string& key, uint8 action, Common::TKVLOrigin origin, 
					timeval timestamp, const string& description = "");

	void skipUpdatesFrom(uint8 manId);         

private:
	// data members        
	TM::EventPort*				itsKVLConn;
	int 						_manIdToSkip;

	// admin members
	typedef list<TM::GCFEvent*> TMsgQueue;
	TMsgQueue 					_msgQueue;
};

} // namespace LogSys
} // namespace GCF
} // namespace LOFAR


#endif
