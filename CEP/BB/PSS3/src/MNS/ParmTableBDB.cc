//# ParmTableBDB.cc: Object to hold parameters in a mysql database table.
//#
//# Copyright (C) 2002
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

#include <MNS/ParmTableBDB.h>
#include <MNS/MeqDomain.h>
#include <Common/BlobIBufChar.h>
#include <Common/BlobOBufChar.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIStream.h>
#include <Common/BlobOStream.h>
#include <Common/BlobString.h>
#include <Common/LofarLogger.h>
#include <casa/BasicMath/Math.h>
#include <stdlib.h>
#include <sstream>

namespace LOFAR {

  ParmTableBDB::ParmTableBDB (const string& userName, const string& tableName) : 
    itsDb (NULL, 0),
    itsTableName (tableName)
  {
    u_int32_t oFlags = DB_CREATE;
    string dbfilename = "/tmp/" + userName + "." + tableName + ".bdb";
    itsDb.set_flags(DB_DUPSORT);
    //  if (itsDb.open(transid, filename, database, dbtype, flags, mode) !=0) {
    if (itsDb.open(NULL, dbfilename.c_str(), NULL, DB_BTREE, oFlags, 0) != 0 ) {
      itsDb.close(0);
      ASSERTSTR(false, "no connection to database");    
    }
    LOG_TRACE_STAT("connected to database");
  }

  ParmTableBDB::~ParmTableBDB()
  {
    itsDb.sync(0);
    itsDb.close(0);
  }

  vector<MeqPolc> ParmTableBDB::getPolcs (const string& parmName,
					  int, int,
					  const MeqDomain& domain)
  {
    LOG_TRACE_STAT("retreiving polynomial coefficients");
    vector<MeqParmHolder> MPH = find(parmName, domain);
    vector<MeqPolc> result;
    for (unsigned int i=0; i<MPH.size(); i++) {
      result.push_back(MPH[i].getPolc());
    }
    LOG_TRACE_STAT_STR("finished retreiving polc: "<<result.size()<<" polcs found.");
    return result;
  }

  MeqPolc ParmTableBDB::getInitCoeff (const string& parmName,
				      int srcnr, int statnr)
  {
    // Try to find the default initial values in the InitialValues subtable.
    // The parameter name consists of parts (separated by dots), so the
    // parameters are categorised in that way.
    // An initial value can be defined for the full name or for a higher
    // category.
    // So look up until found or until no more parts are left.
  
    // for Berkeley DB there is no difference between initial values and normal values (yet)
  
    LOG_TRACE_STAT_STR("retreiving intital coefficients for "<<parmName);
    MeqPolc result;

    MPHKey key(parmName);
    MPHValue value;  
  
    string name = parmName;
    while (true) {
      if (itsDb.get(NULL, &key, &value, 0) != 0) {
	// key wasn't found or there was an error
	string::size_type idx = name.rfind ('.');
	// Exit loop if no more name parts.
	if (idx == string::npos) 
	  {
	    LOG_TRACE_VAR("no match for default value or couldn't reach database");
	    break;
	  }
	// Remove last part and try again.
	name = name.substr (0, idx);
	key.resetKey(name);
      } else {
	result = value.getMPH().getPolc();
	break;
      }
    }

    LOG_TRACE_STAT("finished retreiving intital coefficients");
    return result;
  }
				    
  void ParmTableBDB::putCoeff (const string& parmName,
			       int srcnr, int statnr,
			       const MeqPolc& polc)
  {
    const MeqDomain& domain = polc.domain();

    // right now: search on name only and add MPH with correct domain
    MPHKey key(parmName);
    MPHValue value;
    MeqDomain pdomain;
    Dbc* cursorp;
    itsDb.cursor(NULL, &cursorp, 0);

    int flags = DB_SET;
    while (cursorp->get(&key, &value, flags) == 0) {
      flags = DB_NEXT_DUP; // this means it will only walk through the MPHs with the same key (=name)
      pdomain = value.getMPH().getPolc().domain();
      if (near(domain.startX(), pdomain.startX())  &&
	  near(domain.endX(),   pdomain.endX())  &&
	  near(domain.startY(), pdomain.startY())  &&
	  near(domain.endY(),   pdomain.endY())) 
	{
	  // delete the MPH that has the same name and domain (because the combination should be unique) and then just add the right value
	  cursorp->del(0);
	  break;
	}
    }
    cursorp->close();
    putNewCoeff (parmName, srcnr, statnr, polc);
  }

  void ParmTableBDB::putNewCoeff (const string& parmName,
				  int srcnr, int statnr,
				  const MeqPolc& polc)
  {
    MPHKey key(parmName, polc.domain());
    MeqParmHolder mph(parmName, srcnr, statnr, polc);
    MPHValue value(mph);
    itsDb.put(NULL, &key, &value, 0);
  }

  void ParmTableBDB::putNewDefCoeff (const string& parmName,
				     int srcnr, int statnr,
				     const MeqPolc& polc)
  {
    // For Berkeley DB there is no difference between a default and a normal parm (yet)
    putNewCoeff (parmName, srcnr, statnr, polc);  
  }

  vector<MeqParmHolder> ParmTableBDB::find (const string& parmName,
					    const MeqDomain& domain)
  {
    LOG_TRACE_STAT("searching for MParms");
    vector<MeqParmHolder> set;

    // right now: search on name only and add MPH with correct domain
    MPHKey key(parmName);
    MPHValue value;
    MeqDomain pdomain;
    Dbc* cursorp;
    itsDb.cursor(NULL, &cursorp, 0);

    int flags = DB_SET;
    while (cursorp->get(&key, &value, flags) == 0) {
      flags = DB_NEXT_DUP;
      pdomain = value.getMPH().getPolc().domain();
      if ((domain.endX() > pdomain.startX())  &&
	  (pdomain.endX() > domain.startX())  &&
	  (domain.endY() > pdomain.startY())  &&
	  (pdomain.endY() > domain.startY())) 
	{
	  set.push_back(value.getMPH());
	}
    }
    cursorp->close();
    return set;
  }

  vector<string> ParmTableBDB::getSources()
  {
    LOG_TRACE_STAT("retreiving sources");
    vector<string> nams;
  
    MPHKey key("RA");
    MPHValue value;
  
    // right now: walk complete database
    // later we should add "RA" as a key to the table
    // then we can first find RA and then walk the rest of the database
    // or we can use a secondary key on the first part of the name
    Dbc* cursorp;
    itsDb.cursor(NULL, &cursorp, 0);
    string name;
    int flags = DB_SET_RANGE; //go to RA or the first string that is bigger
    while (cursorp->get(&key, &value, flags) == 0) {
      flags = DB_NEXT;
      name = value.getMPH().getName();
      if (name.find("RA") == string::npos) 
	break;
      nams.push_back(name);
    }
  
    LOG_TRACE_STAT_STR("finished retreiving "<<nams.size()<<" sources from "<<itsTableName);
    cursorp->close();
    return nams;
  }

  void ParmTableBDB::unlock()
  {};

  ParmTableBDB::MPHKey::MPHKey(string name, MeqDomain md) 
    : itsName(name),
      itsDomain(md),
      itsBuffer(0)
  {
    updateThang();
  }
  ParmTableBDB::MPHKey::MPHKey(string name) 
    : itsName(name),
      //itsDomain(MeqDomain(0,0,0,0)),
      itsBuffer(0)
  {
    updateThang();
  }
  ParmTableBDB::MPHKey::MPHKey() 
    : itsName(),
      itsDomain(),
      itsBuffer(0)
  {
    updateThang();
  }
  void ParmTableBDB::MPHKey::resetKey(string name, MeqDomain md)
  {
    itsName = name;
    itsDomain = md;
    updateThang();
  }
  void ParmTableBDB::MPHKey::resetKey(string name)
  {
    itsName = name;
    updateThang();
  }
  void ParmTableBDB::MPHKey::updateThang()
  {
    delete itsBuffer;
    itsBuffer = new char[itsName.size()+1];
    strcpy (itsBuffer, itsName.c_str());
    set_data(itsBuffer);
    set_size(itsName.size() + 1);
  }
  ParmTableBDB::MPHValue::MPHValue(MeqParmHolder mph)
    : itsMPH(mph),
      itsBuffer(0)
  {
    updateThang();
  }
  ParmTableBDB::MPHValue::MPHValue()
    : itsMPH(),
      itsBuffer(0)
  {
    updateThang();
  }
  void ParmTableBDB::MPHValue::resetMPH(MeqParmHolder mph)
  {
    itsMPH = mph;
    updateThang();
  }
  void ParmTableBDB::MPHValue::updateThang()
  {
    // fill memoryBlock with MPH data
	
    // right now we reserve a block of fixed length
    // if the blobstring get too long we really have a problem!
	
    delete itsBuffer;
    //	itsBuffer = new char[totLens + sizeof(MPHData)];
    itsBuffer = new char[2048];
    memset(itsBuffer, 0, 2048);
    MPHData* mphdata = (MPHData *) itsBuffer;
    char *extraData = itsBuffer + sizeof(MPHData);
    char *nextPos = 0;
	
    // convert variable length datatypes to string
    MeqMat2string(itsMPH.getPolc().getCoeff(), extraData, &nextPos);
    mphdata->stringSizes[0] = nextPos - extraData;
    extraData = nextPos;
    MeqMat2string(itsMPH.getPolc().getSimCoeff(), extraData, &nextPos);
    mphdata->stringSizes[1] = nextPos - extraData;
    extraData = nextPos;
    MeqMat2string(itsMPH.getPolc().getPertSimCoeff(), extraData, &nextPos);
    mphdata->stringSizes[2] = nextPos - extraData;
    extraData = nextPos;
    string namestr = itsMPH.getName();
    mphdata->stringSizes[3] = namestr.size() + 1;
    strcpy(extraData, namestr.c_str()); // a normal string is always terminated so we don't need strncpy

    // set values of fixed length variables
    mphdata->perturbation = itsMPH.getPolc().getPerturbation();
    mphdata->x0 = itsMPH.getPolc().getX0();
    mphdata->y0 = itsMPH.getPolc().getY0();
    mphdata->domain = itsMPH.getPolc().domain();
    mphdata->sourceNr = itsMPH.getSourceNr();
    mphdata->station = itsMPH.getStation();
    mphdata->isRelPerturbation = itsMPH.getPolc().isRelativePerturbation();
    mphdata->isNormalized = itsMPH.getPolc().isNormalized();

    // adjust database thang to newly calculated values
    set_data(itsBuffer);
    set_size(sizeof(MPHData) + mphdata->stringSizes[0] + mphdata->stringSizes[1] + mphdata->stringSizes[2] + mphdata->stringSizes[3]);
  }
  void ParmTableBDB::MPHValue::updateFromThang()
  {
    char* dataptr = (char*)get_data();
    // we should check here if the amount of data in the buffer is correct
    //	using get_size()
	
    // copy values from buffer to MPH
    MPHData* mphdata = (MPHData *) dataptr;
    // copy values of fixed length variables
    MeqPolc mp;
    mp.setPerturbation(mphdata->perturbation, mphdata->isRelPerturbation);
    mp.setX0(mphdata->x0);
    mp.setY0(mphdata->y0);
    mp.setDomain(mphdata->domain);
    itsMPH.setSourceNr(mphdata->sourceNr);
    itsMPH.setStation(mphdata->station);
    mp.setNormalize(mphdata->isNormalized);
	
    // copy values of variable length
    char* extraData = &dataptr[sizeof(MPHData)];
    mp.setCoeff(string2MeqMatrix(extraData, mphdata->stringSizes[0]));	
    extraData = &extraData[mphdata->stringSizes[0]];
    mp.setSimCoeff(string2MeqMatrix(extraData, mphdata->stringSizes[1]));	
    extraData = &extraData[mphdata->stringSizes[1]];
    mp.setPertSimCoeff(string2MeqMatrix(extraData, mphdata->stringSizes[2]));	
    extraData = &extraData[mphdata->stringSizes[2]];
    itsMPH.setName(extraData);
    itsMPH.setPolc(mp);
  }
  MeqParmHolder ParmTableBDB::MPHValue::getMPH()
  {
    updateFromThang();
    return itsMPH;
  } 
  MeqMatrix ParmTableBDB::MPHValue::string2MeqMatrix(char* str, int length)
  {
    MeqMatrix MM;
    LOFAR::BlobIBufChar bb(str, length);
    LOFAR::BlobIStream bs(bb);
    bs >> MM;

    return MM;
  }
  void ParmTableBDB::MPHValue::MeqMat2string(const MeqMatrix &MM, char* begin, char** end)
  {
    BlobOBufChar bb(begin, 1024, 0);
    BlobOStream bs(bb);
    bs << MM;
    *end = begin + bb.size();
  };
}
