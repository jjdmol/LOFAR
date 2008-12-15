//# parmdb_main.cc: put values in the ParmDB database
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
#include <ParmDB/ParmMap.h>
#include <ParmDB/ParmSet.h>
#include <ParmDB/ParmCache.h>
#include <ParmDB/ParmValue.h>

#include <Blob/KeyValueMap.h>
#include <Blob/KeyParser.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>

#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/MUString.h>
#include <casa/Containers/Block.h>
#include <casa/Exceptions/Error.h>
#include <iostream>
#include <string>
#include <pwd.h>
#include <unistd.h>
#include <libgen.h>

using namespace std;
using namespace casa;
using namespace LOFAR;
using namespace BBS;

ParmDB* parmtab;

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
  DEL,
  SHOWDEF,
  NAMESDEF,
  NEWDEF,
  UPDDEF,
  DELDEF,
  QUIT
};

char bool2char (bool val)
{
  return (val  ?  'y' : 'n');
}

string int2type (int type)
{
  switch (type) {
  case ParmValue::Scalar:
    return "scalar";
  case ParmValue::Polc:
    return "polc";
  case ParmValue::PolcLog:
    return "polclog";
  default:
    return "unknown type";
  }
}

int getType (const string& str)
{
  String strc(str);
  strc.downcase();
  if (strc == "scalar") {
    return ParmValue::Scalar;
  } else if (strc == "polc") {
    return ParmValue::Polc;
  } else if (strc == "polclog") {
    return ParmValue::PolcLog;
  } else if (strc.empty()) {
    return -1;
  }
  throw AipsError (strc + " is an unknown funklet type");
}

void showValues (std::ostream& os, const Array<double>& values,
                 const Array<bool>& mask)
{
  int n = values.size();
  if (n > 0) {
    const double* v = values.data();
    os << v[0];
    for (int i=1; i<n; i++) {
      os << ',' << v[i];
    }
  }
  os << "  shape=" << values.shape();
  if (mask.size() > 0) {
    const bool* v = mask.data();
    os << "  mask=" << v[0];
    for (int i=1; i<n; i++) {
      os << ',' << v[i];
    }
  }
}

void showHelp()
{
  cerr << endl;
  cerr << "Show and update contents of parameter tables containing the" << endl;
  cerr << "ME parameters and their defaults." << endl;
  cerr << " create db='username' dbtype='casa' tablename='meqparm'" << endl;
  cerr << " open   db='username' dbtype='casa' tablename='meqparm'" << endl;
  cerr << " quit  (or exit or stop)" << endl;
  cerr << endl;
  cerr << " showdef [parmname_pattern]" << endl;
  cerr << " namesdef [parmname_pattern]" << endl;
  cerr << " adddef parmname valuespec" << endl;
  cerr << " updatedef parmname_pattern valuespec" << endl;
  cerr << " removedef parmname_pattern" << endl;
  cerr << endl;
  cerr << " range [parmname_pattern]       (show the total domain range)" << endl;
  cerr << " show [parmname_pattern] [domain=...]" << endl;
  cerr << " names [parmname_pattern]" << endl;
  cerr << " add parmname domain= valuespec" << endl;
  cerr << " remove parmname_pattern [domain=]" << endl;
  cerr << endl;
  cerr << "  domain gives an N-dim domain (usually n is 2) as:" << endl;
  cerr << "      domain=[stx,endx,sty,endy,...]" << endl;
  cerr << "   or domain=[st=[stx,sty,...],end=[endx,...] or size=[sizex,...]]" << endl;
  cerr << "  valuespec gives the values of the parameter attributes as" << endl;
  cerr << "   key=value pairs separated by commas." << endl;
  cerr << "  Attributes not given are not changed. Values shown are defaults when adding." << endl;
  cerr << "   values=1              (coefficients)" << endl;
  cerr << "    if multiple coefficients, specify as vector and specify shape" << endl;
  cerr << "    For example:   values=[1,2,3,4], shape=[1,1,2,2]" << endl;
  cerr << "   mask=                 (mask telling which coefficients are solvable" << endl;
  cerr << "    default is that c[i,j] with i+j>max(shape) are not solvable" << endl;
  cerr << "    For example:   values=[0,0,3], mask=[F,F,T], nx=3" << endl;
  cerr << "   pert=1e-6             (perturbation for numerical differentation)" << endl;
  cerr << "   pertrel=T             (perturbation is relative? Use F for angles)" << endl;
  cerr << "   type='polc'           (funklet type; default is polynomial)" << endl;
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
  if (sc == "range") {
    cmd = RANGE;
  } else if (sc == "show"  ||  sc == "list") {
    cmd = SHOW;
  } else if (sc == "names") {
    cmd = NAMES;
  } else if (sc == "new"  ||  sc == "insert"  ||  sc == "add") {
    cmd = NEW;
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
#ifndef USE_NOSOCKETS
  passwd* aPwd;
  if ((aPwd = getpwuid(getuid())) != 0) {
    return aPwd->pw_name;
  }
#endif
  return "test";
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

// Check the shape and return the size.
int getSize (const IPosition& shape)
{
  int nr = 1;
  for (uint i=0; i<shape.size(); ++i) {
    ASSERTSTR (shape[i] > 0, "a shape value must be > 0");
    nr *= shape[i];
  }
  return nr;
}

// Get the shape and return the size.
int getShape (const KeyValueMap& kvmap, IPosition& shape, int defsize=1)
{
  KeyValueMap::const_iterator value = kvmap.find("shape");
  if (value != kvmap.end()) {
    vector<int> vec = value->second.getVecInt();
    shape.resize (vec.size());
    std::copy (vec.begin(), vec.end(), shape.begin());
  } else {
    shape.resize (2);
    shape[0] = kvmap.getInt ("nx", defsize);
    shape[1] = kvmap.getInt ("ny", defsize);
  }
  return getSize(shape);
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
  vector<bool> vec;
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

Box getDomain (const KeyValueMap& kvmap, int size=2)
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
      // Given as a vector of values (as stx,endx,sty,endy,...).
      vector<double> vec;
      ok = true;
      const vector<KeyValue>& vals = value->second.getVector();
      for (vector<KeyValue>::const_iterator iter = vals.begin();
           iter != vals.end();
           iter++) {
        if (iter->dataType() == KeyValue::DTString) {
          MUString str (iter->getString());
          Quantity res;
          if (MVTime::read (res, str)) {
            vec.push_back (res.getValue());
          } else {
            cout << "Error in interpreting " << iter->getString() << endl;
            ok = false;
            break;
          }
        } else {
          vec.push_back (iter->getDouble());
        }
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
      end.push_back (0);
    }
  }
  return Box(make_pair(st[0], end[0]), make_pair(st[1], end[1]));
}

void showDomain (const Box& domain)
{
  cout << " freq=[" << domain.lowerX()/1e6 << " MHz "
       << '+'<< domain.widthX()/1e3 << " KHz]";
  cout << " time=[" << MVTime::Format(MVTime::YMD, 6)
       << MVTime(Quantity(domain.lowerY(), "s")) << " "
       << '+' << domain.widthY() << " sec]";
}

// IMPLEMENTATION OF THE COMMANDS
void showNames (const string& pattern, PTCommand type)
{
  vector<string> names;
  if (type == NAMES) {
    names = parmtab->getNames (pattern);
  } else {
    ParmMap parmset;
    parmtab->getDefValues (parmset, pattern);
    for (ParmMap::const_iterator iter = parmset.begin();
         iter != parmset.end();
         iter++) {
      names.push_back (iter->first);
    }
  }
  cout << "names: " << names << endl;
}

void showParm (const string& parmName, const ParmValue& parm, const Box& domain,
               const ParmValueSet& pvset, bool showAll)
{
  cout << parmName;
  cout << "  type=" << int2type(pvset.getType());
  cout << endl;
  if (showAll) {
    cout << "    domain: ";
    showDomain (domain);
    cout << endl;
  }
  cout << "    values:  ";
  showValues (cout, parm.getValues(), pvset.getSolvableMask());
  cout << endl;
  cout << "    pert=" << pvset.getPerturbation()
       << " pert_rel=" << bool2char(pvset.getPertRel()) << endl;
}

void showParms (ParmMap& parmSet, bool showAll)
{
  int nr=0;
  for (ParmMap::iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    const string& parmName = iter->first;
    if (iter->second.size() == 0) {
      showParm (parmName,
                iter->second.getFirstParmValue(),
                iter->second.getGrid().getBoundingBox(),
                iter->second, showAll);
      nr++;
    } else {
      for (uint i=0; i<iter->second.size(); ++i) {
        showParm (parmName,
                  iter->second.getParmValue(i),
                  iter->second.getGrid().getCell(i),
                  iter->second, showAll);
        nr++;
      }
    }
  }
  if (nr != 1) {
    cout << parmSet.size() << " parms and " << nr << " values found" << endl;
  }
}

int countParms (const ParmMap& parmSet)
{
  int nr=0;
  for (ParmMap::const_iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    nr += iter->second.size();
  }
  return nr;
}

void newParm (const std::string& parmName, const KeyValueMap& kvmap)
{
  ParmSet parmset;
  ParmId parmid = parmset.addParm (*parmtab, parmName);
  // Get domain and values (if existing).
  Box domain = getDomain (kvmap, 2);
  ParmCache cache (parmset, domain);
  ParmValueSet& pvset = cache.getValueSet(parmid);
  // Assure no value exists yet.
  ASSERTSTR (pvset.size() == 0,
             "Value for this parameter/domain already exists");
  // Check if the parm already exists (for another domain).
  // If so, only the values can be set (but not the coeff shape).
  bool isOldParm = cache.getParmSet().isInParmDB(parmid);
  // Get values using current default values.
  ParmValue defval(pvset.getFirstParmValue());
  int type = pvset.getType();
  double pert = pvset.getPerturbation();
  bool pertrel = pvset.getPertRel();
  Array<Bool> mask = pvset.getSolvableMask();
  IPosition shape = defval.getValues().shape();
  int size = shape.product();
  // Get the new shape and the data.
  IPosition shp;
  int nsize = getShape (kvmap, shp);
  // Get possible new values.
  if (!isOldParm) {
    if (kvmap.isDefined("type")) {
      type = getType (kvmap.getString("type", ""));
    }
    if (nsize > 0) {
      shape = shp;
      size = nsize;
    }
    if (kvmap.isDefined("mask")) {
      Block<Bool> bmask = getMask (kvmap, "mask", size);
      mask.assign (Array<bool>(shape, bmask.storage(), SHARE));
    }
  } else {
    if (nsize > 0  &&  type != ParmValue::Scalar) {
      ASSERTSTR (shp.isEqual(shape),
                 "Parameter has more domains; coeff shape cannot be changed");
    }
    shape = shp;
    size = nsize;
  }
  // Get the values.
  vector<double> values = getArray (kvmap, "values", size);
  Array<double> vals(shape, &(values[0]), SHARE);
  // Create the parm value.
  ParmValue::ShPtr pval(new ParmValue);
  if (pvset.getType() != ParmValue::Scalar) {
    pval->setCoeff (vals);
  } else {
      RegularAxis xaxis(domain.lowerX(), domain.upperX(), shape[0], true);
      RegularAxis yaxis(domain.lowerY(), domain.upperY(), shape[1], true);
      pval->setScalars (Grid(Axis::ShPtr(new RegularAxis(xaxis)),
                             Axis::ShPtr(new RegularAxis(yaxis))),
                        vals);
  }
  vector<ParmValue::ShPtr> valvec (1, pval);
  vector<Box> domains(1, domain);
  pvset = ParmValueSet (Grid(domains), valvec, defval,
                        ParmValue::FunkletType(type), pert, pertrel);
  showParm (parmName, *pval, domain, pvset, true);
  pvset.setDirty();
  cache.flush();
  cout << "Wrote new record for parameter " << parmName << endl;
}

void newDefParm (const std::string& parmName, KeyValueMap& kvmap)
{
  // Get possible funklet type.
  int type = getType (kvmap.getString("type", ""));
  // Get the shape and the data.
  IPosition shape;
  int size = getShape (kvmap, shape);
  // Get values and possible mask.
  vector<double> values = getArray (kvmap, "values", size);
  Block<bool> mask;
  if (kvmap.isDefined("mask")) {
    mask = getMask (kvmap, "mask", size);
  }
  ParmValue pval(values[0]);
  if (type > 0  ||  values.size() > 1) {
    pval.setCoeff (Array<double>(shape, &(values[0]), SHARE));
    if (type<0) type = ParmValue::Polc;
  } else {
    type = ParmValue::Scalar;
  }
  // Get perturbation for numerical derivatives.
  double pert = kvmap.getDouble ("pert", 1e-6);
  bool pertrel = kvmap.getBool ("pert_rel", true);
  ParmValueSet pvset(pval, ParmValue::FunkletType(type), pert, pertrel);
  if (mask.size() > 0) {
    pvset.setSolvableMask (Array<Bool>(shape, mask.storage(), SHARE));
  }
  showParm (parmName, pval, Box(), pvset, false);
  parmtab->putDefValue (parmName, pvset);
  cout << "Wrote new defaultvalue record for parameter " << parmName << endl;
}

void updateDefParm (const string& parmName, const ParmValueSet& pvset,
                    KeyValueMap& kvmap)
{
  ParmValue pval = pvset.getFirstParmValue();
  int type = pvset.getType();
  // Set funklet type with possible constants.
  if (kvmap.isDefined("type")) {
    type = getType (kvmap.getString("type", ""));
  }
  Array<double> values = pval.getValues().copy();
  if (kvmap.isDefined("values")) {
    IPosition shape;
    // Get the coefficients shape and the data.
    int size = getShape (kvmap, shape);
    vector<double> coeff = getArray (kvmap, "values", size);
    values.assign (Array<double> (shape, &(coeff[0]), SHARE));
  }
  Array<bool> mask = pvset.getSolvableMask().copy();
  if (kvmap.isDefined("mask")) {
    Block<bool> bmask = getMask (kvmap, "mask", values.size());
    mask.assign (Array<bool> (values.shape(), bmask.storage(), SHARE));
  }
  // Set perturbation for numerical derivatives.
  double pert = kvmap.getDouble ("pert", pvset.getPerturbation());
  bool pertrel = kvmap.getBool ("pertrel", pvset.getPertRel());
  ParmValue newval(values.data()[0]);
  if (type > 0  ||  values.size() > 1) {
    newval.setCoeff (values);
    if (type<0) type = ParmValue::Polc;
  } else {
    type = ParmValue::Scalar;
  }
  ParmValueSet newset(newval, ParmValue::FunkletType(type), pert, pertrel);
  newset.setSolvableMask (mask);
  showParm (parmName, newval, Box(), newset, false);
  parmtab->putDefValue (parmName, newset);
}

void updateDefParms (ParmMap& parmSet, KeyValueMap& kvmap)
{
  for (ParmMap::iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    updateDefParm (iter->first, iter->second, kvmap);
  }
}


void doIt (bool noPrompt)
{
  parmtab = 0;
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
        string parmName;
        if (cmd == OPEN) {
          ASSERTSTR(parmtab==0, "OPEN or CREATE already done");
          // Connect to database.
          KeyValueMap kvmap = KeyParser::parse (cstr);
          string dbUser = kvmap.getString ("user", getUserName());
          string dbHost = kvmap.getString ("host", "dop50.astron.nl");
          string dbName = kvmap.getString ("db", dbUser);
          string dbType = kvmap.getString ("dbtype", "casa");
          string tableName = kvmap.getString ("tablename", "MeqParm");
          ParmDBMeta meta (dbType, tableName);
          meta.setSQLMeta (dbName, dbUser, "", dbHost);
          parmtab = new ParmDB (meta);
        } else if (cmd == CREATE)  {
          ASSERTSTR(parmtab==0, "OPEN or CREATE already done");
          // create dataBase
          KeyValueMap kvmap = KeyParser::parse (cstr);
          string dbUser = kvmap.getString ("user", getUserName());
          string dbHost = kvmap.getString ("host", "dop50.astron.nl");
          string dbName = kvmap.getString ("db", dbUser);
          string dbType = kvmap.getString ("dbtype", "casa");
          string tableName = kvmap.getString ("tablename", "MeqParm");
          ParmDBMeta meta (dbType, tableName);
          meta.setSQLMeta (dbName, dbUser, "", dbHost);
          parmtab = new ParmDB (meta, true);
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
            if (cmd!=RANGE && cmd!=SHOW && cmd!=SHOWDEF &&
                cmd!=NAMES && cmd!=NAMESDEF) {
              ASSERTSTR (!parmName.empty(), "No parameter name given");
            } else if (parmName.empty()) {
              parmName = "*";
            }
            if (cmd==NEWDEF || cmd==UPDDEF || cmd==DELDEF || cmd==SHOWDEF) {
              // Read the given def parameters and domains.
              ParmMap parmset;
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
                cout << "Updated " << nrparm << " parm defaultvalue records"
                     << endl;
              } else if (cmd == DELDEF) {
                ASSERTSTR (! parmset.empty(), "parameter not found");
                parmtab->deleteDefValues (parmName);
                cout << "Deleted " << nrparm << " parm defaultvalue records"
                     << endl;
              }
            } else if (cmd==NEW || cmd==DEL || cmd==SHOW) {
              // Read the given parameters and domains.
              Box domain = getDomain(kvmap);
              ParmMap parmset;
              parmtab->getValues (parmset, parmName, domain);
              int nrparm = parmset.size();
              int nrvalrec = countParms (parmset);
              if (cmd == NEW) {
                ASSERTSTR (parmset.empty(),
                           "the parameter/domain already exists");
                newParm (parmName, kvmap);
              } else if (cmd == SHOW) {
                showParms (parmset, true);
              } else if (cmd == DEL) {
                ASSERTSTR (! parmset.empty(), "parameter/domain not found");
                parmtab->deleteValues (parmName, domain);
                cout << "Deleted " << nrvalrec << " value records (of "
                     << nrparm << " parms)" << endl;
              }
            } else if (cmd==NAMES || cmd==NAMESDEF)  {
              // show names matching the pattern.
              showNames (parmName, cmd);
            } else if (cmd == RANGE) {
              cout << "Range: ";
              showDomain (parmtab->getRange (parmName));
              cout << endl;
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
