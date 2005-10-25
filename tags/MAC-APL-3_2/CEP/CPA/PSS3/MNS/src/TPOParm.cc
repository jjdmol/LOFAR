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
#ifdef HAVE_DTL

#include <PL/TPersistentObject.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobArray.h>
#include <string>


namespace LOFAR {
  namespace PL {


void DBRep<MeqParmDefHolder>::bindCols (dtl::BoundIOs& cols)
{
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
void DBRep<MeqParmHolder>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<MeqParmDefHolder>::bindCols (cols);
  cols["STARTTIME"]  == itsStartTime;
  cols["ENDTIME"]    == itsEndTime;
  cols["STARTFREQ"]  == itsStartFreq;
  cols["ENDFREQ"]    == itsEndFreq;
}

void DBRep<MeqParmDefHolder>::toDBRep (const MeqParmDefHolder& obj)
{
  // Copy the info from MeqParmDefHolder
  itsName    = obj.getName();
  itsSrcNr   = obj.getSourceNr();
  itsStatNr  = obj.getStation();
  itsCoeff00 = obj.getPolc().getCoeff().getDouble();
  BlobStringType bstype(true);
  BlobString bstr(bstype);
  BlobOBufString bb(bstr);
  BlobOStream bs(bb);
  {
    bb.clear();
    bs.clear();
    bs << obj.getPolc().getCoeff();
    itsCoeff = bstr.getString();
  }
  {
    bb.clear();
    bs.clear();
    bs << obj.getPolc().getSimCoeff();
    itsSimCoeff = bstr.getString();
  }
  {
    bb.clear();
    bs.clear();
    bs << obj.getPolc().getPertSimCoeff();
    itsPertSimCoeff = bstr.getString();
  }
  {
    bb.clear();
    bs.clear();
    //    bs << std::vector<bool>(1,true);
    bs << std::vector<int>(1,1);
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
void DBRep<MeqParmDefHolder>::fromDBRep (MeqParmDefHolder& obj) const
{
  // Finally copy the info to MeqParmDefHolder
  obj.setName     (itsName);
  obj.setSourceNr (itsSrcNr);
  obj.setStation  (itsStatNr);
  MeqPolc polc;
  {
    BlobIBufChar bb(itsCoeff.data(), itsCoeff.size());
    BlobIStream bs(bb);
    MeqMatrix mat;
    bs >> mat;
    polc.setCoeff (mat);
  }
  {
    BlobIBufChar bb(itsSimCoeff.data(), itsSimCoeff.size());
    BlobIStream bs(bb);
    MeqMatrix mat;
    bs >> mat;
    polc.setSimCoeff (mat);
  }
  {
    BlobIBufChar bb(itsPertSimCoeff.data(), itsPertSimCoeff.size());
    BlobIStream bs(bb);
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

void DBRep<MeqParmHolder>::toDBRep (const MeqParmHolder& obj)
{
  // Copy the info from MeqParmDefHolder
  DBRep<MeqParmDefHolder>::toDBRep (obj);
  itsStartTime   = obj.getPolc().domain().startX();
  itsEndTime     = obj.getPolc().domain().endX();
  itsStartFreq   = obj.getPolc().domain().startY();
  itsEndFreq     = obj.getPolc().domain().endY();
}

// fromDatabaseRep copies the fields of the DBRep<MeqParmHolder> structure
// to the persistency layer and the MeqParmHolder class.
void DBRep<MeqParmHolder>::fromDBRep (MeqParmHolder& obj) const
{
  // Copy the info to MeqParmDefHolder.
  DBRep<MeqParmDefHolder>::fromDBRep (obj);
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
  tableName ("MeqParmXDef");
}
template<>
void TPersistentObject<MeqParmHolder>::init()
{
  // Set the default table name.
  tableName ("MeqParmx");
}

template<>
void TPersistentObject<MeqParmDefHolder>::initAttribMap()
{
  // Set the field to column name mapping.
  theirAttribMap["objID"]     = "ObjId";
  theirAttribMap["owner"]     = "Owner";
  theirAttribMap["versionNr"] = "VersionNr";
  theirAttribMap["name"]      = "NAME";
  theirAttribMap["coeff00"]   = "VALUES00";
  theirAttribMap["time0"]     = "TIME0";
  theirAttribMap["freq0"]     = "FREQ0";
  theirAttribMap["normalize"] = "NORMALIZE";
  theirAttribMap["diff"]      = "DIFF";
  theirAttribMap["diffrel"]   = "DIFF_REL";
}
template<>
void TPersistentObject<MeqParmHolder>::initAttribMap()
{
  // Set the field to column name mapping.
  theirAttribMap["objID"]     = "ObjId";
  theirAttribMap["owner"]     = "Owner";
  theirAttribMap["versionNr"] = "VersionNr";
  theirAttribMap["name"]      = "NAME";
  theirAttribMap["startTime"] = "STARTTIME";
  theirAttribMap["endTime"]   = "ENDTIME";
  theirAttribMap["startFreq"] = "STARTFREQ";
  theirAttribMap["endFreq"]   = "ENDFREQ";
  theirAttribMap["coeff00"]   = "VALUES00";
  theirAttribMap["time0"]     = "TIME0";
  theirAttribMap["freq0"]     = "FREQ0";
  theirAttribMap["normalize"] = "NORMALIZE";
  theirAttribMap["diff"]      = "DIFF";
  theirAttribMap["diffrel"]   = "DIFF_REL";
}



//# Force the instantiation of the TPersistent templates.
//# The instantiation of the DBRep templates is not necessary, because
//# they are full specializations, thus already compiled.
template class TPersistentObject<MeqParmDefHolder>;
template class TPersistentObject<MeqParmHolder>;


  }  // end namespace PL
}    // end namespace LOFAR

#endif
