//#  tDH_BBSStrategy.cc: Test program for the DH_BBSStrategy class.
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Transport/DH_BlobStreamable.h>
#include <BBSControl/BBSStrategy.h>
#include <APS/ParameterSet.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <Common/TypeNames.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace LOFAR::ACC::APS;

// If \a doSteps is \c true, then the BBSStep objects within a BBSStrategy
// will also be written.
bool doIt(bool doSteps)
{
  LOG_INFO_STR("doIt(" << (doSteps ? "true" : "false") << ")");
  const char* sendFile = "/tmp/DH_BBSStrategy.send";
  const char* recvFile = "/tmp/DH_BBSStrategy.recv";
  const char* parsetFile = "tBBSControl.parset";
  ParameterSet parset(parsetFile);
  ofstream ofs;

  // Create a BBSStrategy `sendStrategy' to be sent.
  BBSStrategy sendStrategy(parset);
  sendStrategy.shouldWriteSteps(doSteps);

  TH_Mem aTH;
  DH_BlobStreamable sendDH;
  DH_BlobStreamable recvDH;
  Connection conn("conn", &sendDH, &recvDH, &aTH, false);

  // Send `sendStrategy' from `sendDH' to `recvDH' using Connection `conn'.
  sendDH.serialize(sendStrategy);
  conn.write();
  conn.read();
  BBSStrategy* recvStrategy = dynamic_cast<BBSStrategy*>(recvDH.deserialize());
  ASSERT(recvStrategy);

  // If \a doSteps is false, we should write to file a copy of \a sendStrategy
  // that does not contain any BBSStep objects. Otherwise the `diff' will
  // fail.
  if (!doSteps) {
    parset.replace("Strategy.Steps", "[]");
    sendStrategy = BBSStrategy(parset);
  }
  
  // Store the contents of `sendStrategy' in `sendFile'
  ofs.open(sendFile);
  ASSERT(ofs);
  ofs << sendStrategy;
  ASSERT(ofs);
  ofs.close();
  ASSERT(ofs);

  // Store the contents of `recvStrategy' in `recvFile'
  ofs.open(recvFile);
  ASSERT(ofs);
  ASSERT(ofs << *recvStrategy);
  ofs.close();
  ASSERT(ofs);

  // Clean up `recvStrategy'
  delete recvStrategy;

  string cmd;
  string arg(sendFile + string(" ") + recvFile);

  // Compare the contents of `sendFile' and `recvFile'; they should be equal.
  cmd = "diff " + arg;
  LOG_TRACE_FLOW_STR("About to make call `system(" << cmd << ")'");
  int result = system(cmd.c_str());
  LOG_TRACE_FLOW_STR("Return value of system() call: " << result);

  // Clean up the files `sendFile' and `recvFile'
  cmd = "rm -f " + arg;
  LOG_TRACE_FLOW_STR("About to make call `system(" << cmd << ")'");
  system(cmd.c_str());

  if (result != 0) {
    LOG_ERROR("`sendStrategy' and `recvStrategy' are not equal");
    return false;
  }

  return true;
}

int main()
{
  const string progName("tDH_BBSStrategy");

  INIT_LOGGER(progName.c_str());
  LOG_INFO_STR(progName << " is starting up ...");

  try {
    ASSERT(doIt(false) && doIt(true));
  } 
  catch (Exception& e) {
    LOG_FATAL_STR(progName << " terminated due to a fatal exception!\n" << e);
    return 1;
  }

  LOG_INFO_STR(progName << " terminated succesfully");
  return 0;
}
