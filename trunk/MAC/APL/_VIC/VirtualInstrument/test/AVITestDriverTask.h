//#  AVITestDriverTask.h: Automatic test of the RegisterAccess application
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

#ifndef AVITestDriverTask_H
#define AVITestDriverTask_H

//# Includes
//# Common Includes

//# GCF Includes
#include <GCF/TM/GCF_Task.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <RSP_Protocol.ph>
#include <boost/shared_ptr.hpp>
#include <map>

#include "AVITestAnswer.h"

// forward declaration
class GCFEvent;
class GCFPVUnsigned;
class GCFPVBool;
class GCFPVDouble;


namespace LOFAR
{

namespace AVI
{
  class AVITestDriverTask : public GCFTask
  {
    public:
      AVITestDriverTask();
      virtual ~AVITestDriverTask();
  
    protected:
      // protected copy constructor
      AVITestDriverTask(const AVITestDriverTask&);
      // protected assignment operator
      AVITestDriverTask& operator=(const AVITestDriverTask&);
      
    private: 
      bool isEnabled();
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface& p);
      
      static string m_taskName;
      
      AVITestAnswer     m_answer;
      GCFExtPropertySet m_extPropSetVISD;
      GCFExtPropertySet m_extPropSetVI1;
  };  
};


} // namespace LOFAR

#endif
