//  DH_CMatrix.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////


#include <DH_CMatrix.h>
#include <Common/DataConvert.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

  DH_CMatrix::DH_CMatrix (const string& name, int xsize, int ysize, const string& xname, const string& yname)
    : DataHolder (name, "DH_CMatrix"),
      itsXaxis(0),
      itsYaxis(0),
      itsMatrix(0),
      itsXsize(xsize),
      itsYsize(ysize),
      itsXname(xname),
      itsYname(yname)
  {
    LOG_TRACE_STAT_STR("constructing matrix of "<<xsize<<" by "<<ysize);
    char myType[128];
    sprintf(myType, "DH_CMatrix_%dx%d", xsize, ysize);
    setType(myType);
  }

  DH_CMatrix::DH_CMatrix(const DH_CMatrix& that)
    : DataHolder(that)
  {
    itsXsize = that.itsXsize;
    itsYsize = that.itsYsize;
    itsXaxis = 0;
    itsYaxis = 0;
    itsMatrix = 0;
    itsXname = that.itsXname;
    itsYname = that.itsYname;
    LOG_TRACE_STAT("constructing matrix");
    char myType[128];
    sprintf(myType, "DH_CMatrix_%dx%d", itsXsize, itsYsize);
    setType(myType);
  }

  DH_CMatrix::~DH_CMatrix()
  {
    LOG_TRACE_STAT("deleting matrix");
  }

  DataHolder* DH_CMatrix::clone() const
  {
    return new DH_CMatrix(*this);
  }

  void DH_CMatrix::init()
  {
    // create fields in the Blob
    addField("Xaxis", BlobField<Axis>(1));
    addField("Yaxis", BlobField<Axis>(1));
    addField("Matrix", BlobField< valueType >(1, itsXsize, itsYsize));
    createDataBlock();

    // initialize values
    itsXaxis->setName(itsXname.c_str());
    itsXaxis->setBegin(0);
    itsXaxis->setEnd(0);
    itsYaxis->setName(itsYname.c_str());
    itsYaxis->setBegin(0);
    itsYaxis->setEnd(0);
    // leave matrix uninitialized
  }

  void DH_CMatrix::fillDataPointers()
  {
    itsXaxis = getData<Axis>("Xaxis");
    itsYaxis = getData<Axis>("Yaxis");
    itsMatrix = (valueType *) getData< valueType >("Matrix");
    LOG_TRACE_VAR_STR("constructing matrix: "<<itsMatrix);
  }

  void dataConvert (DataFormat fmt, DH_CMatrix::Axis* buf, uint nrval)
  {
    for (uint i=0; i<nrval ;i++) {
      dataConvert (fmt, buf[i].itsName, AXISNAMESIZE);
      dataConvertFloat (fmt, &(buf[i].itsBegin));
      dataConvertFloat (fmt, &(buf[i].itsEnd));
    }
  }

}
// Instantiate the template.
#include <Blob/BlobField.tcc>
template class LOFAR::BlobField<LOFAR::DH_CMatrix::Axis>;

