//# fillsqldb.cc: put values in the database used for MNS
//#
//# Copyright (C) 2004
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
#include <iostream>

#include <Common/KeyValueMap.h>
#include <Common/KeyParser.h>
#include <casa/Quanta/MVTime.h>
#include <string>
#include <pwd.h>
#include <MNS/ParmTablePGSQL.h>
#include <MNS/ParmTableMySQL.h>
#include <MNS/ParmTableMonet.h>
#include <MNS/ParmTableAIPS.h>

using namespace LOFAR;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;

typedef MeqParmHolder MParm;
typedef vector<MParm> MParmSet;

typedef MeqParmDefHolder MParmDef;
typedef vector<MParmDef> MParmDefSet;

string dbHost, dbName, dbType, dbUser, tableName;
ParmTableFiller* PTR;

char bool2char (bool val)
{
  return (val  ?  'y' : 'n');
}

void showArray (std::ostream& os, const MeqMatrix& mat)
{
  int n = mat.nelements();
  if (n > 0) {
    const double* val = mat.doubleStorage();
    os << *val;
    for (int i=1; i<n; i++) {
      os << ',' << val[i];
    }
  }
}

std::string getUserName()
{
  passwd* aPwd;
  if ((aPwd = getpwuid(getuid())) == 0) {
    return "";
  }
  return aPwd->pw_name;
}

int getCommand (char*& str)
{
  while (*str == ' ') {
    str++;
  }
  char* sstr = str;
  while (*str != ' ' && *str != '\0') {
    str++;
  }
  string sc(sstr, str-sstr);
  if (sc == "show"  ||  sc == "list") {
    return 1;
  } else if (sc == "new"  ||  sc == "insert"  ||  sc == "add") {
    return 2;
  } else if (sc == "update") {
    return 3;
  } else if (sc == "delete"  ||  sc == "remove"  || sc == "erase") {
    return 4;
  } else if (sc == "showdef"  ||  sc == "listdef") {
    return 11;
  } else if (sc == "newdef"  ||  sc == "insertdef"  ||  sc == "adddef") {
    return 12;
  } else if (sc == "updatedef") {
    return 13;
  } else if (sc == "deletedef"  ||  sc == "removedef"  || sc == "erasedef") {
    return 14;
  } else if (sc == "connect") {
    return 5;
  } else if (sc == "stop"  ||  sc == "quit"  || sc == "exit") {
    return 0;
  } 
  return 0;
}

std::string getParmName (char*& str)
{
  while (*str == ' ') {
    str++;
  }
  if (*str == '\0') {
    return "";
  }
  char* sstr = str;
  while (*str != ' ' && *str != '\0') {
    str++;
  }
  return std::string(sstr, str-sstr);
}

MeqMatrix getArray (const KeyValueMap& kvmap, const std::string& arrName,
		    uint nx)
{
  KeyValueMap::const_iterator value = kvmap.find(arrName);
  if (value == kvmap.end()) {
    return MeqMatrix (double(1), 1, 1);
  }
  vector<double> vec;
  if (value->second.dataType() == KeyValue::DTValueVector) {
    const vector<KeyValue>& vvec = value->second.getVector();
    for (uint i=0; i<vvec.size(); i++) {
      vec.push_back (vvec[i].getDouble());
    }
  } else {
    value->second.get (vec);
  }
  uint ny = vec.size() / nx;
  MeqMatrix mat (double(0), nx, ny, false);
  memcpy (mat.doubleStorage(), &vec[0], sizeof(double)*vec.size());
  return mat;
}

void newParmDef (const std::string& tableName, const std::string& parmName,
		 KeyValueMap& kvmap)
{
  int srcnr = kvmap.getInt ("srcnr", -1);
  int statnr = kvmap.getInt ("statnr", -1);
  MeqPolc polc;
  polc.setCoeff (getArray (kvmap, "values", kvmap.getInt("nx", 1)));
  polc.setSimCoeff (polc.getCoeff().clone());
  MeqMatrix mat(double(0), polc.getCoeff().nx(), polc.getCoeff().ny());
  polc.setPertSimCoeff (mat);
  polc.setNormalize (kvmap.getBool ("normalize", true));
  double diff = kvmap.getDouble ("diff", 1e-6);
  bool diffrel = kvmap.getBool ("diffrel", true);
  polc.setPerturbation (diff, diffrel);
  polc.setX0 (kvmap.getDouble ("time0", 0.));
  polc.setY0 (kvmap.getDouble ("freq0", 0.));

  PTR->putNewDefCoeff(parmName, srcnr, statnr, polc);
}

void doIt()
{
  PTR=0;
  char cstra[1024];
  char* cstr = cstra;
  // Loop until stop is given.
  while (true) {
    //    try {
      if (! cin.getline (cstr, sizeof(cstra))) {
	cerr << "Error while reading command" << endl;
	break;
      } 
      if (cstr[0] == 0) {
	break;
      }
      int command = getCommand (cstr);
      if (command == 0) {
	break;
      }
      string parmName;
      if (command == 5) {
	// Connect to database
	KeyValueMap kvmap = KeyParser::parse (cstr);
	dbUser = kvmap.getString ("user", getUserName());
	tableName = "MeqParm";
	dbHost = kvmap.getString ("host", "dop50");
	dbName = kvmap.getString ("db", dbUser);
	dbType = kvmap.getString ("dbtype", "postgres");
	tableName = kvmap.getString ("tablename", "MeqParm");
	if (dbType=="postgres"){
	  PTR = new ParmTablePGSQL (dbHost, dbUser, tableName+"def");
	} else if (dbType=="mysql") {
	  PTR = new ParmTableMySQL (dbHost, dbUser, tableName+"def");
	} else if (dbType=="monet") {
	  PTR = new ParmTableMonet (dbHost, dbUser, tableName+"def", true);
	} else if (dbType=="aips") {
	  PTR = new ParmTableAIPS (tableName);
	} else {
	  cerr<<"Unknown database type: '"<<dbType<<"'"<<endl;
	  exit(1);
	};
	cout << "Connected to " << dbType << " database " << dbName
	     << endl;
      } else {
	parmName = getParmName (cstr);
	KeyValueMap kvmap = KeyParser::parse (cstr);
	if (command == 12) {
	  newParmDef (tableName, parmName, kvmap);
	}
	      }
      //    } catch (std::exception& x) {
      //      cerr << "Exception: " << x.what() << endl;
      //    }
  }
  delete PTR;
}

int main()
{
  try {
    doIt();
  } catch (std::exception& x) {
    std::cerr << "Caught exception: " << x.what() << std::endl;
    return 1;
  }
  return 0;
}
