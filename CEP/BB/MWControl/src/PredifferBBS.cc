//#  PredifferBBS.cc: Prediffer BBSler of distributed VDS processing
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <lofar_config.h>
#include <MWControl/PredifferBBS.h>
#include <BBSKernel/Prediffer.h>
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
    itsPrediffer = new Prediffer (dataPartName,
				  parset.getInt32 ("SubBandID", 0),
				  parset.getString("InputData", "DATA"),
				  parset.getString("ParmDB.LocalSky"),
				  parset.getString("ParmDB.Instrument"),
				  parset.getString("ParmDB.History"),
				  parset.getBool  ("CalcUVW", false));
  }

  int PredifferBBS::process (int operation, int streamId,
			     LOFAR::BlobIStream& in,
			     LOFAR::BlobOStream& out)
  {
    return operation;
  }

}} // end namespaces
