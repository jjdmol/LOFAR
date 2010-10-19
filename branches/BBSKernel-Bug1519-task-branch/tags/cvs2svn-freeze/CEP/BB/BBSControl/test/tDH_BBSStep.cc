//#  tDH_BBSStep.cc: Test program for the DH_BBSStep class.
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
#include <BBSControl/BBSStep.h>
#include <APS/ParameterSet.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <Common/TypeNames.h>

using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace LOFAR::ACC::APS;

int main()
{
  const string progName("tDH_BBSStep");

  INIT_LOGGER(progName.c_str());
  LOG_INFO_STR(progName << " is starting up ...");

  try {

    BBSStep* sendStep = 0;
    BBSStep* recvStep = 0;
    const char* sendFile = "/tmp/DH_BBSStep.send";
    const char* recvFile = "/tmp/DH_BBSStep.recv";
    const char* parsetFile = "tBBSControl.parset";
    ofstream ofs;

    // Create a BBSStep `sendStep' to be sent.
    sendStep = BBSStep::create("xyz1", ParameterSet(parsetFile));
    ASSERT(sendStep);

    // Store the contents of `sendStep' in `sendFile'
    ofs.open(sendFile);
    ASSERT(ofs);
    ASSERT(ofs << *sendStep);
    ofs.close();
    ASSERT(ofs);

    TH_Mem aTH;
    DH_BlobStreamable sendDH("sendDH");
    DH_BlobStreamable recvDH("recvDH");
    Connection conn("conn", &sendDH, &recvDH, &aTH, false);

    // Send `sendStep' from `sendDH' to `recvDH' using Connection `conn'.
    sendDH.serialize(*sendStep);
    conn.write();
    conn.read();
    recvStep = dynamic_cast<BBSStep*>(recvDH.deserialize());
    ASSERT(recvStep);

    // Store the contents of `recvStep' in `recvFile'
    ofs.open(recvFile);
    ASSERT(ofs);
    ASSERT(ofs << *recvStep);
    ofs.close();
    ASSERT(ofs);
    
    delete sendStep;
    delete recvStep;

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
      LOG_ERROR("`sendStep' and `recvStep' are not equal");
      return 1;
    }
    
  } 
  catch (Exception& e) {
    LOG_FATAL_STR(progName << " terminated due to a fatal exception!\n" << e);
    return 1;
  }

  LOG_INFO_STR(progName << " terminated succesfully");
  return 0;
}
