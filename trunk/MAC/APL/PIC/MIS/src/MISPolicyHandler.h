//#  MISPolicyHandler.h: 
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

#ifndef MISPOLICYHANDLER_H
#define MISPOLICYHANDLER_H

#include <Common/lofar_list.h>
#include <MIS_Protocol.ph>

namespace LOFAR {
 namespace GCF {
   namespace Common {
     class GCFPVInteger;
   }
 }
 namespace AMI {  

class MISDaemon;

/**
*/

class MISPolicyHandler
{
  public:
    MISPolicyHandler ();
    virtual ~MISPolicyHandler () {}
    
  public: // member functions
    void rereadPolicyFile();
    string checkDiagnose(const MISDiagnosisNotificationEvent& diag, 
                         GCF::Common::GCFPVInteger& curResStateValue);
     
  private: // helper methods
    
  private: // data members
    typedef enum
    {
      LOG,
      MANUAL,
      AUTO
    } TAction;
    
    typedef struct
    {
      string resourceNameFilter;
      string diagnosis;
      uint16 lowConf;
      uint16 highConf;
      TAction action;
    } TPolicyRule;

    typedef list<TPolicyRule> TRules;
    TRules _rules;
    
  private: // admin members
};

inline MISPolicyHandler::MISPolicyHandler()
{
  rereadPolicyFile();
}
 } // namespace AMI
} // namespace LOFAR

#endif
