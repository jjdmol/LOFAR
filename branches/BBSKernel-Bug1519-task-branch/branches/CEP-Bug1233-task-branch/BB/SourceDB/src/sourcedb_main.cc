//# sourcedb.cc: put values in the database used for MNS
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

#include <SourceDB/SourceDB.h>
#include <ParmDB/ParmDomain.h>
#include <Blob/KeyValueMap.h>
#include <Blob/KeyParser.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>

#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Utilities/MUString.h>
#include <casa/Containers/Block.h>
#include <iostream>
#include <string>
#include <pwd.h>
#include <unistd.h>
#include <libgen.h>

using namespace std;
using namespace casa;
using namespace LOFAR;
using namespace SourceDB;
using namespace ParmDB;

LOFAR::SourceDB::SourceDB* sourcetab;

enum PTCommand {
  NOCMD,
  OPEN,
  CREATE,
  CLOSE,
  CLEAR,
  RANGE,
  SHOW,
  NAMES,
  NEW,
  UPD,
  DEL,
  QUIT
};

char bool2char (bool val)
{
  return (val  ?  'y' : 'n');
}

void showValues (std::ostream& os, const vector<double>& values,
		 const vector<int>& shape)
{
  int n = values.size();
  if (n > 0) {
    os << values[0];
    for (int i=1; i<n; i++) {
      os << ',' << values[i];
    }
  }
  os << "  shape=" << shape;
}

void showHelp()
{
  cerr << endl;
  cerr << "Show and update contents of source tables containing the" << endl;
  cerr << "source parameters." << endl;
  cerr << " create db='username' dbtype='aips' tablename='sourcedb'" << endl;
  cerr << " open   db='username' dbtype='aips' tablename='sourcedb'" << endl;
  cerr << " quit  (or exit or stop)" << endl;
  cerr << endl;
  cerr << " show  [sourcename_pattern] [domain=spec,] [ra=, dec=, radius=1 (in arcsec)]" << endl;
  cerr << " names [sourcename_pattern]" << endl;
  cerr << " add sourcename domain=spec, valuespec" << endl;
  cerr << " update sourcename_pattern [domain=spec,] valuespec" << endl;
  cerr << " remove sourcename_pattern [domain=spec,]" << endl;
  cerr << endl;
  cerr << "  domain gives a 1-dim or 2-dim domain as:" << endl;
  cerr << "      domain=[stfreq,endfreq,sttime,endtime]" << endl;
  cerr << "   or domain=[st=[stf,stt],end=[endf,endt] or size=[sizef,sizet]]" << endl;
  cerr << "  valuespec gives the values of the source attributes as" << endl;
  cerr << "   key=value pairs separated by commas." << endl;
  cerr << "  Attributes not given are not changed. Values shown are defaults when adding." << endl;
  cerr << "   type='point'          (source type; default is point)" << endl;
  cerr << "   ra=12:2:34.1          (ra in J2000)" << endl;
  cerr << "   dec=12.2.34.1         (dec in J2000)" << endl;
  cerr << "   flux=[1,0,0,0]        (flux I,Q,U,V; default of each is 0)" << endl;
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
  } else if (sc == "open") {
    cmd = OPEN;
  } else if (sc == "close") {
    cmd = CLOSE;
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
  // On Cray XT3 getpwuid is unavailable.
#ifndef USE_NOSOCKETS
  passwd* aPwd;
  if ((aPwd = getpwuid(getuid())) != 0) {
    return aPwd->pw_name;
  }
#endif
  return "test";
}


std::string getSourceName (char*& str)
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
  // If an = was found, no source is given.
  if (foundeq) {
    str = sstr;
    return "";
  }
  return std::string(sstr, str-sstr);
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

bool getDomainVector (vector<double>& vec, const vector<KeyValue>& vals)
{
  vec.reserve (vals.size());
  for (vector<KeyValue>::const_iterator iter = vals.begin();
       iter != vals.end();
       iter++) {
    if (iter->dataType() == KeyValue::DTString) {
      MUString str (iter->getString());
      Quantity res;
      if (MVTime::read (res, str)) {
	vec.push_back (res.getValue("s"));
      } else {
	cout << "Error in interpreting " << iter->getString() << endl;
	return false;
      }
    } else {
      vec.push_back (iter->getDouble());
    }
  }
  return true;
}

ParmDomain getDomain (const KeyValueMap& kvmap, int size=0)
{
  KeyValueMap::const_iterator value = kvmap.find("domain");
  vector<double> st;
  vector<double> end;
  if (value != kvmap.end()) {
    bool ok = false;
    // See if given as domain=[start=[],end=[] or size=[]].
    if (value->second.dataType() == KeyValue::DTValueMap) {
      const KeyValueMap& kvm = value->second.getValueMap();
      KeyValueMap::const_iterator key = kvm.find("st");
      if (key != kvm.end()) {
	bool okv = getDomainVector (st, key->second.getVector());
	key = kvm.find("end");
	bool hasend = (key != kvm.end());
	if (hasend) okv = okv && getDomainVector (end, key->second.getVector());
	key = kvm.find("size");
	bool hassize = (key != kvm.end());
	if (hassize) end = key->second.getVecDouble();
	// end and size cannot be given both.
	if (hasend != hassize) {
	  if (st.size() == end.size()) {
	    ok = okv;
	    if (hassize) {
	      for (uint i=0; i<end.size(); ++i) {
		end[i] += st[i];
	      }
	    }
	  }
	}
      }
    } else {
      // Given as a vector of values (as stx,endx,sty,endy,...).
      vector<double> vec;
      ok = getDomainVector (vec, value->second.getVector());
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
	       "domain must be given as [[stf,endf],[stt,endt],...]\n"
	       "or as [st=[stf,...],end=[endf,...] or size=[sizef,...]]");
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
    cout << " freq=";
    if (st[0] == -1e30  &&  end[0] == 1e30) {
      cout << "all";
    } else {
      cout <<'[' << st[0]/1e6 << " MHz " << (end[0] - st[0] < 0 ? "-" : "+")
	   << (end[0]-st[0])/1e3 << " KHz]";
    }
    cout << " time=";
    if (st[1] == -1e30  &&  end[1] == 1e30) {
      cout << "all";
    } else {
      cout << '[' << MVTime::Format(MVTime::YMD, 6)
	   << MVTime(Quantity(st[1], "s")) << " "
	   << (end[1] - st[1] < 0 ? "-" : "+") << (end[1] - st[1]) << " sec]";
    }
  } else {
    for (uint i=0; i<st.size(); ++i) {
      if (i != 0) cout << ',';
      cout << '[' << st[i] << ',' << end[i] << ']';
    }
  }
}

// IMPLEMENTATION OF THE COMMANDS
void showSource (const SourceValue& source, bool showAll)
{
  cout << source.getName();
  cout << "  type=" << source.getType();
  cout << "  ra=" << MVAngle::Format(MVAngle::TIME, 9)
       << MVAngle(Quantity(source.getRA(), "rad"));
  cout << " dec=" << MVAngle::Format(MVAngle::ANGLE, 9)
       << MVAngle(Quantity(source.getDEC(), "rad"));
  cout << endl;
  if (showAll) {
    cout << "  spectralindex=" << source.getSpectralIndex();
    cout << "  domain:";
    showDomain (source.getDomain());
    cout << endl;
  }
}

void showSources (const list<SourceValue>& sourceSet, bool showAll)
{
  int nr=0;
  for (list<SourceValue>::const_iterator iter = sourceSet.begin();
       iter != sourceSet.end();
       iter++) {
    showSource (*iter, showAll);
    ++nr;
  }
  if (nr != 1) {
    cout << nr << " source records found" << endl;
  }
}

void newSource (const std::string& sourceName, const ParmDomain& domain,
		const KeyValueMap& kvmap)
{
  SourceValue pvalue;
  string type = kvmap.getString("type", "point");
  ASSERTSTR (type=="point", "Currently only point surces can be given");
  ASSERTSTR (kvmap.isDefined("ra"),  "Keyword 'ra' not given");
  ASSERTSTR (kvmap.isDefined("dec"), "Keyword 'dec' not given");
  double ra  = kvmap.getDouble("ra", 0);
  double dec = kvmap.getDouble("dec", 0);
  double spindex = kvmap.getDouble ("spindex", 1);
  vector<double> vflux = getArray (kvmap, "flux", 4);
  double flux[4] = {1,0,0,0};
  if (vflux.size() > 0) flux[0] = vflux[0];
  if (vflux.size() > 1) flux[1] = vflux[1];
  if (vflux.size() > 2) flux[2] = vflux[2];
  if (vflux.size() > 3) flux[3] = vflux[3];
  pvalue.setPointSource (sourceName, ra, dec, flux, spindex);
  pvalue.setDomain (domain);
  showSource (pvalue, true);
  sourcetab->addSource (pvalue);
  cout << "Wrote new record for source " << sourceName << endl;
}

void updateSource (SourceValue& pvalue, const KeyValueMap& kvmap)
{
  if (kvmap.isDefined("ra")) {
    pvalue.setRA (kvmap.getDouble("ra", 0));
  }
  if (kvmap.isDefined("dec")) {
    pvalue.setDEC (kvmap.getDouble("dec", 0));
  }
  if (kvmap.isDefined("spindex")) {
    pvalue.setSpectralIndex (kvmap.getDouble("spindex", 0));
  }
  if (kvmap.isDefined("flux")) {
    vector<double> vflux = getArray (kvmap, "flux", 4);
    if (vflux.size() > 0) pvalue.setFluxI (vflux[0]);
    if (vflux.size() > 1) pvalue.setFluxQ (vflux[1]);
    if (vflux.size() > 2) pvalue.setFluxU (vflux[2]);
    if (vflux.size() > 3) pvalue.setFluxV (vflux[3]);
  }
  showSource (pvalue, true);
}

void updateSources (list<SourceValue>& sourceSet, KeyValueMap& kvmap)
{
  for (list<SourceValue>::iterator iter = sourceSet.begin();
       iter != sourceSet.end();
       iter++) {
    updateSource (*iter, kvmap);
  }
  sourcetab->lock();
  for (list<SourceValue>::iterator iter = sourceSet.begin();
       iter != sourceSet.end();
       iter++) {
    sourcetab->updateSource (*iter);
  }
  sourcetab->unlock();
}


void doIt (bool noPrompt)
{
  sourcetab = 0;
  const int buffersize = 1024000;
  char cstra[buffersize];
  // Loop until stop is given.
  while (true) {
    try {
      if (!noPrompt) {
	cerr << "Command: ";
      }
      char* cstr = cstra;
      if (! cin.getline (cstr, buffersize)) {
	cerr << "Error while reading command" << endl;
	break;
      } 
      while (*cstr == ' ') {
	cstr++;
      }
      // Skip empty lines and comment lines.
      if (cstr[0] == 0  ||  cstr[0] == '#') {
	continue;
      }
      if (cstr[0] == '?') {
	showHelp();
      } else {
	PTCommand cmd = getCommand (cstr);
	ASSERTSTR(cmd!=NOCMD, "invalid command given: " << cstr);
	if (cmd == QUIT) {
	  break;
	}
	string sourceName;
	if (cmd == OPEN) {
	  ASSERTSTR(sourcetab==0, "OPEN or CREATE already done");
	  // Connect to database.
	  KeyValueMap kvmap = KeyParser::parse (cstr);
	  string dbUser = kvmap.getString ("user", getUserName());
	  string dbHost = kvmap.getString ("host", "dop50.astron.nl");
	  string dbName = kvmap.getString ("db", dbUser);
	  string dbType = kvmap.getString ("dbtype", "aips");
	  string tableName = kvmap.getString ("tablename", "MeqSource");
	  SourceDBMeta meta (dbType, tableName);
	  meta.setSQLMeta (dbName, dbUser, "", dbHost);
	  sourcetab = new LOFAR::SourceDB::SourceDB (meta);
	} else if (cmd == CREATE)  {
	  ASSERTSTR(sourcetab==0, "OPEN or CREATE already done");
	  // create dataBase
	  KeyValueMap kvmap = KeyParser::parse (cstr);
	  string dbUser = kvmap.getString ("user", getUserName());
	  string dbHost = kvmap.getString ("host", "dop50.astron.nl");
	  string dbName = kvmap.getString ("db", dbUser);
	  string dbType = kvmap.getString ("dbtype", "aips");
	  string tableName = kvmap.getString ("tablename", "MeqSource");
	  SourceDBMeta meta (dbType, tableName);
	  meta.setSQLMeta (dbName, dbUser, "", dbHost);
	  sourcetab = new LOFAR::SourceDB::SourceDB (meta, true);
	} else {
	  ASSERTSTR(sourcetab!=0, "OPEN or CREATE not done yet");
	  if (cmd == CLOSE)  {
	    delete sourcetab;
	    sourcetab = 0;
	  } else if (cmd == CLEAR)  {
	    // clear database tables
	    sourcetab->clearTables();
	  } else {
	    // Other commands expect a possible sourcename and keywords
	    sourceName = getSourceName (cstr);
	    KeyValueMap kvmap = KeyParser::parse (cstr);
	    // For list functions the sourcename defaults to *.
	    // Otherwise a sourcename or pattern must be given.
	    if (cmd!=SHOW && cmd!=NAMES) {
	      ASSERTSTR (!sourceName.empty(), "No source name given");
	    } else if (sourceName.empty()) {
	      sourceName = "*";
	    }
	    if (cmd==NEW || cmd==UPD || cmd==DEL) {
	      // Read the given parameters and domains.
	      ParmDomain domain = getDomain(kvmap);
	      list<SourceValue> sourceset;
	      vector<string> names (sourcetab->getNames(sourceName));
	      if (!names.empty()) {
		sourceset = sourcetab->getSources (names, domain);
	      }
	      int nrsource = sourceset.size();
	      if (cmd == NEW) {
		ASSERTSTR (sourceset.empty(),
			   "the source/domain already exists");
		newSource (sourceName, domain, kvmap);
	      } else if (cmd == UPD) {
		ASSERTSTR (! sourceset.empty(), "source/domain not found");
		updateSources (sourceset, kvmap);
		cout << "Updated " << nrsource << " source record(s)" << endl;
	      } else if (cmd == DEL) {
		ASSERTSTR (! sourceset.empty(), "parameter/domain not found");
		sourcetab->deleteSources (sourceName, domain);
		cout << "Deleted " << nrsource << " source record(s)" << endl;
	      }
	    } else if (cmd == SHOW) {
	      ParmDomain domain = getDomain(kvmap);
	      list<SourceValue> sourceset;
	      if (kvmap.isDefined("ra") && kvmap.isDefined("dec")) {
		double ra     = kvmap.getDouble("ra", 0);
		double dec    = kvmap.getDouble("dec", 0);
		double radius = kvmap.getDouble("radius", 1);
		radius = Quantity(radius, "arcsec").getValue("rad");
		sourceset = sourcetab->getSources (ra, dec, radius, domain);
	      } else {
		// Read the given parameters and domains.
		vector<string> names (sourcetab->getNames(sourceName));
		if (!names.empty()) {
		  sourceset = sourcetab->getSources (names, domain);
		}
	      }
	      showSources (sourceset, true);
	    } else if (cmd==NAMES)  {
	      // show names matching the pattern.
	      cout << "names: "
		   << sourcetab->getNames(sourceName) << endl;
	    } else {
	      cerr << "Unknown command given (type ? for help)" << endl;
	    }
	  }
	}
      }
    } catch (std::exception& x) {
      cerr << "Exception: " << x.what() << endl;
    }
  }
  delete sourcetab;
  sourcetab = 0;
}

int main (int argc, char *argv[])
{
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);
  
  try {
    doIt (argc > 1);
  } catch (std::exception& x) {
    std::cerr << "Caught exception: " << x.what() << std::endl;
    return 1;
  }
  
  return 0;
}
