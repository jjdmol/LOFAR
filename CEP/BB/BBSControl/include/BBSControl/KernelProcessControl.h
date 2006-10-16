//#  BBSKernelProcessControl.h: 
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

#ifndef LOFAR_BBSCONTROL_BBSKERNELPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_BBSKERNELPROCESSCONTROL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <PLC/ProcessControl.h>
#include <APS/ParameterSet.h>

namespace LOFAR
{
  //# Forward declations
  namespace ParmDB { class ParmDB; }
  class CSConnection;
  class TH_Socket;

  namespace BBS
  {
    //# Forward declations
    class BBSStep;
    class BBSStrategy;
    class BBSPredictStep;
    class BBSSubtractStep;
    class BBSCorrectStep;
    class BBSSolveStep;
    class Prediffer;
    struct Context;
    class DH_BlobStreamable;

    // \addtogroup BBS
    // @{

    // Implementation of the ProcessControl interface for the local BBSKernel
    // controller.
    class BBSKernelProcessControl: public ACC::PLC::ProcessControl
    {
    public:
      // Default constructor.
      BBSKernelProcessControl();

      // Destructor
      virtual ~BBSKernelProcessControl();

      // @name Implementation of PLC interface.
      // @{
      virtual tribool define();
      virtual tribool init();
      virtual tribool run();
      virtual tribool pause(const string& condition);
      virtual tribool quit();
      virtual tribool snapshot(const string& destination);
      virtual tribool recover(const string& source);
      virtual tribool reinit(const string& configID);
      virtual string  askInfo(const string& keylist);
      // @}

      bool handle(const BBSStrategy *strategy);
      bool handle(const BBSStep *step);

    private:
      // @name Implementation of handle() for the different BBSStep types.
      // @{
      bool doHandle(const BBSPredictStep *step);
      bool doHandle(const BBSSubtractStep *step);
      bool doHandle(const BBSCorrectStep *step);
      bool doHandle(const BBSSolveStep *step);
      // @}

      void convertStepToContext(const BBSStep *step, Context &context);

      // Parameter set for this process controller.
      ACC::APS::ParameterSet itsParameterSet;

      // Prediffer
      Prediffer* itsPrediffer;

      // History database.
      LOFAR::ParmDB::ParmDB* itsHistory;

      // DataHolder for exchanging data between local (BBSKernel) and global
      // (BBS) process control.
      DH_BlobStreamable* itsDataHolder;

      // TransportHolder used to exchange DataHolders. The local controller
      // will open a client connection to the global controller.
      TH_Socket* itsTransportHolder;

      // Connection between the local (BBSKernel) process control and the
      // global (BBS) process control.
      CSConnection* itsConnection;
    };

  } // namespace BBS

} // namespace LOFAR

#endif
