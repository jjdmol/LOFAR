//# parmdb.cc: access the parameter database
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

// This program accesses the parm table and default parm table.
// A default parmtable has to be created in the database using:
//   psql -h dop50 -U postgres <user>
//    create table MeqParmDef (ObjID bigint not null unique primary key, Owner bigint not null, VersionNr integer not null, NAME text, SRCNR integer, STATNR integer, VALUES00 double precision, VALUES text, SIM_VALUES text, SIM_PERT text, TIME0 double precision, FREQ0 double precision, NORMALIZED smallint, SOLVABLE text, DIFF double precision, DIFF_REL smallint);
//    create trigger MeqParmDef_updateversionnr before insert on MeqParmDef for each row execute procedure updateversionnr();
// A parmtable has to be created using:
//    create table MeqParm (ObjID bigint not null unique primary key, Owner bigint not null, VersionNr integer not null, NAME text, SRCNR integer, STATNR integer, VALUES00 double precision, VALUES text, SIM_VALUES text, SIM_PERT text, TIME0 double precision, FREQ0 double precision, NORMALIZED smallint, SOLVABLE text, DIFF double precision, DIFF_REL smallint, STARTTIME double precision, ENDTIME double precision, STARTFREQ double precision, ENDFREQ double precision);
//    create trigger MeqParm_updateversionnr before insert on MeqParm for each row execute procedure updateversionnr();
//
// Note that each name can be used instead of the name MeqParm used here.

#include <lofar_config.h>
#include <iostream>

#ifdef HAVE_LOFAR_PL
#include <MNS/TPOParm.h>
#include <PL/PersistenceBroker.h>
#include <PL/QueryObject.h>
#include <PL/Attrib.h>
#include <PL/Collection.h>
#include <Common/KeyValueMap.h>
#include <Common/KeyParser.h>
#include <aips/Quanta/MVTime.h>
#include <string>
#include <pwd.h>

using namespace LOFAR;
using namespace LOFAR::PL;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;

typedef MeqParmHolder MParm;
typedef TPersistentObject<MParm> TPOMParm;
typedef Collection<TPOMParm> MParmSet;

typedef MeqParmDefHolder MParmDef;
typedef TPersistentObject<MParmDef> TPOMParmDef;
typedef Collection<TPOMParmDef> MParmDefSet;

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

void showTime (std::ostream& os, double val)
{
  os << MVTime::Format(MVTime::YMD, 6) << MVTime(val);
}

void showHelp()
{
  cerr << endl;
  cerr << "Show and update contents of database tables containing the" << endl;
  cerr << "ME parameters and their defaults." << endl;
  cerr << " connect db='username' dbtype='postgres' tablename='meqparm'" << endl;
  cerr << " quit  (or exit or stop)" << endl;
  cerr << endl;
  cerr << " show [parmname/pattern]     (default is * (all))" << endl;
  cerr << " add parmname valuespec" << endl;
  cerr << " update parmname/pattern valuespec" << endl;
  cerr << " remove parmname/pattern" << endl;
  cerr << endl;
  cerr << " showdef [parmname/pattern]     (default is * (all))" << endl;
  cerr << " adddef parmname valuespec" << endl;
  cerr << " updatedef parmname/pattern valuespec" << endl;
  cerr << " removedef parmname/pattern" << endl;
  cerr << endl;
  cerr << "  valuespec gives the values of the parameter attributes as" << endl;
  cerr << "   key=value pairs separated by commas." << endl;
  cerr << "  Attributes not given are not changed. Values shown are defaults when adding." << endl;
  cerr << "   starttime='1858/11/17/00:00:00'" << endl;
  cerr << "   timestep=3600         (seconds)" << endl;
  cerr << "   startfreq=100000000   (hz)" << endl;
  cerr << "   freqstep=1000000      (hz)" << endl;
  cerr << "   values=1              (coefficients)" << endl;
  cerr << "    if multiple coefficients, specify as vector and specify nx (defaults to 1)" << endl;
  cerr << "    For example:   values=[1,2,3,4], nx=2" << endl;
  cerr << "   normalize=T           (normalize coefficients?)" << endl;
  cerr << "   diff=1e-6             (perturbation for numerical differentation)" << endl;
  cerr << "   diffrel=T             (perturbation is relative? Use F for angles)" << endl;
  cerr << "   time0=0               (time0 for time calculations)" << endl;
  cerr << "   freq0=0               (freq0 for frequency calculations)" << endl;
  cerr << endl;
  cerr << "  starttime,timestep,startfreq,freqstep are not applicable for parmdef." << endl;
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
  AssertMsg (*str != '\0', "No command given");
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
  AssertMsg (false, "'" << sc << "' is an invalid command");
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
  AssertMsg (nx*ny == vec.size(),
	     "nx*ny mismatches number of values (" << vec.size() << ")");
  MeqMatrix mat (double(0), nx, ny, false);
  memcpy (mat.doubleStorage(), &vec[0], sizeof(double)*vec.size());
  return mat;
}

MParmSet getParms (const std::string& tableName, const std::string& parmName)
{
  TPOMParm tpo;
  tpo.tableName (tableName);
  return tpo.retrieve (attrib(tpo,"name").like (parmName));
}

MParmDefSet getParmsDef (const std::string& tableName,
			 const std::string& parmName)
{
  TPOMParmDef tpo;
  tpo.tableName (tableName+"Def");
  return tpo.retrieve (attrib(tpo,"name").like (parmName));
}

void showParm (const MParm& parm)
{
  cout << parm.getName();
  const MeqDomain& domain = parm.getPolc().domain();
  cout << " time=[";
  showTime(cout, domain.startX());
  cout << '+' << (domain.endX()-domain.startX()) << " sec]";
  cout << " freq=[" << domain.startY()/1e6 << '+'
       << (domain.endY()-domain.startY())/1e6 << " Mhz]";
  cout << " coeff=[";
  showArray (cout, parm.getPolc().getCoeff());
  cout << ']';
  cout << " shape=[" << parm.getPolc().getCoeff().nx()
       << ',' << parm.getPolc().getCoeff().ny() << ']' << endl;
}

void showParm (const MParmDef& parm)
{
  cout << parm.getName();
  cout << " coeff00=" << parm.getPolc().getCoeff().getDouble();
  cout << " shape=[" << parm.getPolc().getCoeff().nx()
       << ',' << parm.getPolc().getCoeff().ny() << ']' << endl;
}

void showParms (const MParmSet& parmSet)
{
  MParmSet::const_iterator iter = parmSet.begin();
  if (parmSet.size() == 1) {
    showParm (iter->data());
  } else {
    cout << "Found " << parmSet.size() << " records" << endl;
    int nr=0;
    for (; iter!=parmSet.end(); iter++, nr++) {
      cout << ' ' << nr << ":  ";
      showParm (iter->data());
    }
  } 
}

void showParms (const MParmDefSet& parmSet)
{
  MParmDefSet::const_iterator iter = parmSet.begin();
  if (parmSet.size() == 1) {
    showParm (iter->data());
  } else {
    cout << "Found " << parmSet.size() << " records" << endl;
    int nr=0;
    for (; iter!=parmSet.end(); iter++, nr++) {
      cout << ' ' << nr << ":  ";
      showParm (iter->data());
    }
  } 
}

void removeParms (const MParmSet& parmSet)
{
  MParmSet::const_iterator iter = parmSet.begin();
  for (; iter!=parmSet.end(); iter++) {
    iter->erase();
  }
  cout << "Removed " << parmSet.size() << " record(s)" << endl;
}

void removeParms (const MParmDefSet& parmSet)
{
  MParmDefSet::const_iterator iter = parmSet.begin();
  for (; iter!=parmSet.end(); iter++) {
    iter->erase();
  }
  cout << "Removed " << parmSet.size() << " record(s)" << endl;
}

void updateParm (TPOMParm& tpoparm, const KeyValueMap& kvmap)
{
  MParm& parm = tpoparm.data();
  parm.setSourceNr (kvmap.getInt ("srcnr", parm.getSourceNr()));
  parm.setStation (kvmap.getInt ("statnr", parm.getStation()));
  MeqPolc polc = parm.getPolc();
  const MeqDomain& domain = polc.domain();
  double stime = domain.startX();
  double etime = domain.endX() - stime;
  string str;
  str = kvmap.getString ("starttime", str);
  if (!str.empty()) {
    Quantity time;
    AssertMsg (MVTime::read(time, str, true),
	       str << " is an invalid date/time");
    stime = MVTime(time);
  }
  etime = stime + kvmap.getDouble("timestep", etime);
  double sfreq = domain.startY();
  double efreq = domain.endY() - sfreq;
  sfreq = kvmap.getDouble("startfreq", sfreq);
  efreq = sfreq + kvmap.getDouble("freqstep", efreq);
  polc.setDomain (MeqDomain(stime, etime, sfreq, efreq));
  if (kvmap.isDefined("values")) {
    polc.setCoeff (getArray (kvmap, "values", kvmap.getInt("nx", 1)));
    polc.setSimCoeff (polc.getCoeff().clone());
    MeqMatrix mat(double(0), polc.getCoeff().nx(), polc.getCoeff().ny());
    polc.setPertSimCoeff (mat);
  }
  polc.setNormalize (kvmap.getBool ("normalize", polc.isNormalized()));
  double diff = kvmap.getDouble ("diff", polc.getPerturbation());
  bool diffrel = kvmap.getBool ("diffrel", polc.isRelativePerturbation());
  polc.setPerturbation (diff, diffrel);
  polc.setX0 (kvmap.getDouble ("time0", polc.getX0()));
  polc.setY0 (kvmap.getDouble ("freq0", polc.getY0()));
  parm.setPolc (polc);
  tpoparm.update();
  cout << "Updated record for parameter " << parm.getName() << endl;
}

void updateParm (TPOMParmDef& tpoparm, const KeyValueMap& kvmap)
{
  MParmDef& parm = tpoparm.data();
  parm.setSourceNr (kvmap.getInt ("srcnr", parm.getSourceNr()));
  parm.setStation (kvmap.getInt ("statnr", parm.getStation()));
  MeqPolc polc = parm.getPolc();
  if (kvmap.isDefined("values")) {
    polc.setCoeff (getArray (kvmap, "values", kvmap.getInt("nx", 1)));
    polc.setSimCoeff (polc.getCoeff().clone());
    MeqMatrix mat(double(0), polc.getCoeff().nx(), polc.getCoeff().ny());
    polc.setPertSimCoeff (mat);
  }
  polc.setNormalize (kvmap.getBool ("normalize", polc.isNormalized()));
  double diff = kvmap.getDouble ("diff", polc.getPerturbation());
  bool diffrel = kvmap.getBool ("diffrel", polc.isRelativePerturbation());
  polc.setPerturbation (diff, diffrel);
  polc.setX0 (kvmap.getDouble ("time0", polc.getX0()));
  polc.setY0 (kvmap.getDouble ("freq0", polc.getY0()));
  parm.setPolc (polc);
  tpoparm.update();
  cout << "Updated record for parameter " << parm.getName() << endl;
}

void updateParms (MParmSet& parmSet, KeyValueMap& kvmap)
{
  MParmSet::iterator iter = parmSet.begin();
  for (; iter!=parmSet.end(); iter++) {
    updateParm (*iter, kvmap);
  }
}

void updateParms (MParmDefSet& parmSet, KeyValueMap& kvmap)
{
  MParmDefSet::iterator iter = parmSet.begin();
  for (; iter!=parmSet.end(); iter++) {
    updateParm (*iter, kvmap);
  }
}

void newParm (const std::string& tableName, const std::string& parmName,
	      KeyValueMap& kvmap)
{
  int srcnr = kvmap.getInt ("srcnr", -1);
  int statnr = kvmap.getInt ("statnr", -1);
  MeqPolc polc;
  double stime;
  string str;
  str = kvmap.getString ("starttime", str);
  if (str.empty()) {
    stime = MVTime();
  } else {
    Quantity time;
    AssertMsg (MVTime::read(time, str, true),
	       str << " is an invalid date/time");
    stime = MVTime(time);
  }
  double etime = stime + kvmap.getDouble("timestep", 3600);
  double sfreq = kvmap.getDouble("startfreq", 100e6);
  double efreq = sfreq + kvmap.getDouble("freqstep", 1e6);
  polc.setDomain (MeqDomain(stime, etime, sfreq, efreq));
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
  MParm parm(parmName, srcnr, statnr, polc);
  showParm (parm);
  TPOMParm tpoparm(parm);
  tpoparm.tableName (tableName);
  tpoparm.insert();
  cout << "Inserted new record for parameter " << parm.getName() << endl;
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
  MParmDef parm(parmName, srcnr, statnr, polc);
  showParm (parm);
  TPOMParmDef tpoparm(parm);
  tpoparm.tableName (tableName+"Def");
  tpoparm.insert();
  cout << "Inserted new record for parameter " << parm.getName() << endl;
}

void doIt()
{
  PersistenceBroker pb;
  char cstra[1024];
  char* cstr = cstra;
  std::string dbUser = getUserName();
  std::string tableName = "MeqParm";
  // Loop until stop is given.
  while (true) {
    try {
      cout << "Command: ";
      if (! cin.getline (cstr, sizeof(cstra))) {
	cerr << "Error while reading command" << endl;
	break;
      } 
      if (cstr[0] == 0) {
	break;
      }
      if (cstr[0] == '?') {
	showHelp();
      } else {
	int command = getCommand (cstr);
	if (command == 0) {
	  break;
	}
	string parmName;
	if (command == 5) {
	  // Connect to database
	  KeyValueMap kvmap = KeyParser::parse (cstr);
	  std::string dbName = kvmap.getString ("db", dbUser);
	  std::string dbType = kvmap.getString ("dbtype", "postgres");
	  tableName = kvmap.getString ("tablename", "MeqParm");
	  pb.connect(dbName, dbType, "");
	  cout << "Connected to " << dbType << " database " << dbName
	       << endl;
	} else {
	  parmName = getParmName (cstr);
	  if (parmName.empty()) {
	    AssertMsg (command%10==1, "No parameter name given");
	    parmName = "*";           // default for show is all parms
	  }
	  KeyValueMap kvmap = KeyParser::parse (cstr);
	  if (command < 10) {
	    MParmSet parmSet = getParms (tableName, parmName);
	    if (command == 2) {
	      if (! parmSet.empty()) {
		cout << "The following domains already exist for parameter "
		     << parmName << endl;
		showParms (parmSet);
	      }
	      newParm (tableName, parmName, kvmap);
	    } else {
	      if (parmSet.empty()) {
		cout << "Parameter(s) " << parmName << " unknown" << endl;
	      } else {
		if (command == 1) {
		  showParms (parmSet);
		} else if (command == 4) {
		  removeParms (parmSet);
		} else {
		  updateParms (parmSet, kvmap);
		}
	      }
	    }
	  } else {
	    MParmDefSet parmSet = getParmsDef (tableName, parmName);
	    if (command == 12) {
	      AssertMsg (parmSet.empty(), "Parameter " << parmName
			 << " already exists");
	      newParmDef (tableName, parmName, kvmap);
	    } else {
	      if (parmSet.empty()) {
		cout << "Parameter(s) " << parmName << " unknown" << endl;
	      } else {
		if (command == 11) {
		  showParms (parmSet);
		} else if (command == 14) {
		  removeParms (parmSet);
		} else {
		  updateParms (parmSet, kvmap);
		}
	      }
	    }
	  }
	}
      }
    } catch (std::exception& x) {
      cerr << "Exception: " << x.what() << endl;
    }
  }
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

#else
int main()
{
  std::cerr << "No database access compiled in; parmdb cannot be run"
	    << std::endl;
  return 0;
}
#endif
