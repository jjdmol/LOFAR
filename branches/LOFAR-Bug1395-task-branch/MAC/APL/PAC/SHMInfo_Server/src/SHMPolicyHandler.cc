//#  SHMPolicyHandler.cc: 
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

#include <lofar_config.h>
#include <Common/lofar_fstream.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>

//#include <GCF/LogSys/GCF_KeyValueLogger.h>
//#include <GCF/GCF_PVInteger.h>
//#include <APL/APLCommon/APL_Defines.h>
#include "SHMInfoServer.h"
#include "SHM_Protocol.ph"
#include "SHMDefines.h"
#include "fnmatch.h"

using std::ifstream;

#define ASCII_LINE_SIZE 1024

namespace LOFAR 
{
using namespace GCF::Common;
//using namespace APLCommon;

 namespace AMI
 {

string trim(string source)
{  
  string::size_type pos = source.find_first_not_of(" \t");
  if (pos != string::npos) {
    source.erase(0, pos);
  }
  pos = source.find_first_of(" \t");
  if (pos != string::npos) {
    source.erase(pos);
  }
  return source;
}

void SHMPolicyHandler::rereadPolicyFile()
{
	ifstream    policyRulesFile;

	// Try to open the policy rules file  
	ConfigLocator		aCL;
	string				policyFilename(aCL.locate("mis.pol"));
	LOG_DEBUG_STR("Using policy file: " << policyFilename);
	policyRulesFile.open(policyFilename.c_str(), ifstream::in);

	if (!policyRulesFile) {
		LOG_ERROR("File 'mis.pol' with policy rules could not be opend. No rules (re)loaded!");
		return;
	}

	char asciiLine[ASCII_LINE_SIZE];

	_rules.clear();

	// Read the file line by line and convert to rules.
	uint16 lineNr(0);
	while (policyRulesFile.getline (asciiLine, ASCII_LINE_SIZE)) {
		lineNr++;
		if (strlen(asciiLine) == 0) { // empty row -> skip
			continue;
		}
		else if (asciiLine[0] == '#') { // comment row -> skip
			continue;
		}
		else if (strchr(asciiLine, '|') > 0) {
			vector<string> ruleElements = StringUtil::split(asciiLine, '|');
			int confFound;
			if (ruleElements.size() != 5) {
				LOG_ERROR(formatString( "Line %d: A policy rule row should have five colums.", lineNr));
				continue;      
			}
			TPolicyRule newRule;
			newRule.resourceNameFilter = trim(ruleElements[0]);
			newRule.diagnosis = trim(ruleElements[1]);
			if (newRule.diagnosis != "FAULTY" && newRule.diagnosis != "HEALTHY") {
				LOG_ERROR(formatString(
							"Line %d: \"%s\" not a possible diagnosis. Chose from (FAULTY | HEALTHY). Skipped!",
							lineNr,
							newRule.diagnosis.c_str()));              
				continue;
			}
			confFound = sscanf(ruleElements[2].c_str(), "%hd", &newRule.lowConf);
			if (confFound < 1) {
				newRule.lowConf = 0;
			}
			confFound = sscanf(ruleElements[3].c_str(), "%hd", &newRule.highConf);
			if (confFound < 1) {
				newRule.highConf = 0xFFFF;
			}
			if (newRule.lowConf > newRule.highConf) {
				LOG_ERROR(formatString(
							"Line %d: Lowest confidence level (%d) is higher then highest confidence level (%d)!. Skipped!",
							lineNr,
							newRule.lowConf,
							newRule.highConf));
				continue;
			}
			if (trim(ruleElements[4]) == "LOG") {
				newRule.action = LOG;          
			}
			else if (trim(ruleElements[4]) == "MANUAL" && newRule.diagnosis == "FAULTY") {
				newRule.action = MANUAL;          
			}
			else if (trim(ruleElements[4]) == "AUTO") {
				newRule.action = AUTO;          
			}
			else {
				LOG_ERROR(formatString(
							"Line %d: \"%s\" not a possible action. Choose from (LOG | MANUAL | AUTO). Skipped!",
							lineNr,
							trim(ruleElements[4]).c_str()));
				continue;
			}
			_rules.push_back(newRule);
		}    
	} // while
	policyRulesFile.close();  
	LOG_DEBUG(formatString(
		"Reading policy file ready. It contains %d valid rules on %d lines.",
		_rules.size(),
		lineNr));
}

#if 0
string SHMPolicyHandler::checkDiagnose(const SHMDiagnosisNotificationEvent& diag, 
                                       GCFPVInteger& curResStateValue)
{
  LOG_INFO(formatString(
      "Diagnose: %s, %s, %d, %s",
      diag.component.c_str(),
      diag.diagnosis.c_str(),
      diag.confidence,
      diag.diagnosis_id.c_str()));
  string response("NAK (No rule matches with given diagnosis.)");
  TPolicyRule curRule;
  if (!IS_IDLE(curResStateValue.getValue()) && 
      !IS_BUSY(curResStateValue.getValue()) && 
      diag.diagnosis.find("FAULTY") == 0)
  {
    response = formatString("NAK (Component has the wrong resource state (=%d) for given diagnosis!)", 
               curResStateValue.getValue());
  }
  if (!IS_SUSPECT(curResStateValue.getValue()) && 
      diag.diagnosis.find("HEALTHY") == 0)
  {
    response = formatString("NAK (Component has the wrong resource state (=%d) for given diagnosis!)", 
               curResStateValue.getValue());
  }

  for (TRules::iterator iter = _rules.begin(); iter != _rules.end(); ++iter)
  {    
    curRule = *iter;
    
    if (curRule.lowConf > diag.confidence)
    {      
    }
    else if (curRule.highConf < diag.confidence)
    {
    }
    else if (diag.diagnosis.find(curRule.diagnosis) == string::npos)
    {
    }
    else
    {
      if (fnmatch(curRule.resourceNameFilter.c_str(), diag.component.c_str(), FNM_NOESCAPE) != 0)
      {
        continue;
      }
      // bingo this rule applies to the diagnosis
      int32 curState = curResStateValue.getValue();
      response = "ACK";
      switch (curRule.action)
      {
        case LOG:
        {
          LOG_INFO("Rule found. Operator has specified 'LOG' action in the found rule.");
          response = "NAK (ignored by policy)";
          timeval ts = {diag.timestamp_sec, diag.timestamp_nsec / 1000};
          string descr(formatString (
              "%s(cl:%d, url:%d)",
              diag.diagnosis.c_str(),
              diag.confidence,
              diag.diagnosis_id.c_str()));          

          // 1 means ignore action state
          ADD_ACTION(diag.component, 1, KVL_ORIGIN_SHM, ts, descr);
          break;
        }
        case MANUAL:
          LOG_INFO("Rule found. Operator has specified 'MANUAL' action in the found rule.");
          MAKE_SUSPECT(curState);
          break;
          
        case AUTO:
          LOG_INFO("Rule found. Operator has specified 'AUTO' action in the found rule.");
          if (curRule.diagnosis == "FAULTY")
          {
            curState = RS_DEFECT;
          }
          else
          {
            MAKE_UNSUSPECT(curState);
          }
          break;
      }
      curResStateValue.setValue(curState);            
      break;
    }
  }
  
  return response;   
}
#endif

 } // namespace AMI
} // namespace LOFAR
