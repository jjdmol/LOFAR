//# TPOParm.cc: Persistency object for MeqParmHolder
//#
//# Copyright (C) 2003
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

#include <MNS/TPOParm.h>
#include <PL/DTLBase.h>
#include <PL/Collection.h>
#include <PL/Query.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobArray.h>
#include <string>


namespace LOFAR {
  namespace PL {


void MetaDataRep::metaBindCols (dtl::BoundIOs& cols)
{
  cols["ObjID"]      == itsOid;
  cols["Owner"]      == itsOwnerOid;
  cols["VersionNr"]  == itsVersionNr;
}

void DBRep<MeqParmDefHolder>::Rep::bindCols (dtl::BoundIOs& cols)
{
  cols["ObjID"]      == itsOid;
  cols["Owner"]      == itsOwnerOid;
  cols["VersionNr"]  == itsVersionNr;
  cols["NAME"]       == itsName;
  cols["SRCNR"]      == itsSrcNr;
  cols["STATNR"]     == itsStatNr;
  cols["VALUES00"]   == itsCoeff00;
  cols["VALUES"]     == itsCoeff;
  cols["SIM_VALUES"] == itsSimCoeff;
  cols["SIM_PERT"]   == itsPertSimCoeff;
  cols["TIME0"]      == itsTime0;
  cols["FREQ0"]      == itsFreq0;
  cols["NORMALIZED"] == itsNormalized;
  cols["SOLVABLE"]   == itsMask;
  cols["DIFF"]       == itsDiff;
  cols["DIFF_REL"]   == itsDiffRel;
}
void DBRep<MeqParmHolder>::Rep::bindCols (dtl::BoundIOs& cols)
{
  DBRep<MeqParmDefHolder>::Rep::bindCols (cols);
  cols["STARTTIME"]  == itsStartTime;
  cols["ENDTIME"]    == itsEndTime;
  cols["STARTFREQ"]  == itsStartFreq;
  cols["ENDFREQ"]    == itsEndFreq;
}

// toDatabaseRep copies the fields of the persistency layer
// and of the MeqParmHolder class to the given DBRep<MeqParmHolder>
// structure
void MetaDataRep::metaToRep (const PersistentObject& obj)
{
  // copy info of the MetaData to the DBRep object.
  itsOid       = obj.metaData().oid()->get();
  itsOwnerOid  = obj.metaData().ownerOid()->get();
  itsVersionNr = obj.metaData().versionNr();  
}
void MetaDataRep::metaFromRep (PersistentObject& obj) const
{
  // copy info of the DBRep object to the MetaData.
  obj.metaData().oid()->set      (itsOid);
  obj.metaData().ownerOid()->set (itsOwnerOid);
  obj.metaData().versionNr() =    itsVersionNr;
}

void DBRep<MeqParmDefHolder>::Rep::toRep (const MeqParmDefHolder& obj)
{
  // Copy the info from MeqParmDefHolder
  itsName    = obj.getName();
  itsSrcNr   = obj.getSourceNr();
  itsStatNr  = obj.getStation();
  itsCoeff00 = obj.getPolc().getCoeff().getDouble();
  BlobStringType bstype(true);
  BlobString bstr(bstype);
  BlobOBufString bb(bstr);
  BlobOStream bs(&bb);
  {
    bstr.resize(0);
    bs << obj.getPolc().getCoeff();
    itsCoeff = bstr.getString();
  }
  {
    bstr.resize(0);
    bs << obj.getPolc().getSimCoeff();
    itsSimCoeff = bstr.getString();
  }
  {
    bstr.resize(0);
    bs << obj.getPolc().getPertSimCoeff();
    itsPertSimCoeff = bstr.getString();
  }
  {
    bstr.resize(0);
    bs << true;
    itsMask = bstr.getString();
  }
  itsTime0       = obj.getPolc().getX0();
  itsFreq0       = obj.getPolc().getY0();
  itsNormalized  = obj.getPolc().isNormalized();
  itsDiff        = obj.getPolc().getPerturbation();
  itsDiffRel     = obj.getPolc().isRelativePerturbation();
}


// fromDatabaseRep copies the fields of the DBRep<MeqParmHolder> structure
// to the persistency layer and the MeqParmHolder class.
void DBRep<MeqParmDefHolder>::Rep::fromRep (MeqParmDefHolder& obj) const
{
  // Finally copy the info to MeqParmDefHolder
  obj.setName     (itsName);
  obj.setSourceNr (itsSrcNr);
  obj.setStation  (itsStatNr);
  MeqPolc polc;
  {
    BlobIBufChar bb(itsCoeff.data(), itsCoeff.size());
    BlobIStream bs(&bb);
    MeqMatrix mat;
    bs >> mat;
    polc.setCoeff (mat);
  }
  {
    BlobIBufChar bb(itsSimCoeff.data(), itsSimCoeff.size());
    BlobIStream bs(&bb);
    MeqMatrix mat;
    bs >> mat;
    polc.setSimCoeff (mat);
  }
  {
    BlobIBufChar bb(itsPertSimCoeff.data(), itsPertSimCoeff.size());
    BlobIStream bs(&bb);
    MeqMatrix mat;
    bs >> mat;
    polc.setPertSimCoeff (mat);
  }
  polc.setX0 (itsTime0);
  polc.setY0 (itsFreq0);
  polc.setNormalize (itsNormalized);
  polc.setPerturbation (itsDiff, itsDiffRel);
  obj.setPolc (polc);
}

void DBRep<MeqParmHolder>::Rep::toRep (const MeqParmHolder& obj)
{
  // Copy the info from MeqParmDefHolder
  DBRep<MeqParmDefHolder>::Rep::toRep (obj);
  itsStartTime   = obj.getPolc().domain().startX();
  itsEndTime     = obj.getPolc().domain().endX();
  itsStartFreq   = obj.getPolc().domain().startY();
  itsEndFreq     = obj.getPolc().domain().endY();
}

// fromDatabaseRep copies the fields of the DBRep<MeqParmHolder> structure
// to the persistency layer and the MeqParmHolder class.
void DBRep<MeqParmHolder>::Rep::fromRep (MeqParmHolder& obj) const
{
  // Copy the info to MeqParmDefHolder.
  DBRep<MeqParmDefHolder>::Rep::fromRep (obj);
  obj.setDomain (MeqDomain (itsStartTime, itsEndTime,
			    itsStartFreq, itsEndFreq));
}

//
// Initialize the internals of TPersistentObject<MeqParmHolder>
//
template<>
void TPersistentObject<MeqParmDefHolder>::init()
{
  // Set the default table name.
  tableName ("MeqParmDefX");
}
template<>
void TPersistentObject<MeqParmHolder>::init()
{
  // Set the default table name.
  tableName ("MeqParmx");
}



//# Force the instantiation of the templates.
template class TPersistentObject<MeqParmDefHolder>;
template class TPersistentObject<MeqParmHolder>;
template class DBRep<MeqParmDefHolder>;
template class DBRep<MeqParmHolder>;


  }  // end namespace PL
}    // end namespace LOFAR
