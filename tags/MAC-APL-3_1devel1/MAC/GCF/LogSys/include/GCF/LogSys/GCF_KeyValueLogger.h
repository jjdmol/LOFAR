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

namespace LOFAR 
{
 namespace GCF 
 {
  namespace Common
  {
class GCFPValue;
  }
  namespace LogSys 
  {

/**
*/

class GCFKeyValueLogger : public TM::GCFTask
{
  public:
    static GCFKeyValueLogger* instance();

  public: // member functions
    void logKeyValue(const string key, const Common::GCFPValue& value, Common::TKVLOrigin origin, const timeval& timestamp);
    void logKeyValue(const string key, const Common::GCFPValue& value, Common::TKVLOrigin origin);
  
  private:
    GCFKeyValueLogger ();
    virtual ~GCFKeyValueLogger () {}

  private: // state methods
    TM::GCFEvent::TResult initial     (TM::GCFEvent& e, TM::GCFPortInterface& p);
    TM::GCFEvent::TResult operational (TM::GCFEvent& e, TM::GCFPortInterface& p);
        
  private: // helper methods

  private: // data members        
    TM::GCFPort _kvlClientPort;
    static GCFKeyValueLogger* _pInstance;

  private: // admin members
    typedef list<TM::GCFEvent*> TLogQueue;
    TLogQueue _logQueue;
};
  } // namespace LogSys
 } // namespace GCF
} // namespace LOFAR

#define LOG_KEYVALUE(key, value, origin, timestamp) \
  LOFAR::GCF::LogSys::GCFKeyValueLogger::instance()->logKeyValue(key, value, origin, timestamp);
  
#define LOG_KEYVALUE_DT(key, value, origin) \
  LOFAR::GCF::LogSys::GCFKeyValueLogger::instance()->logKeyValue(key, value, origin);

#endif
