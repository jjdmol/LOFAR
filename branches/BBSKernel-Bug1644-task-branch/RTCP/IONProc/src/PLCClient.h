//#  PLCCient.h: A client to talk to ApplController using LCS/Stream
//#
//#  Copyright (C) 2006
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
//#  $Id: BeamletBufferToComputeNode.h 17549 2011-03-11 10:34:48Z mol $

#ifndef LOFAR_IONPROC_PLCCLIENT_H
#define LOFAR_IONPROC_PLCCLIENT_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Stream/Stream.h>
#include <Thread/Thread.h>
#include <Common/LofarTypes.h>
#include <string>

namespace LOFAR {
namespace RTCP {

// LCS/ACC/PLC/include/PLC/ProcControlComm.h

// The result field that is passed from an application process is a bitmask
// representing the result of the command.<br>
// See \c resultInfo method for more information.
typedef enum { PCCmdMaskOk           = 0x0001,
               PCCmdMaskNotSupported = 0x0008,
               PCCmdMaskCommError    = 0x8000 } PcCmdResultMask;

// LCS/ACC/PLC/include/PLC/PCCmd.h

// The PCCmd enumeration is a list of command(numbers) that are used to tell
// the ProcControl server-side (= application process) what command should be
// executed.
enum PCCmd {    PCCmdNone = 0,
                PCCmdBoot = 100,  PCCmdQuit,
                PCCmdDefine,      PCCmdInit,
                PCCmdPause,       PCCmdRun,
                PCCmdRelease,
                PCCmdSnapshot,    PCCmdRecover,
                PCCmdReinit,      PCCmdParams,
                PCCmdInfo,        PCCmdAnswer,
                PCCmdReport,
                PCCmdAsync,
                PCCmdResult = 0x1000
};


class PLCClient {
public:
  PLCClient( Stream &s, const std::string &procID )
  :
    itsStream( s ),
    itsProcID( procID ),
    itsDone( false ),
    itsThread( 0 )
  {
    itsThread = new InterruptibleThread( this, &PLCClient::mainLoop, "[PLC] ", 65535 );
  }

  virtual ~PLCClient()
  {
    itsDone = true;

    delete itsThread;
  }

  bool isDone() const {
    return itsDone;
  }

  void waitForDone() {
    delete itsThread;
    itsThread = 0;
  }

  // ----- Functions to overload -----

  // signal functions are called from the PLCClient::mainLoop thread synchronously
  virtual bool define()
  {
    return true;
  }

  virtual bool init()
  {
    return true;
  }

  virtual bool run()
  {
    return true;
  }

  virtual bool pause( const double &when )
  {
    (void)when;

    return true;
  }

  virtual void quit()
  {
  }

  // return whether the observation is still running
  virtual bool observationRunning()
  {
    return false;
  }

private:
  Stream &itsStream;
  const std::string itsProcID;
  bool itsDone;
  InterruptibleThread *itsThread;

  void mainLoop();

protected:
  void sendCmd( PCCmd cmd, const std::string &options );
  void sendResult( PCCmd cmd, const std::string &options, uint16 result );
  void recvCmd( PCCmd &cmd, std::string &options ) const;
};

} // namespace RTCP
} // namespace LOFAR

#endif

