//# parmdb.cc: put values in the database used for MNS
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

#include <MNS/ParmTableAIPS.h>
#include <MNS/ParmTableBDB.h>
#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>
#include <Common/KeyParser.h>
#include <casa/Quanta/MVTime.h>
#include <string>
#include <pwd.h>
#include <unistd.h>

using namespace LOFAR;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;

string dbHost, dbName, dbType, dbUser, tableName;
ParmTableRep* PTR;

enum Command {
  NOCMD,
  SHOW,
  NEW,
  UPDATE,
  DELETE,
  SHOWDEF,
  NEWDEF,
  UPDATEDEF,
  DELETEDEF,
  CONNECT,
  CLEAR,
  CREATE,
  SHOWALLSOURCES,
  SHOWALLDEFSOURCES,
  QUIT
};


Command getCommand (char*& str)
{
  Command cmd = NOCMD;
  while (*str == ' ') {
    str++;
  }
  char* sstr = str;
  while (*str != ' ' && *str != '\0') {
    str++;
  }
  string sc(sstr, str-sstr);
  if (sc == "show"  ||  sc == "list") {
    cmd = SHOW;
  } else if (sc == "new"  ||  sc == "insert"  ||  sc == "add") {
    cmd = NEW;
  } else if (sc == "update") {
    cmd = UPDATE;
  } else if (sc == "delete"  ||  sc == "remove"  || sc == "erase") {
    cmd = DELETE;
  } else if (sc == "showdef"  ||  sc == "listdef") {
    cmd = SHOWDEF;
  } else if (sc == "newdef"  ||  sc == "insertdef"  ||  sc == "adddef") {
    cmd = NEWDEF;
  } else if (sc == "updatedef") {
    cmd = UPDATEDEF;
  } else if (sc == "deletedef"  ||  sc == "removedef"  || sc == "erasedef") {
    cmd = DELETEDEF;
  } else if (sc == "connect") {
    cmd = CONNECT;
  } else if (sc == "clear") {
    cmd = CLEAR;
  } else if (sc == "create") {
    cmd = CREATE;
  } else if (sc == "showAllSources") {
    cmd = SHOWALLSOURCES;
  } else if (sc == "showAllDefSources") {
    cmd = SHOWALLDEFSOURCES;
  } else if (sc == "stop"  ||  sc == "quit"  || sc == "exit") {
    cmd = QUIT;
  } 
  return cmd;
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
		    uint nx, double defaultValue = 1)
{
  KeyValueMap::const_iterator value = kvmap.find(arrName);
  if (value == kvmap.end()) {
    return MeqMatrix (double(defaultValue), 1, 1);
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

MeqDomain getDomain (const KeyValueMap& kvmap, const std::string& arrName)
{
  KeyValueMap::const_iterator value = kvmap.find(arrName);
  if (value == kvmap.end()) {
    MeqDomain MDdef (0, 1, 0, 1);
    return MDdef;
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
  if (vec.size()!=4) {
    cerr<<"wrong number of domain values"<<endl;
    exit(1);
  }
  MeqDomain MD(vec[0], vec[1], vec[2], vec[3]);
  return MD;
}

// IMPLEMENTATION OF THE COMMANDS
void newParm (const std::string& tableName, const std::string& parmName,
	      KeyValueMap& kvmap)
{
  MeqPolc polc;
 
  polc.setSimCoeff (getArray (kvmap, "values", kvmap.getInt("nx", 1)));
  polc.setPertSimCoeff(getArray (kvmap, "absPertValues", kvmap.getInt("nx", 1), 0));
  MeqMatrix MM = polc.getSimCoeff().clone();
  MM += polc.getPertSimCoeff().clone();
  polc.setCoeff(MM);  
  double diff = kvmap.getDouble ("diff", 1e-6);
  bool diffrel = kvmap.getBool ("diff_rel", true);
  polc.setPerturbation (diff, diffrel);
  polc.setX0 (kvmap.getDouble ("freq0", 0.));
  polc.setY0 (kvmap.getDouble ("time0", 0.));
  polc.setXScale (kvmap.getDouble ("freqscale", 1e6));
  polc.setYScale (kvmap.getDouble ("timescale", 1.));
  polc.setDomain (getDomain (kvmap, "domain"));
  //  cout<<"now putting "<<parmName<<endl;
  PTR->putNewCoeff(parmName, polc);
}

void newParmDef (const std::string& tableName, const std::string& parmName,
		 KeyValueMap& kvmap)
{
  MeqPolc polc;
  polc.setSimCoeff (getArray (kvmap, "values", kvmap.getInt("nx", 1)));
  polc.setPertSimCoeff(getArray (kvmap, "absPertValues", kvmap.getInt("nx", 1), 0));
  MeqMatrix MM = polc.getSimCoeff().clone();
  MM += polc.getPertSimCoeff().clone();
  polc.setCoeff(MM);  
  cout<<"simcoeff:"<<endl;
  showArray(cout, polc.getSimCoeff());
  cout<<"coeff:"<<endl;
  showArray(cout, polc.getCoeff());
  cout<<"pertsimcoeff:"<<endl;
  showArray(cout, polc.getPertSimCoeff());
  cout<<endl;
  double diff = kvmap.getDouble ("diff", 1e-6);
  bool diffrel = kvmap.getBool ("diff_rel", true);
  polc.setPerturbation (diff, diffrel);
  polc.setX0 (kvmap.getDouble ("freq0", 0.));
  polc.setY0 (kvmap.getDouble ("time0", 0.));
  polc.setXScale (kvmap.getDouble ("freqscale", 1e6));
  polc.setYScale (kvmap.getDouble ("timescale", 1.));
  PTR->putNewDefCoeff(parmName, polc);
}

void updateDef (const std::string& tableName, const std::string& parmName,
		KeyValueMap& kvmap)
{
  MeqPolc polc;
  polc.setSimCoeff (getArray (kvmap, "values", kvmap.getInt("nx", 1)));
  polc.setPertSimCoeff(getArray (kvmap, "absPertValues", kvmap.getInt("nx", 1), 0));
  MeqMatrix MM = polc.getSimCoeff().clone();
  MM += polc.getPertSimCoeff().clone();
  polc.setCoeff(MM);  
  cout<<"simcoeff:"<<endl;
  showArray(cout, polc.getSimCoeff());
  cout<<"coeff:"<<endl;
  showArray(cout, polc.getCoeff());
  cout<<"pertsimcoeff:"<<endl;
  showArray(cout, polc.getPertSimCoeff());
  cout<<endl;
  double diff = kvmap.getDouble ("diff", 1e-6);
  bool diffrel = kvmap.getBool ("diff_rel", true);
  polc.setPerturbation (diff, diffrel);
  polc.setX0 (kvmap.getDouble ("freq0", 0.));
  polc.setY0 (kvmap.getDouble ("time0", 0.));
  polc.setXScale (kvmap.getDouble ("freqscale", 1e6));
  polc.setYScale (kvmap.getDouble ("timescale", 1.));
  PTR->putDefCoeff(parmName, polc);
}

void ShowDef(string name)
{
  cout<<name<<" : "<<endl;
  MeqPolc MP = PTR->getInitCoeff(name);
  cout<<" Coeff        : "<<MP.getCoeff()<<endl;
  cout<<" Simcoeff     : "<<MP.getSimCoeff()<<endl;
  cout<<" PertSimcoeff : "<<MP.getPertSimCoeff()<<endl;
  cout<<" perturbation : "<<MP.getPerturbation()<<endl;
  cout<<" relperturb   : "<<MP.isRelativePerturbation()<<endl;
}

void Show(string name, MeqDomain &domain)
{
  vector<MeqPolc> VMP = PTR->getPolcs(name, domain);
  for (uint i=0; i<VMP.size(); i++) {
    cout<<name<<" : "<<endl;
    cout<<" coeff        : "<<VMP[i].getCoeff()<<endl;
    cout<<" Simcoeff     : "<<VMP[i].getSimCoeff()<<endl;
    cout<<" PertSimcoeff : "<<VMP[i].getPertSimCoeff()<<endl;
    cout<<" perturbation : "<<VMP[i].getPerturbation()<<endl;
    cout<<" relperturb   : "<<VMP[i].isRelativePerturbation()<<endl;
    MeqDomain md = VMP[i].domain();
    cout<<" domain       : "<<md.startX()<<", "<<md.endX()<<", "<<md.startY()<<", "<<md.endY()<<", "<<endl;
  }
}

void ShowAllSources()
{
  vector<string> names = PTR->getSources();
  cout << "names: "<<names<<endl;
  MeqDomain MD;
  for (uint i=0; i<names.size(); i++) {
    Show(names[i], MD);
  }
}

void ShowAllDefSources()
{
  vector<string> names = PTR->getSources();
  cout << "names: "<<names<<endl;
  for (uint i=0; i<names.size(); i++) {
    ShowDef(names[i]);
  }
}

void doIt()
{
  PTR=0;
  int buffersize=1024;
  char cstra[buffersize];
  // Loop until stop is given.
  while (true) {
    try {
      char* cstr = cstra;
      if (! cin.getline (cstr, buffersize)) {
	cerr << "Error while reading command" << endl;
	break;
      } 
      if (cstr[0] == 0) {
	break;
      }
      Command cmd = getCommand (cstr);
      if (cmd == QUIT) {
	break;
      }
      string parmName;
      if (cmd == CONNECT) {
	// Connect to database
	KeyValueMap kvmap = KeyParser::parse (cstr);
	dbUser = kvmap.getString ("user", getUserName());
	tableName = "MeqParm";
	dbHost = kvmap.getString ("host", "dop50.astron.nl");
	dbName = kvmap.getString ("db", dbUser);
	dbType = kvmap.getString ("dbtype", "postgres");
	tableName = kvmap.getString ("tablename", "MeqParm");
	if (dbType=="aips") {
	  PTR = new ParmTableAIPS (dbUser, tableName);
	} else if (dbType=="bdb") {
	  PTR = new ParmTableBDB (dbUser, tableName);
	} else {
	  cerr<<"Unknown database type: '"<<dbType<<"'"<<endl;
	  exit(1);
	};
	PTR->connect();
	//cout << "Connected to " << dbType << " database " << dbName
	//     << endl;
      } else if (cmd == CLEAR)  {
	// clear dataBase
	PTR->clearTable();
      } else if (cmd == CREATE)  {
	// create dataBase	
	KeyValueMap kvmap = KeyParser::parse (cstr);
	dbUser = kvmap.getString ("user", getUserName());
	tableName = "MeqParm";
	dbHost = kvmap.getString ("host", "dop50.astron.nl");
	dbName = kvmap.getString ("db", dbUser);
	dbType = kvmap.getString ("dbtype", "postgres");
	tableName = kvmap.getString ("tablename", "MeqParm");
	if (dbType=="aips") {
	  ParmTableAIPS::createTable(dbUser, tableName);
	} else if (dbType=="bdb") {
	  ParmTableBDB::createTable(dbUser, tableName);
	} else {
	  cerr<<"Unknown database type: '"<<dbType<<"'"<<endl;
	  exit(1);
	};
      } else if (cmd == SHOWALLSOURCES)  {
	// show all default values for sources
	ASSERTSTR(PTR!=0, "Must have connect info for database before creating it");
	ShowAllSources();
      } else if (cmd == SHOWALLSOURCES)  {
	// show all default values for sources
	ASSERTSTR(PTR!=0, "Must have connect info for database before creating it");
	ShowAllSources();
      } else if (cmd == SHOWALLDEFSOURCES)  {
	// show all default values for sources
	ASSERTSTR(PTR!=0, "Must have connect info for database before creating it");
	ShowAllDefSources();
      } else {
	parmName = getParmName (cstr);
	KeyValueMap kvmap = KeyParser::parse (cstr);
	if (cmd == NEWDEF) {
	  newParmDef (tableName, parmName, kvmap);
	} else if (cmd == NEW) {
	  newParm (tableName, parmName, kvmap);
	} else if (cmd == UPDATEDEF) {
	  updateDef (tableName, parmName, kvmap);
	}
      }
    } catch (std::exception& x) {
      cerr << "Exception: " << x.what() << endl;
    }
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
