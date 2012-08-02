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
#include <Common/LofarTypes.h>
#include <Interface/SmartPtr.h>
#include <Stream/Stream.h>
#include <Common/Thread/Thread.h>
#include <Common/Thread/Condition.h>
#include <Common/Thread/Mutex.h>

#include <string>
#include <time.h>

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

class PLCRunnable {
public:
  virtual ~PLCRunnable() {}

  virtual bool define() = 0;
  virtual bool init() = 0;
  virtual bool run() = 0;
  virtual bool pause( const double &when ) = 0;
  virtual bool quit() = 0;

  virtual bool observationRunning() = 0;
};

class PLCClient {
public:
  PLCClient( Stream &s, PLCRunnable &job, const std::string &procID, unsigned observationID );
  void start();

  virtual ~PLCClient();

  bool isDone() const {
    return itsDone;
  }

  void waitForDone();

private:
  Stream &itsStream;
  PLCRunnable &itsJob;
  const std::string itsProcID;
  time_t itsStartTime;
  bool itsDefineCalled;
  bool itsDone;
  const std::string itsLogPrefix;
  Mutex itsMutex;
  Condition itsCondition;
  SmartPtr<Thread> itsThread;

  static const unsigned defineWaitTimeout = 300; // #seconds for ApplController to call define() before we disconnect

  void mainLoop();

protected:
  void sendCmd( PCCmd cmd, const std::string &options );
  void sendResult( PCCmd cmd, const std::string &options, uint16 result );
  void recvCmd( PCCmd &cmd, std::string &options ) const;
};

} // namespace RTCP
} // namespace LOFAR

#endif

