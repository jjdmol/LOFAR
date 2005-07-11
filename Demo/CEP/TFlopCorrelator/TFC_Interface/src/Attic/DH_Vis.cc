//  DH_Vis.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////
#include <lofar_config.h>

#include <APS/ParameterSet.h>

#include <DH_Vis.h>

namespace LOFAR
{

DH_Vis::DH_Vis (const string& name, short startfreq, const ACC::APS::ParameterSet pSet)
: DataHolder    (name, "DH_Vis"),
  itsPS         (pSet),
  itsBuffer     (0),
  itsStartFreq  (startfreq), 
  itsNPols      (2),
  itsMatrix      (0)
{
   itsNPols = itsPS.getInt32("DH_Vis.NPols");
   itsNStations  = itsPS.getInt32("DH_Vis.stations");
   itsNBaselines = itsNStations * (itsNStations - 1);
}

DH_Vis::DH_Vis(const DH_Vis& that)
  : DataHolder(that),
    itsPS     (that.itsPS),
    itsBuffer (0),
    itsStartFreq (that.itsStartFreq),
    itsNStations (that.itsNStations),
    itsNBaselines(that.itsNBaselines),
    itsNPols  (that.itsNPols),
    itsMatrix      (0)
{}

DH_Vis::~DH_Vis()
{
}

DataHolder* DH_Vis::clone() const
{
  return new DH_Vis(*this);
}

void DH_Vis::init()
{
  // Determine the size of the buffer.
  itsBufSize = itsNFChannels * itsNBaselines * itsNPols*itsNPols;
  addField("Buffer", BlobField<BufferType>(1, itsBufSize));
  createDataBlock();  // calls fillDataPointers

  memset(itsBuffer, 0, itsBufSize*sizeof(BufferType)); 

  vector<DimDef> vdd;
  vdd.push_back(DimDef("Freq", itsNFChannels));
  vdd.push_back(DimDef("Baseline", itsNBaselines));
  vdd.push_back(DimDef("Polarisation", itsNPols));
  
  itsMatrix = new RectMatrix<BufferType> (vdd);
  itsMatrix->setBuffer(itsBuffer, itsBufSize);
}

void DH_Vis::fillDataPointers() 
{
  itsBuffer = getData<BufferType> ("Buffer");
}


}
