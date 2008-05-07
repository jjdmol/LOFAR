//#  SolverProcessControl.h: Implementation of the ProcessControl
//#     interface for the BBS solver component.
//#
//#  Copyright (C) 2004
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

#ifndef LOFAR_BBSCONTROL_SOLVERPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_SOLVERPROCESSCONTROL_H

// \file
// Implementation of the ProcessControl interface for the BBS solver component.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <BBSControl/DomainRegistrationRequest.h>
#include <BBSControl/IterationRequest.h>
#include <BBSControl/IterationResult.h>
#include <BBSControl/BlobStreamableVector.h>
#include <BBSControl/BlobStreamableConnection.h>
#include <BBSControl/CommandQueue.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/KernelGroup.h>

#include <Common/lofar_smartptr.h>
#include <PLC/ProcessControl.h>
#include <APS/ParameterSet.h>

#include <scimath/Fitting/LSQFit.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    // Implementation of the ProcessControl interface for the BBS solver
    // component.
    class SolverProcessControl: public ACC::PLC::ProcessControl
    {
    public:
      // Default constructor.
      SolverProcessControl();

      // Destructor.
      virtual ~SolverProcessControl();

      // @name Implementation of PLC interface.
      // @{
      virtual tribool define();
      virtual tribool init();
      virtual tribool run();
      virtual tribool pause(const string& condition);
      virtual tribool release();
      virtual tribool quit();
      virtual tribool snapshot(const string& destination);
      virtual tribool recover(const string& source);
      virtual tribool reinit(const string& configID);
      virtual string  askInfo(const string& keylist);
      // @}

    private:
      struct Domain
      {
        uint32          index;
        vector<double>  unknowns;
        casa::LSQFit    solver;
      };

      enum RunState {
        UNDEFINED = 0,
        IDLE,
        SOLVING,
        N_States
      };

      // Returns whether the current solver control process controls a global
      // solver or not. 
      // \note For the time being we will assume that, when the number of
      // kernels that will connect to the solver control process is more than
      // one, the solver control process controls a global solver.
      bool isGlobal() const;

      // Set kernel groups. The argument \a groups should be interpreted as
      // follows:
      // - The size of the vector determines the number of groups.
      // - The sum of the elements determines the total number of kernels.
      // - Each element determines the number of kernels per group.
      void setKernelGroups(const vector<uint>& groups);

      // Return a reference to the kernel group of the kernel with id \a
      // kernelId.
      KernelGroup& kernelGroup(const KernelId& id);


#if 0
      struct Kernel
      {
        KernelId id;
        KernelGroupId groupId;
        shared_ptr<BlobStreamableConnection> connection;
      };

      // A kernel group is a collection of kernels that connect to the same
      // solver. The state of the group as a whole depends on the state of
      // each individual kernel in the group.
      struct KernelGroup
      {
        shared_ptr<SolverState> state;
        vector<Kernel> kernels;
      };

      class SolverState
      {
      public:
        virtual void handle() = 0;
      protected:
        shared_ptr<Message> recvMessage() const;
        void setState(const KernelGroup&);
      };

      class SolverIndexing : public SolverState
      {
      public:
        virtual void handle();
      };

      class SolverInitializing : public SolverState
      {
      public:
        virtual void handle();
      };

      class SolverIterating : public SolverState
      {
      public:
        virtual void handle();
      };
#endif

//       enum SolverState {
//         UNDEFINED = 0,
//         INDEXING,
//         INITIALZING,
//         ITERATING,
//         N_States
//       };

      // (Run) state of the solver control process
      RunState itsState;

      // Number of kernels that will connect to this solver control process.
      uint itsNrKernels;

      // Connection to the command queue.
      scoped_ptr<CommandQueue> itsCommandQueue;

      bool dispatch(const BlobStreamable *message);
      bool handle(const DomainRegistrationRequest *request);
      bool handle(const BlobStreamableVector<DomainRegistrationRequest> *request);
      bool handle(const IterationRequest *request) const;
      bool handle(const BlobStreamableVector<IterationRequest> *request) const;

      bool registerDomain(const DomainRegistrationRequest *request);
      IterationResult *performIteration(const IterationRequest *request);

      // Vector of kernels.
      vector< shared_ptr<BlobStreamableConnection> > itsKernels;
//       vector<Kernel> itsKernels;
      
      // Container of kernel groups. 
      vector<KernelGroup> itsKernelGroups;

      // Vector used for look-up of kernel group-id given a kernel-id. This is
      // just one way of solving the look-up. It's not the most memory
      // efficient, but that's not really an issue, since the number of
      // kernels is not large. Besides, this type of look-up is constant time.
      vector<uint> itsKernelGroupIds;

      // Current solve command. We need to keep track of it, because we need
      // the information in it between different calls to the run() method.
      shared_ptr<const SolveStep> itsSolveStep;
      
//       // The Message handler is responsible for handling the messages coming
//       // from the kernel.
//       KernelMessageHandler itsMessageHandler;

      map<uint32, Domain> itsRegisteredDomains;
    };
    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
