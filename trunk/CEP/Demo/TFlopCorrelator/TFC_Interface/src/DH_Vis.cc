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

DH_Vis::DH_Vis (const string& name, short startfreq)
: DataHolder    (name, "DH_Vis"),
  itsBuffer     (0),
  itsStartFreq  (startfreq), 
  itsNPols      (2)
{
   ACC::APS::ParameterSet  myPS("TFlopCorrelator.cfg");
   itsNStations  = myPS.getInt32("DH_CorrCube.stations");
}

DH_Vis::DH_Vis(const DH_Vis& that)
  : DataHolder(that),
    itsBuffer(0)
{
  itsNPols = that.itsNPols;
  itsNStations = that.itsNStations;
}

DH_Vis::~DH_Vis()
{
}

DataHolder* DH_Vis::clone() const
{
  return new DH_Vis(*this);
}

void DH_Vis::init()
{
  // Determine the number of bytes needed for DataPacket and buffer.
  itsBufSize = itsNStations * itsNStations * itsNFChannels * itsNPols*itsNPols;
  addField("Buffer", BlobField<fcomplex>(1, itsBufSize));
  createDataBlock();  // calls fillDataPointers
  //itsBuffer = getData<fcomplex> ("Buffer");
  memset(itsBuffer, 0, itsBufSize*sizeof(fcomplex)); 

}

void DH_Vis::fillDataPointers() {
  itsBuffer = getData<fcomplex> ("Buffer");

}


}
