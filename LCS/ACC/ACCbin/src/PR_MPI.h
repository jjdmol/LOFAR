//#  PR_MPI.h: ProcesRule based on mpirun
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACC_PR_MPI_H
#define LOFAR_ACC_PR_MPI_H

// \file PR_MPI.h
// ProcessRule based on mpirun

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes

#include <map.h>
#include <ACC/ProcRule.h>

namespace LOFAR {
  namespace ACC {
    // \addtogroup ACC
    // @{

    // This PR is somewhat more difficult than PR_Shell
    // because mpi programs should be started with mpirun
    // mpirun should be called once for a group of processes that communicates
    // using MPI

    // PR_MPI is what the ApplicationController sees and it has the same interface
    // as the other PR's.

    // PR_MPI_Group is a collection of PR_MPI's that describes one set of mpi programs
    // the PR_MPI can find its group by calling the static PR_MPI_Group::getGroup("name");

    class PR_MPI;

    class PR_MPI_Group {
    public:
      // no constructor because the PR_MPI needs to find its group using getGroup
      ~PR_MPI_Group();

      bool start();
      bool stop();

      // the PR_MPI will call these to sign up for a group
      void signUp(PR_MPI* pr);
      void signOff(PR_MPI* pr);

      void setNumberOfNodes(int non);

      static PR_MPI_Group* getGroup(const string& groupName);

    private:
      PR_MPI_Group(const string& name);

      int itsNumberOfNodes;
      string itsGroupName;
      map<string, PR_MPI*> itsPRList;

      bool itsIsRunning;
      string itsParamFile;

      static map<string, PR_MPI_Group*> theirGroupList;
    };


    // The PR_MPI class contains all information to (over)rule a process.
    // Its known how to start and stop a process and knows its current state.
    class PR_MPI : public ProcRule {
    public:
      PR_MPI(const string& aNodeName, 
	     const string& aProcName,
	     const string& aExecName,
	     const string& aParamfile,
	     const int numberOfNodes,
	     const string& group);
      PR_MPI(const PR_MPI& other);

      ~PR_MPI();
      
      // Redefine the start and stop commands.
      virtual bool start();
      virtual bool stop();
      virtual string getType() const;
      virtual PR_MPI* clone() const;
      
      void markAsStarted();
      
    private:
      
      // Default construction not allowed
      PR_MPI();
      
      //# --- Datamembers ---
      PR_MPI_Group* itsMPIGroup;
    };


    inline void PR_MPI_Group::setNumberOfNodes(int non)
      { itsNumberOfNodes = non; };

    inline void PR_MPI::markAsStarted()
      { itsIsStarted = false;};

    // @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
