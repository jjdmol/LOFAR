//#  ARATestDriverTask.h: Automatic test of the RegisterAccess application
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

#ifndef ARATestDriverTask_H
#define ARATestDriverTask_H

//# Includes
//# Common Includes

//# GCF Includes
#include <GCF/TM/GCF_Task.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <RSP_Protocol.ph>
#include <boost/shared_ptr.hpp>
#include <map>

#include "ARATestAnswer.h"

// forward declaration
class GCFEvent;
class GCFPVUnsigned;
class GCFPVBool;
class GCFPVDouble;

namespace ARA
{
  class ARATestDriverTask : public GCFTask
  {
    public:
      ARATestDriverTask();
      virtual ~ARATestDriverTask();
  
    protected:
      // protected copy constructor
      ARATestDriverTask(const ARATestDriverTask&);
      // protected assignment operator
      ARATestDriverTask& operator=(const ARATestDriverTask&);
      
    private: 
      typedef map<string,boost::shared_ptr<GCFExtPropertySet> > TPropertyMap;
    
      void addPropertySet(string scope);
      void updateETHstatus(string& propName,const GCFPValue* pvalue);
      void updateAPstatus(string& propName,const GCFPValue* pvalue);
      void updateBPstatus(string& propName,const GCFPValue* pvalue);
      void updateRCUstatus(string& propName,const GCFPValue* pvalue);
      void updateSystemStatus();
      void updateStats();
      bool isEnabled();
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface& p);
      
      
      static string m_taskName;
      static string m_RATestServerName;
      
      ARATestAnswer   m_answer;
      GCFPort         m_RSPserver;
      TPropertyMap    m_propMap;
      
      RSP_Protocol::SystemStatus m_systemStatus;
      RSP_Protocol::Statistics   m_stats;
      
      double          m_substatusPeriod;
      double          m_substatsPeriod;
      long            m_updStatusTimerId;
      long            m_updStatsTimerId;
      uint32          m_updStatsHandleSP;
      uint32          m_updStatsHandleSM;
      uint32          m_updStatsHandleBP;
      uint32          m_updStatsHandleBM;
      int             n_racks;
      int             n_subracks_per_rack;
      int             n_boards_per_subrack;
      int             n_aps_per_board;
      int             n_rcus_per_ap;
      int             n_rcus;
  };  
};

#endif
