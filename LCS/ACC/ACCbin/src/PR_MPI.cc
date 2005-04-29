//#  PR_MPI.cc: ProcessRule based on mpirun
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
#include<Common/LofarLogger.h>
#include<Common/lofar_fstream.h>
#include<ACC/PR_MPI.h>

namespace LOFAR {
  namespace ACC {

    map<string, PR_MPI_Group*> PR_MPI_Group::theirGroupList;

    PR_MPI_Group::PR_MPI_Group(const string& groupName)
      : itsNumberOfNodes(0),
	itsGroupName(groupName),
	itsIsRunning(false)
    {
      LOG_TRACE_OBJ_STR("PR_MPI_Group: constructor");      
    };
    PR_MPI_Group::~PR_MPI_Group()
    {
      // We should not delete the PR_MPI's, they are managed by the ProcRuler
      // we should only be deleted if the vector is empty
      itsPRList.clear();
    }

    // find a group with this name or create it
    PR_MPI_Group* PR_MPI_Group::getGroup(const string& groupName)
    {
      string index = groupName;
      map<string, PR_MPI_Group*>::iterator pos;
      pos = theirGroupList.find(index);
      if (pos==theirGroupList.end()) {
	theirGroupList[index] = new PR_MPI_Group(index);
      };
      return theirGroupList[index];
    }

    void PR_MPI_Group::signUp(PR_MPI* pr)
    {
      // its possible that a pr with this index already exists. In that case the new
      // one is clone of the old one. So forget about the old one and remember the new one
      itsPRList[pr->getName()] = pr;
      LOG_TRACE_OBJ_STR("pr added to PR_MPI_Group, we now have "<<itsPRList.size()<<" prs");
    };
    void PR_MPI_Group::signOff(PR_MPI* pr)
    {
      string index = pr->getName();
      map<string, PR_MPI*>::iterator pos;
      pos = itsPRList.find(index);
      if (pos!=itsPRList.end()) {
	// check if it is really pointing to this pr
	if (pos->second == pr) {
	  itsPRList.erase(index);
	  LOG_TRACE_OBJ_STR("pr removed from PR_MPI_Group, we now have "<<itsPRList.size()<<" prs");
	}
      };
    }
    bool PR_MPI_Group::start()
    {
      if (!itsIsRunning){
	LOG_TRACE_OBJ_STR("PR_MPI_Group: start called and not running yet");
	map<string, PR_MPI*>::iterator prPos = itsPRList.begin();
	for (; prPos!=itsPRList.end(); prPos++) {
	  // i'm not sure if the hostnames end up in the right order
	  LOG_TRACE_OBJ_STR("PR_MPI_Group: "<<prPos->first<<" : "<<*(prPos->second));
	}	

	if (itsPRList.size() == itsNumberOfNodes) {
	  LOG_TRACE_OBJ_STR ("PR_MPI_Group:start starting");

	  map<string, PR_MPI*>::iterator prPos = itsPRList.begin();
	  PR_MPI* pr = prPos->second;
	  string executable = pr->getExecName();
	  itsParamFile = pr->getParamFile();


	  // create machinefile
	  string machineFile = itsGroupName + ".machinefile";
	  ofstream mFile;
	  mFile.open(machineFile.c_str(), ofstream::out);

	  for (; prPos!=itsPRList.end(); prPos++) {
	    // i'm not sure if the hostnames end up in the right order
	    mFile << prPos->second->getNodeName() << endl;
	  }
	  
	  mFile.close();


	  // we should be able to choose scampi or mpich here
	  string startCmd = formatString("./startMPI.sh scampi %d %s %s %s %s", 
					 itsNumberOfNodes,
					 machineFile.c_str(),
					 itsGroupName.c_str(),
					 executable.c_str(),
					 itsParamFile.c_str());
	  
	  int32 result = system (startCmd.c_str());

	  if (result == 0) {
	    map<string, PR_MPI*>:: iterator it;
	    for(it=itsPRList.begin(); it!=itsPRList.end(); it++) {
	      it->second->markAsStarted();
	    }

	    itsIsRunning=true;
	  }
	} else {
	  LOG_TRACE_OBJ_STR ("PR_MPI_Group:start don't have right number of PR's");

	  itsIsRunning=false;
	};
      } else {
	LOG_TRACE_OBJ_STR ("PR_MPI_Group:start already running");
      }
      return itsIsRunning;
    }
  
    bool PR_MPI_Group::stop()
    {
      if (itsIsRunning) {
	string stopCmd  = formatString("./stopMPI.sh %s %s", 
				       itsGroupName.c_str(),
				       itsParamFile.c_str());
	
	LOG_TRACE_OBJ_STR ("PR_MPI_Group:stop " << itsGroupName);

	int32 result = system (stopCmd.c_str());

	if (result == 0) {
	  itsIsRunning = false;
	  map<string, PR_MPI*>::iterator it;
	  for(it=itsPRList.begin(); it!=itsPRList.end(); it++) {
	    it->second->markAsStopped();
	  }
	}	
      }
      return (!itsIsRunning);
    }

    PR_MPI::PR_MPI(const string&  aNodeName,
		   const string&  aProcName,
		   const string&  aExecName,
		   const string&  aParamFile,
		   const int numberOfNodes,
		   const string& groupName)
      : ProcRule(aNodeName, aProcName, aExecName, aParamFile)
    {
      LOG_TRACE_OBJ_STR ("PR_MPI: constructor");
      itsMPIGroup = PR_MPI_Group::getGroup(groupName);
      itsMPIGroup->setNumberOfNodes(numberOfNodes);
      itsMPIGroup->signUp(this);
    }
    PR_MPI::PR_MPI(const PR_MPI& other)
      : ProcRule(other)
    {
      LOG_TRACE_OBJ_STR ("PR_MPI: copy constructor");
      itsMPIGroup = other.itsMPIGroup;
      itsMPIGroup->signUp(this);
    }

    PR_MPI::~PR_MPI()
    {
      LOG_TRACE_OBJ_STR ("PR_MPI: destructor");
      itsMPIGroup->signOff(this);
    }

    // forward start and stop to the group
    bool PR_MPI::start() 	{ 
      if (itsMPIGroup != 0) {
	return itsMPIGroup->start();
      } else {
	return false;
      };
    }
    bool PR_MPI::stop() { 
      if (itsMPIGroup != 0) {
	return itsMPIGroup->stop();
      } else {
	return false;
      };
    }
    string PR_MPI::getType() const
    { return ("PR_MPI"); }
    PR_MPI* PR_MPI::clone() const
    { return (new PR_MPI(*this)); }

  } // namespace ACC
} // namespace LOFAR
