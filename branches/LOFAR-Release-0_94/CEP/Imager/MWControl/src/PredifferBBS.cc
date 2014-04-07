//# PredifferBBS.cc: A WorkerProxy to handle BBSKernel prediffer commands
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
#include <MWControl/PredifferBBS.h>
///#include <BBSKernel/Prediffer.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <ParmDB/ParmDB.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobString.h>

using namespace LOFAR::BBS;

namespace LOFAR { namespace CEP {

  PredifferBBS::PredifferBBS()
    : itsPrediffer (0)
  {}

  PredifferBBS::~PredifferBBS()
  {
    delete itsPrediffer;
  }

  WorkerProxy::ShPtr PredifferBBS::create()
  {
    return WorkerProxy::ShPtr (new PredifferBBS());
  }

  void PredifferBBS::setInitInfo (const ParameterSet& parset,
				  const string& dataPartName)
  {
    delete itsPrediffer;
    itsPrediffer = 0;
    Measurement::Ptr ms(new MeasurementAIPS(dataPartName,
                                            parset.getInt32 ("ObsID", 0),
                                            parset.getInt32 ("SubBandID", 0),
                                            parset.getInt32 ("FieldID", 0)));
    ParmDB instDB (ParmDBMeta(parset.getString("ParmDB.Instrument"), "casa"));
    ParmDB skyDB  (ParmDBMeta(parset.getString("ParmDB.LocalSky"), "casa"));
    ///itsPrediffer = new Prediffer (ms, instDB, skyDB);
  }

  int PredifferBBS::process (int operation, int streamId,
			     LOFAR::BlobIStream& in,
			     LOFAR::BlobOStream& out)
  {
    return operation;
  }

}} // end namespaces
