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
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVDouble.h>
#include <RSP_Protocol.ph>
#include <boost/shared_ptr.hpp>
#include <map>

#include "ARATestAnswer.h"


namespace LOFAR
{

// forward declaration
class GCF::TM::GCFEvent;

namespace ARA
{
  class ARATestDriverTask : public GCF::TM::GCFTask
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
      typedef map<string,boost::shared_ptr<GCF::PAL::GCFExtPropertySet> > TPropertyMap;
    
      void addPropertySet(string scope);
      void updateETHstatus(string& propName,const GCF::Common::GCFPValue* pvalue);
      void updateAPstatus(string& propName,const GCF::Common::GCFPValue* pvalue);
      void updateBPstatus(string& propName,const GCF::Common::GCFPValue* pvalue);
      void updateRCUstatus(string& propName,const GCF::Common::GCFPValue* pvalue);
      void updateSystemStatus();
      void updateStats();
      bool isEnabled();
      GCF::TM::GCFEvent::TResult initial(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult enabled(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      void getHardwareIndexes(const string& propName,const string& scope, vector<int>& hardwareIndexes);
      
      
      static string m_taskName;
      static string m_RATestServerName;
      
      ARATestAnswer   m_answer;
      GCF::TM::GCFPort         m_RSPserver;
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


} // namespace LOFAR

#endif
