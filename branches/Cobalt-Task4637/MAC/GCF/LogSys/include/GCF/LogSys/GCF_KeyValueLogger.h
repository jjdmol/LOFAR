//#  GCF_KeyValueLogger.h: singleton class; bridge between controller application 
//#                    and KeyValueLoggerDaemon
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

#ifndef GCF_KEYVALUELOGGER_H
#define GCF_KEYVALUELOGGER_H

#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/GCF_Defines.h>

namespace LOFAR {
  namespace GCF {
    namespace Common {
      class GCFPValue;
    }
    namespace LogSys {


//
// GCFKeyValueLogger
//
class GCFKeyValueLogger : public TM::GCFTask
{
public:
	static GCFKeyValueLogger* instance();

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
	GCFKeyValueLogger ();
	virtual ~GCFKeyValueLogger () {}

	// state methods
	TM::GCFEvent::TResult initial     (TM::GCFEvent& e, TM::GCFPortInterface& p);
	TM::GCFEvent::TResult operational (TM::GCFEvent& e, TM::GCFPortInterface& p);

	// data members        
	TM::GCFPort 				_kvlClientPort;
	int 						_manIdToSkip;
	static GCFKeyValueLogger* 	_pInstance;

	// admin members
	typedef list<TM::GCFEvent*> TMsgQueue;
	TMsgQueue 					_msgQueue;
};

} // namespace LogSys
} // namespace GCF
} // namespace LOFAR

#define LOG_KEYVALUE_TSD(key, value, origin, timestamp, desc) \
LOFAR::GCF::LogSys::GCFKeyValueLogger::instance()->logKeyValue(key, value, origin, timestamp, desc);

#define LOG_KEYVALUE_TS(key, value, origin, timestamp) \
LOFAR::GCF::LogSys::GCFKeyValueLogger::instance()->logKeyValue(key, value, origin, timestamp);

#define LOG_KEYVALUE_D(key, value, origin, desc) \
LOFAR::GCF::LogSys::GCFKeyValueLogger::instance()->logKeyValue(key, value, origin, desc);

#define LOG_KEYVALUE(key, value, origin) \
LOFAR::GCF::LogSys::GCFKeyValueLogger::instance()->logKeyValue(key, value, origin);

#define ADD_ACTION(key, action, origin, timestamp, desc) \
LOFAR::GCF::LogSys::GCFKeyValueLogger::instance()->addAction(key, action, origin, timestamp, desc);

#define SKIP_UPDATES_FROM(manID) \
LOFAR::GCF::LogSys::GCFKeyValueLogger::instance()->skipUpdatesFrom(manID);

#endif
