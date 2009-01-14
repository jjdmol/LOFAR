//# PredifferTest.cc: Test class mimicking the PredifferBBS behaviour
//#
//# Copyright (C) 2005
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

#include <lofar_config.h>
#include "PredifferTest.h"
#include "MWStepTester.h"
#include <MWCommon/MasterControl.h>
#include <MWCommon/MWStepFactory.h>
#include <MWCommon/MWIos.h>
#include <MWCommon/MWError.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>

using namespace std;


namespace LOFAR { namespace CEP {

  PredifferTest::PredifferTest()
  {}

  PredifferTest::~PredifferTest()
  {}

  WorkerProxy::ShPtr PredifferTest::create()
  {
    return WorkerProxy::ShPtr (new PredifferTest());
  }

  void PredifferTest::setInitInfo (const ParameterSet& parset,
				   const std::string& dataPartName)
  {
    MWCOUT << "PredifferTest::setInitInfo" << endl
           << "  MS:         " << dataPartName << endl
           << "  Column:     " << parset.getString("InputData", "DATA") << endl
           << "  SkyParmDB:  " << parset.getString("ParmDB.LocalSky") << endl
           << "  InstParmDB: " << parset.getString("ParmDB.Instrument") << endl
           << "  Subband:    " << parset.getInt32 ("SubBandID", 0) << endl
           << "  CalcUVW:    " << parset.getBool  ("CalcUVW", false) << endl;
  }

  int PredifferTest::process (int operation, int streamId,
			      LOFAR::BlobIStream& in,
			      LOFAR::BlobOStream& out)
  {
    int resOper = operation;
    MWCOUT << "PredifferTest::doProcess" << endl;
    MWCOUT << "  Operation: " << operation << endl;
    MWCOUT << "  StreamId:  " << streamId << endl;
    switch (operation) {
    case MasterControl::SetWd:
    {
      ObsDomain workDomain;
      in >> workDomain;
      MWCOUT << "  Set work domain: " << workDomain << endl;
      break;
    }
    case MasterControl::Step:
    {
      // A step has to be processed; first construct the object.
      MWStep::ShPtr step = MWStepFactory::create (in.getNextType());
      // Fill it from the blobstream.
      step->fromBlob (in);
      // Process the step (using a visitor).
      MWStepTester visitor (streamId, &out);
      step->visit (visitor);
      resOper = visitor.getResultOperation();
      break;
    }
    case MasterControl::GlobalInfo:
    {
      MWCOUT << "  GetEq" << endl;
      out << true;
      break;
    }
    case MasterControl::GlobalExec:
    {
      MWCOUT << "  Solve" << endl;
      bool value;
      in >> value;
      resOper = -1;     // no reply to be sent
      break;
    }
    default:
      THROW (MWError, "PredifferTest::process: operation "
	     << operation << " is unknown");
    }
    return resOper;
  }

}} // end namespaces
