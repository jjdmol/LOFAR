//# MasterControl.h: Master control of a distributed process
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_MWCOMMON_MASTERCONTROL_H
#define LOFAR_MWCOMMON_MASTERCONTROL_H

// @file
// @brief Master control of a distributed process.
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/MWStepVisitor.h>
#include <MWCommon/MWStep.h>
#include <MWCommon/ObsDomain.h>
#include <MWCommon/WorkDomainSpec.h>
#include <MWCommon/MWConnectionSet.h>
#include <MWCommon/ParameterHandler.h>
#include <string>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Master control of a distributed process.

  // This class does the overall control of the master/worker framework.
  // It defines the basic operations (see the enum) that can be done by the
  // workers.
  //
  // Its operations are as follows:
  // <ol>
  //  <li> The \a setInitInfo function sends the basic info to all
  //       workers like the name of the VDS to be used.
  //  <li> The \a setWorkDomain function defines the work domain info
  //       in a WorkDomainSpec object.
  //  <li> The processSteps function does the actual processing.
  //       It loops over the entire observation domain in work domain chunks.
  //       For each work domain it loops over the steps to be processed.
  //       This is done by using the MasterControl as a visitor to an MWStep.
  //  <li> After all steps are processed, it sends a quit command to the
  //       workers.
  // </ol>
  // As said above, a step is processed by using the MasterControl as
  // an MWStepVisitor object. Usually step maps directly to an operation
  // and prcoessing the step simply consists of sending a single command
  // to the workers.
  // However, in case of a solve it is more ivolved.
  // It consists of sending multiple operations to localWorkers and globalWorker
  // and testing if the globalWorker has converged. This is all handled in the
  // \a visitSolve function.
  //
  // Instead of using MasterControl as the visitor, it might also be
  // possible to pass a visitor object to the MasterControl. However,
  // apart from processing the steps the MasterControl is doing hardly
  // anything at all, so it might be better to have anther XXXControl
  // class resembling this one.
  // (It might be better to rename MasterControl to BBSControl as it is
  // modeled after the BBSKernel functionality).

  class MasterControl: public MWStepVisitor
  {
  public:
    // Define the possible standard operations.
    enum Operation {
      // initialize
      Init=1,
      // set work domain
      SetWd,
      // process a step
      Step,
      // global initial info (e.g. solvable parm info)
      GlobalInit,
      // get all iteration info (e.g. normal equations)
      GlobalInfo,
      // execute a global iteration (e.g. solve)
      GlobalExec,
      // end the processing of a work domain
      EndWd
    };
    
    // Provide a descriptive string for the standard operations
    // @param op Enumeration to be described
    friend std::ostream& operator<<(std::ostream& os, MasterControl::Operation op);

    // Create the master control with the given localWorker and globalWorker
    // connections.
    MasterControl (const MWConnectionSet::ShPtr& localWorkers,
		   const MWConnectionSet::ShPtr& globalWorkers);

    ~MasterControl();

    // Set the MS name to process.
    void setInitInfo (const ParameterSet& parms,
		      const std::vector<std::string>& dataPartNames,
		      const ObsDomain&);

    // Set the work domain specification.
    void setWorkDomainSpec (const WorkDomainSpec&);

    // Process a step (which can consist of multiple steps).
    void processSteps (const MWStep&);

    // End the processing.
    void quit();

  private:
    // Process the various MWStep types.
    // @{
    virtual void visitGlobal (const MWGlobalStep&);
    virtual void visitLocal  (const MWLocalStep&);
    // @}

    // Read the result from all localWorkers and/or globalWorkers.
    // This is merely to see if the workers have performed the step.
    void readAllWorkers (bool localWorkers, bool globalWorkers);

    //# Data members.
    ObsDomain              itsFullDomain;
    WorkDomainSpec         itsWds;
    MWConnectionSet::ShPtr itsLocalWorkers;
    MWConnectionSet::ShPtr itsGlobalWorkers;
  };

}} //# end namespaces

#endif
