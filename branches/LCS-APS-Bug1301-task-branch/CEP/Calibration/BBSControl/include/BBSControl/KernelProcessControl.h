//#  KernelProcessControl.h:  Local (kernel) process controller
//#
//#  Copyright (C) 2002-2007
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

#ifndef LOFAR_BBSCONTROL_KERNELPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_KERNELPROCESSCONTROL_H

// \file
// Local (kernel) process controller

//# Includes
#include <BBSControl/CommandQueue.h>
#include <BBSControl/BlobStreamableConnection.h>
#include <BBSControl/CommandExecutor.h>
#include <BBSControl/SolveStep.h>

#include <Common/lofar_smartptr.h>
#include <PLC/ProcessControl.h>

namespace LOFAR
{
namespace BBS
{
// \addtogroup BBSControl
// @{

// Implementation of the ProcessControl interface for the local Kernel
// controller.
class KernelProcessControl: public ACC::PLC::ProcessControl
{
public:
    enum State
    {
        INIT,
        RUN,
        WAIT
    };

    // Constructor
    KernelProcessControl();

    // Destructor
    virtual ~KernelProcessControl();

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
    State                                   itsState;

    // Command Queue.
    shared_ptr<CommandQueue>                itsCommandQueue;

    // Connection to the global solver.
    shared_ptr<BlobStreamableConnection>    itsSolver;
    
    // Command Executor.
    scoped_ptr<CommandExecutor>             itsCommandExecutor;
};
//@}

} // namespace BBS
} // namespace LOFAR

#endif
