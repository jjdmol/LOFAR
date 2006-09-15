//#  BBSKernelProcessControl.cc: 
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
//#  $Id: 

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <BBSKernel/BBSKernelProcessControl.h>

namespace LOFAR 
{
namespace BBS 
{
    
    BBSKernelProcessControl::BBSKernelProcessControl()
    : ProcessControl()
    {
    }
    
    // Destructor
    BBSKernelProcessControl::~BBSKernelProcessControl()
    {
    }

    // \name Command to control the processes.
    // There are a dozen commands that can be sent to a application process
    // to control its flow. The return values for these command are:<br>
    // - True   - Command executed succesfully.
    // - False  - Command could not be executed.
    // 
    // @{

    // During the \c define state the process check the contents of the
    // ParameterSet it received during start-up. When everthing seems ok the
    // process constructs the communication channels for exchanging data
    // with the other processes. The connection are NOT made in the stage.
    boost::logic::tribool BBSKernelProcessControl::define()
    {
        LOG_TRACE_FLOW("define()");
        return true;
    }

    // When a process receives an \c init command it allocates the buffers it
    // needs an makes the connections with the other processes. When the
    // process succeeds in this it is ready for dataprocessing (or whatever
    // task the process has).
    boost::logic::tribool BBSKernelProcessControl::init()
    {
        LOG_TRACE_FLOW("init()");
        return true;
    }

    // During the \c run phase the process does the work it is designed for.
    // The run phase stays active until another command is send.
    boost::logic::tribool BBSKernelProcessControl::run()
    {
        LOG_TRACE_FLOW("run()");
        return true;
    }

    // With the \c pause command the process stops its run phase and starts
    // waiting for another command. The \c condition argument contains the
    // contition the process should use for ending the run phase. This
    // condition is a key-value pair that can eg. contain a timestamp or a
    // number of a datasample.
    boost::logic::tribool BBSKernelProcessControl::pause(const string& condition)
    {
        return false;
    }

    // \c Quit stops the process.
    // The process \b must call \c unregisterAtAC at ProcControlServer during 
    // the execution of this command to pass the final results to the 
    // Application Controller.
    boost::logic::tribool BBSKernelProcessControl::quit()
    {
        LOG_TRACE_FLOW("quit()");
        return true;
    }

    // With the \c snapshot command the process is instructed to save itself
    // in a database is such a way that on another moment in time it can
    // be reconstructed and can continue it task.<br>
    // The \c destination argument contains database info the process
    // must use to save itself.
    boost::logic::tribool BBSKernelProcessControl::snapshot(const string& destination)
    {
        return false;
    }

    // \c Recover reconstructs the process as it was saved some time earlier.
    // The \c source argument contains the database info the process must use
    // to find the information it needs.
    boost::logic::tribool BBSKernelProcessControl::recover(const string& source)
    {
        return false;
    }

    // With \c reinit the process receives a new parameterset that it must use
    // to reinitialize itself.
    boost::logic::tribool BBSKernelProcessControl::reinit(const string& configID)
    {
        return false;
    }
    // @}

    // Define a generic way to exchange info between client and server.
    std::string BBSKernelProcessControl::askInfo(const string& keylist)
    {
        return std::string("");
    }

} // namespace BBS
} // namespace LOFAR
