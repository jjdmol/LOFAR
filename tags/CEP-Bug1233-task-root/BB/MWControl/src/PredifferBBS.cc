//#  PredifferBBS.cc: Prediffer BBSler of distributed VDS processing
//#
//#  Copyright (C) 2007
//#
//#  $Id$

#include <lofar_config.h>
#include <MWControl/PredifferBBS.h>
#include <BBSKernel/Prediffer.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <ParmDB/ParmDB.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobString.h>

using namespace LOFAR::BBS;
using LOFAR::ParmDB::ParmDBMeta;

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
    Measurement::Pointer ms(new MeasurementAIPS(dataPartName,
						parset.getInt32 ("ObsID", 0),
						parset.getInt32 ("SubBandID", 0),
						parset.getInt32 ("FieldID", 0)));
    LOFAR::ParmDB::ParmDB instDB
      (ParmDBMeta(parset.getString("ParmDB.Instrument"), "aips"));
    LOFAR::ParmDB::ParmDB skyDB
      (ParmDBMeta(parset.getString("ParmDB.LocalSky"), "aips"));
    itsPrediffer = new Prediffer (ms, instDB, skyDB);
  }

  int PredifferBBS::process (int operation, int streamId,
			     LOFAR::BlobIStream& in,
			     LOFAR::BlobOStream& out)
  {
    return operation;
  }

}} // end namespaces
