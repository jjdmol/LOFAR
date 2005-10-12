//# Stub_PhaseCorr.cc: Stub for connection of delay control with
//# poly phase filter.
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# $Id$
//#
////////////////////////////////////////////////////////////////////

#include <TFC_Interface/Stub_PhaseCorr.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC::APS;

namespace LOFAR { 

Stub_PhaseCorr::Stub_PhaseCorr (bool inBGLProc, const ACC::APS::ParameterSet pSet)
  : itsInBGLProc    (inBGLProc),
    itsPS           (pSet),
    itsTH           (0),
    itsConnection   (0)
{
}
  
Stub_PhaseCorr::~Stub_PhaseCorr()
{
  delete itsTH;
  delete itsConnection;
}
  
void Stub_PhaseCorr ::connect (TinyDataManager& dm, int dhNr)
{
  string service = itsPS.getString("PhaseCorrection.RequestPort");
  
  if (!itsInBGLProc) // on the input side, start server socket
  {
    DBGASSERTSTR(itsTH == 0, "Stub input already been connected.");
    // Create a server socket
    itsTH = new TH_Socket(service,
			  true,
			  Socket::TCP,
			  5,
			  false);
    itsConnection = new Connection("toBG", 0, 
				   dm.getGeneralOutHolder(dhNr),
				   itsTH, true);
    dm.setOutConnection(dhNr, itsConnection);
  } 
  else    // on the BG/L side, so start a client socket
  {
    DBGASSERTSTR(itsTH == 0, "Stub output has already been connected.");
    // Create a client socket
    string server = itsPS.getString("DelayCompensation.ServerHost");
    itsTH = new TH_Socket(server,
			  service,
			  true,
			  Socket::TCP,
			  false);
    itsConnection = new Connection("fromDelayCompensation", 
				   dm.getGeneralInHolder(dhNr), 
				   0, itsTH, true);
    dm.setInConnection(dhNr, itsConnection);

  }
  
};

} //namespace

