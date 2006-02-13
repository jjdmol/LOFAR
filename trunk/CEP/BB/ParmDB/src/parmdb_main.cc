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

#include <ParmDB/ParmDB.h>
#include <Blob/KeyValueMap.h>
#include <Blob/KeyParser.h>
#include <Common/VectorUtil.h>
#include <Common/LofarLogger.h>

#include <casa/Quanta/MVTime.h>
#include <casa/Containers/Block.h>
#include <iostream>
#include <string>
#include <pwd.h>
#include <unistd.h>

using namespace std;
using namespace casa;
using namespace LOFAR;
using namespace ParmDB;

LOFAR::ParmDB::ParmDB* parmtab;

enum PTCommand {
  NOCMD,
  OPEN,
  CREATE,
  CLOSE,
  CLEAR,
  SHOW,
  NAMES,
  NEW,
  UPD,
  DEL,
  SHOWDEF,
  NAMESDEF,
  NEWDEF,
  UPDDEF,
  DELDEF,
  SHOWOLD,
  NAMESOLD,
  UPDOLD,
  DELOLD,
  TOOLD,
  QUIT
};

char bool2char (bool val)
{
  return (val  ?  'y' : 'n');
}

void showValues (std::ostream& os, const vector<double>& values,
		 const vector<bool>& mask, const vector<int>& shape)
{
  int n = values.size();
  if (n > 0) {
    os << values[0];
    for (int i=1; i<n; i++) {
      os << ',' << values[i];
    }
  }
  os << "  shape=" << shape;
  if (n > 0) {
    os << "  mask=" << mask[0];
    for (int i=1; i<n; i++) {
      os << ',' << mask[i];
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
  cerr << "Show and update contents of parameter tables containing the" << endl;
  cerr << "ME parameters and their defaults." << endl;
  cerr << " create db='username' dbtype='aips' tablename='meqparm'" << endl;
  cerr << " open   db='username' dbtype='aips' tablename='meqparm'" << endl;
  cerr << " quit  (or exit or stop)" << endl;
  cerr << endl;
  cerr << " showdef [parmname_pattern]" << endl;
  cerr << " namesdef [parmname_pattern]" << endl;
  cerr << " adddef parmname valuespec" << endl;
  cerr << " updatedef parmname_pattern valuespec" << endl;
  cerr << " removedef parmname_pattern" << endl;
  cerr << endl;
  cerr << " show [parmname_pattern] [domain=...] [parentid=...]" << endl;
  cerr << " names [parmname_pattern]" << endl;
  cerr << " add parmname valuespec" << endl;
  cerr << " update parmname_pattern [domain=] valuespec" << endl;
  cerr << " remove parmname_pattern [domain=]" << endl;
  cerr << endl;
  cerr << " toold parmname_pattern [domain=] [newparentid=]" << endl;
  cerr << endl;
  cerr << " showold [parmname_pattern] [domain=...] [parentid=...]" << endl;
  cerr << " namesold [parmname_pattern]" << endl;
  cerr << " updateold parmname_pattern [domain=] valuespec" << endl;
  cerr << " removeold parmname_pattern [domain=]" << endl;
  cerr << endl;
  cerr << "  valuespec gives the values of the parameter attributes as" << endl;
  cerr << "   key=value pairs separated by commas." << endl;
  cerr << "  Attributes not given are not changed. Values shown are defaults when adding." << endl;
  cerr << "   starttime='1858/11/17/00:00:00'" << endl;
  cerr << "   timestep=1      (seconds)" << endl;
  cerr << "   startfreq=0     (MHz)" << endl;
  cerr << "   freqstep=1      (Hz)" << endl;
  cerr << "   values=1              (coefficients)" << endl;
  cerr << "    if multiple coefficients, specify as vector and specify shape" << endl;
  cerr << "    For example:   values=[1,2,3,4], nx=2" << endl;
  cerr << "    For example:   values=[1,2,3,4], shape=[1,1,2,2]" << endl;
  cerr << "   pert=1e-6             (perturbation for numerical differentation)" << endl;
  cerr << "   pertrel=T             (perturbation is relative? Use F for angles)" << endl;
  cerr << "   offset=[]             (offset for each values dimension)" << endl;
  cerr << "   scale=[]              (scale for each values dimension)" << endl;
  cerr << endl;
}

PTCommand getCommand (char*& str)
{
  PTCommand cmd = NOCMD;
  while (*str == ' ') {
    str++;
  }
  const char* sstr = str;
  while (*str != ' ' && *str != '\0') {
    str++;
  }
  string sc(sstr, str-sstr);
  if (sc == "show"  ||  sc == "list") {
    cmd = SHOW;
  } else if (sc == "names") {
    cmd = NAMES;
  } else if (sc == "new"  ||  sc == "insert"  ||  sc == "add") {
    cmd = NEW;
  } else if (sc == "update") {
    cmd = UPD;
  } else if (sc == "delete"  ||  sc == "remove"  || sc == "erase") {
    cmd = DEL;
  } else if (sc == "showdef"  ||  sc == "listdef") {
    cmd = SHOWDEF;
  } else if (sc == "namesdef") {
    cmd = NAMESDEF;
  } else if (sc == "newdef"  ||  sc == "insertdef"  ||  sc == "adddef") {
    cmd = NEWDEF;
  } else if (sc == "updatedef") {
    cmd = UPDDEF;
  } else if (sc == "deletedef"  ||  sc == "removedef"  || sc == "erasedef") {
    cmd = DELDEF;
  } else if (sc == "showold"  ||  sc == "listold") {
    cmd = SHOWOLD;
  } else if (sc == "namesold") {
    cmd = NAMESOLD;
  } else if (sc == "updateold") {
    cmd = UPDOLD;
  } else if (sc == "deleteold"  ||  sc == "removeold"  || sc == "eraseold") {
    cmd = DELOLD;
  } else if (sc == "toold") {
    cmd = TOOLD;
  } else if (sc == "open") {
    cmd = OPEN;
  } else if (sc == "clear") {
    cmd = CLEAR;
  } else if (sc == "create") {
    cmd = CREATE;
  } else if (sc == "stop"  ||  sc == "quit"  || sc == "exit") {
    cmd = QUIT;
  } 
  return cmd;
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
  bool foundeq = false;
  while (*str != ' ' && *str != '\0') {
    if (*str == '=') {
      foundeq = true;
      break;
    }
    str++;
  }
  // If an = was found, no parm is given.
  if (foundeq) {
    str = sstr;
    return "";
  }
  return std::string(sstr, str-sstr);
}

// Get the shape and return the size.
int getShape (const KeyValueMap& kvmap, vector<int>& shape)
{
  KeyValueMap::const_iterator value = kvmap.find("shape");
  if (value != kvmap.end()) {
    shape = value->second.getVecInt();
  } else {
    shape.resize (2);
    shape[0] = kvmap.getInt ("nx", 1);
    shape[1] = kvmap.getInt ("ny", 1);
  }
  int nr = 1;
  for (uint i=0; i<shape.size(); ++i) {
    ASSERTSTR (shape[i] > 0, "a shape value must be > 0");
    nr *= shape[i];
  }
  return nr;
}

vector<double> getArray (const KeyValueMap& kvmap, const std::string& arrName,
			 uint size=0, double defaultValue = 1)
{
  vector<double> res(size, 0.);
  KeyValueMap::const_iterator value = kvmap.find(arrName);
  if (value == kvmap.end()) {
    if (size > 0) res[0] = defaultValue;
    return res;
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
  if (size == 0) {
    return vec;
  }
  ASSERTSTR (vec.size() <= size, "More than "<<size<<" values are given");
  for (uint i=0; i<vec.size(); ++i) {
    res[i] = vec[i];
  }
  return res;
}

// Return as Block instead of vector, because vector<bool> uses bits.
Block<bool> getMask (const KeyValueMap& kvmap, const std::string& arrName,
		     uint size)
{
  Block<bool> res(size, false);
  KeyValueMap::const_iterator value = kvmap.find(arrName);
  if (value == kvmap.end()) {
    if (size > 0) res[0] = true;
    return res;
  }
  vector<double> vec;
  if (value->second.dataType() == KeyValue::DTValueVector) {
    const vector<KeyValue>& vvec = value->second.getVector();
    for (uint i=0; i<vvec.size(); i++) {
      vec.push_back (vvec[i].getBool());
    }
  } else {
    value->second.get (vec);
  }
  ASSERTSTR (vec.size() <= size, "More than "<<size<<" mask values are given");
  for (uint i=0; i<vec.size(); ++i) {
    res[i] = vec[i];
  }
  return res;
}

ParmDomain getDomain (const KeyValueMap& kvmap, int size=0)
{
  KeyValueMap::const_iterator value = kvmap.find("domain");
  vector<double> st;
  vector<double> end;
  if (value != kvmap.end()) {
    bool ok = false;
    // See if given as domain=[start=[],end=[] or step=[]].
    if (value->second.dataType() == KeyValue::DTValueMap) {
      const KeyValueMap& kvm = value->second.getValueMap();
      KeyValueMap::const_iterator key = kvm.find("st");
      if (key != kvm.end()) {
	st = key->second.getVecDouble();
	key = kvm.find("end");
	bool hasend = (key != kvm.end());
	if (hasend) end = key->second.getVecDouble();
	key = kvm.find("size");
	bool hassize = (key != kvm.end());
	if (hassize) end = key->second.getVecDouble();
	// end and size cannot be given both.
	if (hasend != hassize) {
	  if (st.size() == end.size()) {
	    ok = true;
	    if (hassize) {
	      for (uint i=0; i<end.size(); ++i) {
		end[i] += st[i];
	      }
	    }
	  }
	}
      }
    } else {
      // Given as a vector of doubles (as stx,endx,sty,endy,...).
      ok = true;
      vector<double> vec;
      try {
	vec = value->second.getVecDouble();
      } catch (...) {
	ok = false;
      }
      if (ok) {
	if (vec.size() % 2 != 0) {
	  ok = false;
	} else {
	  int nr = vec.size() / 2;
	  st.resize(nr);
	  end.resize(nr);
	  for (int i=0; i<nr; ++i) {
	    st[i] = vec[2*i];
	    end[i] = vec[2*i + 1];
	  }
	}
      }
    }
    ASSERTSTR (ok,
	       "domain must be given as [[stx,endx],[sty,endy],...]\n"
	       "or as [st=[stx,...],end=[endx,...] or size=[sizex,...]]");
  }
  if (size > 0) {
    ASSERTSTR (int(st.size()) <= size, "Domain has too many axes");
    int nr = size - st.size();
    for (int i=0; i<nr; ++i) {
      st.push_back (0);
      end.push_back (1);
    }
  }
  return ParmDomain(st, end);
}

void showDomain (const ParmDomain& domain)
{
  const vector<double>& st = domain.getStart();
  const vector<double>& end = domain.getEnd();
  if (st.size() == 2) {
    cout << " freq=[" << st[0]/1e6 << " MHz +"
	 << (end[1]-st[0])/1e3 << " KHz]";
    cout << " time=[";
    showTime(cout, st[1]);
    cout << " +" << (end[1]-st[1]) << " sec]";
  } else {
    for (uint i=0; i<st.size(); ++i) {
      if (i != 0) cout << ',';
      cout << '[' << st[i] << ',' << end[i] << ']';
    }
  }
}

// IMPLEMENTATION OF THE COMMANDS
void showNames (const string& pattern, PTCommand type)
{
  vector<string> names;
  if (type == NAMES) {
    names = parmtab->getNames (pattern, ParmDBRep::UseNormal);
  } else if (type == NAMESOLD) {
    names = parmtab->getNames (pattern, ParmDBRep::UseHistory);
  } else {
    map<string,ParmValueSet> parmset;
    parmtab->getDefValues (parmset, pattern);
    for (map<string,ParmValueSet>::const_iterator iter = parmset.begin();
	 iter != parmset.end();
	 iter++) {
      names.push_back (iter->first);
    }
  }
  cout << "names: " << names << endl;
}

void showParm (const string& parmName, const ParmValueRep& parm, bool showAll)
{
  cout << parmName;
  cout << "  type=" << parm.itsType;
  if (parm.itsConstants.size() > 0) {
    cout << " constants=" << parm.itsConstants;
  }
  cout << endl;
  if (showAll) {
    cout << "    domain: ";
    showDomain (parm.itsDomain);
    cout << endl;
    cout << "    offset=" << parm.itsOffset << " scale=" << parm.itsScale
	 << endl;
  }
  cout << "    values:  ";
  showValues (cout, parm.itsCoeff, parm.itsSolvMask, parm.itsShape);
  cout << endl;
  cout << "    pert=" << parm.itsPerturbation
       << " pert_rel=" << bool2char(parm.itsIsRelPert) << endl;
  if (showAll) {
    cout << "    weight=" << parm.itsWeight << "  ID=" << parm.itsID
	 << " parentID=" << parm.itsParentID << endl;
  }
}

void showParms (map<string,ParmValueSet>& parmSet, bool showAll)
{
  int nr=0;
  for (map<string,ParmValueSet>::iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    vector<ParmValue>& vec = iter->second.getValues();
    const string& parmName = iter->first;
    for (uint i=0; i<vec.size(); ++i) {
      showParm (parmName, vec[i].rep(), showAll);
      nr++;
    }
  }
  if (nr != 1) {
    cout << nr << " parms found" << endl;
  }
}

int countParms (map<string,ParmValueSet>& parmSet)
{
  int nr=0;
  for (map<string,ParmValueSet>::const_iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    nr += iter->second.getValues().size();
  }
  return nr;
}

void newParm (const std::string& parmName, const KeyValueMap& kvmap)
{
  ParmValue pvalue;
  ParmValueRep& pval = pvalue.rep();
  // Set funklet type with possible constants.
  pval.setType (kvmap.getString("type", "polc"), getArray(kvmap, "constants"));
  vector<int> shape;
  // Get the coefficients shape and the data.
  int size = getShape (kvmap, shape);
  vector<double> coeff = getArray (kvmap, "values", size);
  if (kvmap.isDefined("mask")) {
    Block<bool> mask = getMask (kvmap, "mask", size);
    pval.setCoeff (&coeff[0], mask.storage(), shape);
  } else {
    pval.setCoeff (&coeff[0], shape);
  }
  // Set perturbation for numerical derivatives.
  double pert = kvmap.getDouble ("pert", 1e-6);
  bool pertrel = kvmap.getBool ("pert_rel", true);
  pval.setPerturbation (pert, pertrel);
  // Set domain and default offset/scale.
  pval.setDomain (getDomain (kvmap, shape.size()));
  // Only set offset/scale if really given.
  // If given, make sure their lengths equal #dim.
  if (getArray(kvmap, "offset").size() > 0) {
    pval.itsOffset = getArray (kvmap, "offset", shape.size(), 1);
  }
  if (getArray(kvmap, "scale").size() > 0) {
    pval.itsScale = getArray (kvmap, "scale", shape.size(), 1);
  }
  // Set weight.
  pval.itsWeight = kvmap.getDouble ("weight", 1);
  showParm (parmName, pval, true);
  parmtab->putValue (parmName, pvalue);
  cout << "Wrote new record for parameter " << parmName << endl;
}

void newDefParm (const std::string& parmName, KeyValueMap& kvmap)
{
  ParmValue pvalue;
  ParmValueRep& pval = pvalue.rep();
  // Set funklet type with possible constants.
  pval.setType (kvmap.getString("type", "polc"), getArray(kvmap, "constants"));
  vector<int> shape;
  // Get the coefficients shape and the data.
  int size = getShape (kvmap, shape);
  vector<double> coeff = getArray (kvmap, "values", size);
  if (kvmap.isDefined("mask")) {
    Block<bool> mask = getMask (kvmap, "mask", size);
    pval.setCoeff (&coeff[0], mask.storage(), shape);
  } else {
    pval.setCoeff (&coeff[0], shape);
  }
  // Set perturbation for numerical derivatives.
  double pert = kvmap.getDouble ("pert", 1e-6);
  bool pertrel = kvmap.getBool ("pert_rel", true);
  pval.setPerturbation (pert, pertrel);
  showParm (parmName, pval, false);
  parmtab->putDefValue (parmName, pvalue);
  cout << "Wrote new def record for parameter " << parmName << endl;
}

void updateParm (const string& parmName, ParmValue& pvalue,
		 KeyValueMap& kvmap)
{
  ParmValueRep& pval = pvalue.rep();
  // Set funklet type with possible constants.
  if (kvmap.isDefined("type")) {
      pval.itsType = kvmap.getString("type", "polc");
  }
  if (kvmap.isDefined("constants")) {
    pval.itsConstants = getArray(kvmap, "constants");
  }
  if (kvmap.isDefined("values")) {
    vector<int> shape;
    // Get the coefficients shape and the data.
    int size = getShape (kvmap, shape);
    vector<double> coeff = getArray (kvmap, "values", size);
    if (kvmap.isDefined("mask")) {
      Block<bool> mask = getMask (kvmap, "mask", size);
      pval.setCoeff (&coeff[0], mask.storage(), shape);
    } else {
      pval.setCoeff (&coeff[0], shape);
    }
  }
  // Set perturbation for numerical derivatives.
  if (kvmap.isDefined("pert")) {
    double pert = kvmap.getDouble ("pert", 1e-6);
    bool pertrel = kvmap.getBool ("pert_rel", true);
    pval.setPerturbation (pert, pertrel);
  }
  // Set offset/scale if given.
  // If given, make sure their lengths equal #dim.
  if (getArray(kvmap, "offset").size() > 0) {
    pval.itsOffset = getArray (kvmap, "offset", pval.itsShape.size(), 1);
  }
  if (getArray(kvmap, "scale").size() > 0) {
    pval.itsScale = getArray (kvmap, "scale", pval.itsShape.size(), 1);
  }
  // Set weight.
  if (kvmap.isDefined("weight")) {
    pval.itsWeight = kvmap.getDouble ("weight", 1);
  }
  if (kvmap.isDefined("id")) {
    pval.itsID = kvmap.getInt ("id", 0);
  }
  if (kvmap.isDefined("parentid")) {
    pval.itsParentID = kvmap.getInt ("parentid", 0);
  }
  showParm (parmName, pval, true);
}

void updateParms (map<string,ParmValueSet>& parmSet, KeyValueMap& kvmap,
		  ParmDBRep::TableType ttype)
{
  for (map<string,ParmValueSet>::iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    vector<ParmValue>& vec = iter->second.getValues();
    const string& parmName = iter->first;
    for (uint i=0; i<vec.size(); ++i) {
      updateParm (parmName, vec[i], kvmap);
    }
  }
  parmtab->putValues (parmSet, ttype);
}

void moveParms (const string& parmName, const ParmDomain& domain, int parentId,
		map<string,ParmValueSet>& parmSet, KeyValueMap& kvmap)
{
  int newparid = kvmap.getInt ("newparentid", -1);
  for (map<string,ParmValueSet>::iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    vector<ParmValue>& vec = iter->second.getValues();
    for (uint i=0; i<vec.size(); ++i) {
      if (newparid >= 0) {
	vec[i].rep().itsParentID = newparid;
      }
    }
  }
  parmtab->putValues (parmSet, ParmDBRep::UseHistory);
  parmtab->deleteValues (parmName, domain, parentId);
}

void updateDefParm (const string& parmName, ParmValue& pvalue,
		    KeyValueMap& kvmap)
{
  ParmValueRep& pval = pvalue.rep();
  // Set funklet type with possible constants.
  if (kvmap.isDefined("type")) {
      pval.itsType = kvmap.getString("type", "polc");
  }
  if (kvmap.isDefined("constants")) {
    pval.itsConstants = getArray(kvmap, "constants");
  }
  if (kvmap.isDefined("values")) {
    vector<int> shape;
    // Get the coefficients shape and the data.
    int size = getShape (kvmap, shape);
    vector<double> coeff = getArray (kvmap, "values", size);
    if (kvmap.isDefined("mask")) {
      Block<bool> mask = getMask (kvmap, "mask", size);
      pval.setCoeff (&coeff[0], mask.storage(), shape);
    } else {
      pval.setCoeff (&coeff[0], shape);
    }
  }
  // Set perturbation for numerical derivatives.
  if (kvmap.isDefined("pert")) {
    double pert = kvmap.getDouble ("pert", 1e-6);
    bool pertrel = kvmap.getBool ("pert_rel", true);
    pval.setPerturbation (pert, pertrel);
  }
  // Set offset/scale if given.
  // If given, make sure their lengths equal #dim.
  if (getArray(kvmap, "offset").size() > 0) {
    pval.itsOffset = getArray (kvmap, "offset", pval.itsShape.size(), 1);
  }
  if (getArray(kvmap, "scale").size() > 0) {
    pval.itsScale = getArray (kvmap, "scale", pval.itsShape.size(), 1);
  }
  // Set weight.
  if (kvmap.isDefined("weight")) {
    pval.itsWeight = kvmap.getDouble ("weight", 1);
  }
  showParm (parmName, pval, false);
  parmtab->putValue (parmName, pvalue);
}

void updateDefParms (map<string,ParmValueSet>& parmSet, KeyValueMap& kvmap)
{
  for (map<string,ParmValueSet>::iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    vector<ParmValue>& vec = iter->second.getValues();
    const string& parmName = iter->first;
    for (uint i=0; i<vec.size(); ++i) {
      updateDefParm (parmName, vec[i], kvmap);
    }
  }
}


void doIt (bool noPrompt)
{
  parmtab = 0;
  int buffersize = 1024;
  char cstra[buffersize];
  // Loop until stop is given.
  while (true) {
    try {
      if (!noPrompt) {
	cout << "Command: ";
      }
      char* cstr = cstra;
      if (! cin.getline (cstr, buffersize)) {
	cerr << "Error while reading command" << endl;
	break;
      } 
      if (cstr[0] == 0) {
	break;
      }
      if (cstr[0] == '?') {
	showHelp();
      } else {
	PTCommand cmd = getCommand (cstr);
	ASSERTSTR(cmd!=NOCMD, "invalid command given");
	if (cmd == QUIT) {
	  break;
	}
	string parmName;
	if (cmd == OPEN) {
	  ASSERTSTR(parmtab==0, "OPEN or CREATE already done");
	  // Connect to database.
	  KeyValueMap kvmap = KeyParser::parse (cstr);
	  string dbUser = kvmap.getString ("user", getUserName());
	  string dbHost = kvmap.getString ("host", "dop50.astron.nl");
	  string dbName = kvmap.getString ("db", dbUser);
	  string dbType = kvmap.getString ("dbtype", "aips");
	  string tableName = kvmap.getString ("tablename", "MeqParm");
	  ParmDBMeta meta (dbType, tableName);
	  meta.setSQLMeta (dbName, dbUser, "", dbHost);
	  parmtab = new LOFAR::ParmDB::ParmDB (meta);
	} else if (cmd == CREATE)  {
	  ASSERTSTR(parmtab==0, "OPEN or CREATE already done");
	  // create dataBase	
	  KeyValueMap kvmap = KeyParser::parse (cstr);
	  string dbUser = kvmap.getString ("user", getUserName());
	  string dbHost = kvmap.getString ("host", "dop50.astron.nl");
	  string dbName = kvmap.getString ("db", dbUser);
	  string dbType = kvmap.getString ("dbtype", "aips");
	  string tableName = kvmap.getString ("tablename", "MeqParm");
	  ParmDBMeta meta (dbType, tableName);
	  meta.setSQLMeta (dbName, dbUser, "", dbHost);
	  parmtab = new LOFAR::ParmDB::ParmDB (meta, true);
	} else {
	  ASSERTSTR(parmtab!=0, "OPEN or CREATE not done yet");
	  if (cmd == CLOSE)  {
	    delete parmtab;
	    parmtab = 0;
	  } else if (cmd == CLEAR)  {
	    // clear database tables
	    parmtab->clearTables();
	  } else {
	    // Other commands expect a possible parmname and keywords
	    parmName = getParmName (cstr);
	    KeyValueMap kvmap = KeyParser::parse (cstr);
	    // For list functions the parmname defaults to *.
	    // Otherwise a parmname or pattern must be given.
	    if (cmd!=SHOW && cmd!=SHOWDEF && cmd!=SHOWOLD &&
		cmd!=NAMES && cmd!=NAMESDEF && cmd!=NAMESOLD) {
	      ASSERTSTR (!parmName.empty(), "No parameter name given");
	    } else if (parmName.empty()) {
	      parmName = "*";
	    }
	    if (cmd==NEWDEF || cmd==UPDDEF || cmd==DELDEF || cmd==SHOWDEF) {
	      // Read the given def parameters and domains.
	      map<string,ParmValueSet> parmset;
	      parmtab->getDefValues (parmset, parmName);
	      int nrparm = parmset.size();
	      if (cmd == NEWDEF) {
		ASSERTSTR (parmset.empty(),
			   "the parameter already exists");
		newDefParm (parmName, kvmap);
	      } else if (cmd == SHOWDEF) {
		showParms (parmset, false);
	      } else if (cmd == UPDDEF) {
		ASSERTSTR (! parmset.empty(), "parameter not found");
		updateDefParms (parmset, kvmap);
		cout << "Updated " << nrparm << " default parm value records"
		     << endl;
	      } else if (cmd == DELDEF) {
		ASSERTSTR (! parmset.empty(), "parameter not found");
		parmtab->deleteDefValues (parmName);
		cout << "Deleted " << nrparm << " default parm value records"
		     << endl;
	      }
	    } else if (cmd==NEW || cmd==UPD || cmd==DEL || cmd==SHOW ||
		       cmd==TOOLD) {
	      // Read the given parameters and domains.
	      ParmDomain domain = getDomain(kvmap);
	      int parentId = kvmap.getInt ("parentid", -1);
	      map<string,ParmValueSet> parmset;
	      parmtab->getValues (parmset, parmName, domain, parentId);
	      int nrparm = parmset.size();
	      int nrvalrec = countParms (parmset);
	      if (cmd == NEW) {
		ASSERTSTR (parmset.empty(),
			   "the parameter/domain already exists");
		newParm (parmName, kvmap);
	      } else if (cmd == SHOW) {
		showParms (parmset, true);
	      } else if (cmd == UPD) {
		ASSERTSTR (! parmset.empty(), "parameter/domain not found");
		updateParms (parmset, kvmap, ParmDBRep::UseNormal);
		cout << "Updated " << nrvalrec << " value records (of "
		     << nrparm << " parms)" << endl;
	      } else if (cmd == DEL) {
		ASSERTSTR (! parmset.empty(), "parameter/domain not found");
		parmtab->deleteValues (parmName, domain, parentId);
		cout << "Deleted " << nrvalrec << " value records (of "
		     << nrparm << " parms)" << endl;
	      } else if (cmd == TOOLD) {
		ASSERTSTR (! parmset.empty(), "parameter/domain not found");
		moveParms (parmName, domain, parentId, parmset, kvmap);
		cout << "Moved " << nrvalrec << " value records (of "
		     << nrparm << " parms)" << endl;
	      }
	    } else if (cmd==UPDOLD || cmd==DELOLD || cmd==SHOWOLD) {
	      // Read the given parameters and domains.
	      ParmDomain domain = getDomain(kvmap);
	      int parentId = kvmap.getInt ("parentid", -1);
	      map<string,ParmValueSet> parmset;
	      parmtab->getValues (parmset, parmName, domain, parentId,
				  ParmDBRep::UseHistory);
	      int nrparm = parmset.size();
	      int nrvalrec = countParms (parmset);
	      if (cmd == SHOWOLD) {
		showParms (parmset, true);
	      } else if (cmd == UPDOLD) {
		ASSERTSTR (! parmset.empty(), "parameter/domain not found");
		updateParms (parmset, kvmap, ParmDBRep::UseHistory);
		cout << "Updated " << nrvalrec << " old value records (of "
		     << nrparm << " parms)" << endl;
	      } else if (cmd == DELOLD) {
		ASSERTSTR (! parmset.empty(), "parameter/domain not found");
		parmtab->deleteValues (parmName, domain, parentId);
		cout << "Deleted " << nrvalrec << " old value records (of "
		     << nrparm << " parms)" << endl;
	      }
	    } else if (cmd==NAMES || cmd==NAMESOLD || cmd==NAMESDEF)  {
		// show names matching the pattern.
		showNames (parmName, cmd);
	    } else {
	      cerr << "Unknown command given" << endl;
	    }
	  }
	}
      }
    } catch (std::exception& x) {
      cerr << "Exception: " << x.what() << endl;
    }
  }
  delete parmtab;
  parmtab = 0;
}

int main (int argc)
{
  try {
    doIt (argc > 1);
  } catch (std::exception& x) {
    std::cerr << "Caught exception: " << x.what() << std::endl;
    return 1;
  }
  
  return 0;
}
