//# GlobalProcessControl.h: Implementation of ACC/PLC ProcessControl class.
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_GLOBALPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_GLOBALPROCESSCONTROL_H

// \file
// Implementation of ACC/PLC ProcessControl class

//# Includes
#include <PLC/ProcessControl.h>

namespace LOFAR
{
  //# Forward Declarations.
#if 0
  class BlobStreamable;
  class DH_BlobStreamable;
  class TH_Socket;
  class CSConnection;
#endif

  namespace BBS
  {
    //# Forward Declarations.
    class BBSStrategy;
    class BBSStep;
    class CommandQueue;

    // \addtogroup BBSControl
    // @{

    // Implementation of the ProcessControl interface for the global BBS
    // controller.
    class GlobalProcessControl : public ACC::PLC::ProcessControl
    {
    public:
      // Default constructor.
      GlobalProcessControl();

      // Destructor.
      virtual ~GlobalProcessControl();

      // @name Implementation of PLC interface.
      // @{
      virtual tribool define();
      virtual tribool init();
      virtual tribool run();
      virtual tribool quit();
      virtual tribool pause(const string& condition);
      virtual tribool snapshot(const string& destination);
      virtual tribool recover(const string& source);
      virtual tribool reinit(const string& configID);
      virtual string  askInfo(const string& keylist);
      // @}

    private:
#if 0
      // Send the strategy or one of the steps across.
      bool sendObject(const BlobStreamable& bs);

      // Receive a BlobStreamable object, e.g., a BBSStatus.
      BlobStreamable* recvObject();
#endif

      // The strategy that will be executed by this controller.
      BBSStrategy*            itsStrategy;

      // Vector containing all the separate steps, in sequential order, that
      // the strategy consists of.
      vector<const BBSStep*>  itsSteps;

      // Iterator for keeping track where we left while traversing the vector
      // \c itsSteps. We need this iterator, because the run() method will be
      // invoked several times by ACCmain. In each call to run() we must
      // execute one BBSStep.
      vector<const BBSStep*>::const_iterator itsStepsIterator;

#if 0
      // DataHolder for exchanging data between global (BBS) and local
      // (BBSKernel) global control.
      DH_BlobStreamable* itsDataHolder;

      // TransportHolder used to exchange DataHolders. The global controller
      // will open a server connection, waiting for local controllers to
      // connect.
      TH_Socket* itsTransportHolder;

      // Connection between the global (BBS) control and the local
      // (BBSKernel) control.
      CSConnection* itsConnection;
#endif

      // CommandQueue where strategies and steps can be "posted".
      CommandQueue* itsCommandQueue;

      // Flag indicating whether we've sent \c itsStrategy. We only need to
      // send it once, as the very first message.
      bool itsStrategySent;
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
