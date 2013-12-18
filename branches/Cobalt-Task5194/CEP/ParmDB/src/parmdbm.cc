//# parmdbm.cc: put values in the ParmDB database
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$


#include <lofar_config.h>

#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmMap.h>
#include <ParmDB/ParmSet.h>
#include <ParmDB/ParmCache.h>
#include <ParmDB/ParmValue.h>
#include <ParmDB/Package__Version.h>

#include <Blob/KeyValueMap.h>
#include <Blob/KeyParser.h>
#include <Common/StreamUtil.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <Common/ReadLine.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>

#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/MUString.h>
#include <casa/Containers/Block.h>
#include <casa/Exceptions/Error.h>
#include <Common/lofar_string.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_fstream.h>
#include <pwd.h>

using namespace casa;
using namespace LOFAR;
using namespace BBS;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

ParmDB* parmtab;

enum PTCommand {
  NOCMD,
  OPEN,
  CREATE,
  CLOSE,
  CLEAR,
  SET,
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
  EXPORT,
  CHECKSHAPE,
  HELP,
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

void showValues (ostream& ostr, const Array<double>& values,
                 const Array<double>& errors, const Array<bool>& mask)
{
  int n = values.size();
  if (n > 0) {
    const double* v = values.data();
    ostr << v[0];
    for (int i=1; i<n; i++) {
      ostr << ',' << v[i];
    }
  }
  ostr << "  shape=" << values.shape();
  if (errors.size() > 0) {
    const double* v = errors.data();
    ostr << "  errors=" << v[0];
    for (int i=1; i<n; i++) {
      ostr << ',' << v[i];
    }
  }
  if (mask.size() > 0) {
    const bool* v = mask.data();
    ostr << "  mask=" << v[0];
    for (int i=1; i<n; i++) {
      ostr << ',' << v[i];
    }
  }
}

void showHelp()
{
  cerr << endl;
  cerr << "Show and update contents of parameter tables containing the" << endl;
  cerr << "BBS parameters and their defaults." << endl;
  cerr << "Frequency is the x-axis and time is the y-axis." << endl;
  cerr << endl;
  cerr << " create db='username' dbtype='casa' table[name]=" << endl;
  cerr << " open   db='username' dbtype='casa' table[name]=" << endl;
  cerr << " set    stepx=defaultstepsize, stepy=defaultstepsize" << endl;
  cerr << " quit  (or exit or stop)" << endl;
  cerr << endl;
  cerr << " showdef  [parmname_pattern]" << endl;
  cerr << " namesdef [parmname_pattern]" << endl;
  cerr << " adddef    parmname         valuespec" << endl;
  cerr << " updatedef parmname_pattern valuespec" << endl;
  cerr << " removedef parmname_pattern" << endl;
  cerr << " export    parmname_pattern tablename= append=0 dbtype='casa'" << endl;
  cerr << endl;
  cerr << " range [parmname_pattern]       (show the total domain range)" << endl;
  cerr << " show  [parmname_pattern] [domain=...]" << endl;
  cerr << " names [parmname_pattern]" << endl;
  cerr << " add    parmname          domain=  valuespec" << endl;
  cerr << " remove parmname_pattern [domain=]" << endl;
  cerr << " checkshape [parmname_pattern]  (check consistency of parm shapes)" << endl;
  cerr << endl;
  cerr << "  domain gives an N-dim domain (usually N is 2) as:" << endl;
  cerr << "       domain=[stx,endx,sty,endy,...]" << endl;
  cerr << "    or domain=[st=[stx,sty,...],end=[endx,...] or size=[sizex,...]]" << endl;
  cerr << "  valuespec gives the values of the parameter attributes as" << endl;
  cerr << "    key=value pairs separated by commas." << endl;
  cerr << "  Attributes not given are not changed. Values shown are defaults when adding." << endl;
  cerr << "    values=1              (coefficients)" << endl;
  cerr << "      if multiple coefficients, specify as vector and specify shape" << endl;
  cerr << "      For example:   values=[1,2,3,4], shape=[2,2]" << endl;
  cerr << "    mask=                 (mask telling which coefficients are solvable" << endl;
  cerr << "      default is that c[i,j] with i+j>max(shape) are not solvable" << endl;
  cerr << "      For example:   values=[0,0,3], mask=[F,F,T], nx=3" << endl;
  cerr << "    errors=               (optional error for each coefficient)" << endl;
  cerr << "    pert=1e-6             (perturbation for numerical differentation)" << endl;
  cerr << "    pertrel=T             (perturbation is relative? Use F for angles)" << endl;
  cerr << "    type='polc'           (funklet type; default is polc (polynomial))" << endl;
  cerr << endl;
}

PTCommand getCommand (string& line)
{
  PTCommand cmd = NOCMD;
  uint st = lskipws(line, 0, line.size());
  uint sst = st;
  while (st < line.size()  &&  line[st] != ' ') {
    st++;
  }
  string sc = line.substr(sst, st-sst);
  line = line.substr(st);
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
  } else if (sc == "export") {
    cmd = EXPORT;
  } else if (sc == "open") {
    cmd = OPEN;
  } else if (sc == "close") {
    cmd = CLOSE;
  } else if (sc == "clear") {
    cmd = CLEAR;
  } else if (sc == "create") {
    cmd = CREATE;
  } else if (sc == "set") {
    cmd = SET;
  } else if (sc == "checkshape") {
    cmd = CHECKSHAPE;
  } else if (sc == "help") {
    cmd = HELP;
  } else if (sc == "stop"  ||  sc == "quit"  || sc == "exit") {
    cmd = QUIT;
  } 
  return cmd;
}

string getUserName()
{
#ifndef USE_NOSOCKETS
  passwd* aPwd;
  if ((aPwd = getpwuid(getuid())) != 0) {
    return aPwd->pw_name;
  }
#endif
  return "test";
}


string getParmName (string& line)
{
  uint st = lskipws (line, 0, line.size());
  if (st >= line.size()) {
    return string();
  }
  uint sst=st;
  bool foundeq = false;
  while (st < line.size()  &&  line[st] != ' ') {
    if (line[st] == '=') {
      foundeq = true;
      break;
    }
    st++;
  }
  // If an = was found, no parm is given.
  if (foundeq) {
    return string();
  }
  string name = line.substr(sst, st-sst);
  line = line.substr(st);
  return name;
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

vector<double> getArray (const KeyValueMap& kvmap, const string& arrName,
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
Block<bool> getMask (const KeyValueMap& kvmap, const string& arrName,
                     const IPosition& shape)
{
  uint size = shape.product();
  Block<bool> res(size, false);
  KeyValueMap::const_iterator value = kvmap.find(arrName);
  if (value == kvmap.end()) {
    // If not given, set higher coefficients to False.
    int ndim = std::max(shape[0], shape[1]);
    uint i=0;
    for (int iy=0; iy<shape[1]; ++iy) {
      for (int ix=0; ix<shape[0]; ++ix) {
        res[i] = (ix+iy < ndim);
        ++i;
      }
    }
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

Box getDomain (const KeyValueMap& kvmap, ostream& ostr,
               int size=2, double defEnd=0)
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
            vec.push_back (res.getValue("s"));
          } else {
            ostr << "Error in interpreting " << iter->getString() << endl;
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
      end.push_back (defEnd);
    }
  }
  return Box(make_pair(st[0], st[1]), make_pair(end[0], end[1]));
}

void showDomain (const Box& domain, ostream& ostr)
{
  ostr << " freq=[" << domain.lowerX()/1e6 << " MHz "
       << '+'<< domain.widthX()/1e3 << " KHz]";
  ostr << " time=[" << MVTime::Format(MVTime::YMD, 6)
       << MVTime(Quantity(domain.lowerY(), "s")) << " "
       << '+' << domain.widthY() << " sec]";
}

// IMPLEMENTATION OF THE COMMANDS
void showNames (const string& pattern, PTCommand type, ostream& ostr)
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
  ostr << "names: " << names << endl;
}

void showParm (const string& parmName, const ParmValue& parm, const Box& domain,
               const ParmValueSet& pvset, bool showAll, ostream& ostr)
{
  ostr << parmName;
  ostr << "  type=" << int2type(pvset.getType());
  ostr << endl;
  if (showAll) {
    ostr << "    domain: ";
    showDomain (domain, ostr);
    ostr << endl;
  } else if (! pvset.getScaleDomain().empty()) {
    ostr << "    scale domain: ";
    showDomain (pvset.getScaleDomain(), ostr);
    ostr << endl;
  }
  ostr << "    values:  ";
  Array<double> errors;
  if (parm.hasErrors()) {
    errors.reference (parm.getErrors());
  }
  showValues (ostr, parm.getValues(), errors, pvset.getSolvableMask());
  ostr << endl;
  ostr << "    pert=" << pvset.getPerturbation()
       << " pert_rel=" << bool2char(pvset.getPertRel()) << endl;
}

void showParms (ParmMap& parmSet, bool showAll, ostream& ostr)
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
                iter->second, showAll, ostr);
      nr++;
    } else {
      for (uint i=0; i<iter->second.size(); ++i) {
        showParm (parmName,
                  iter->second.getParmValue(i),
                  iter->second.getGrid().getCell(i),
                  iter->second, showAll, ostr);
        nr++;
      }
    }
  }
  if (nr != 1) {
    ostr << parmSet.size();
    if (!showAll) ostr << " default";
    ostr << " parms and " << nr << " values found" << endl;
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

void newParm (const string& parmName, const KeyValueMap& kvmap, ostream& ostr)
{
  ParmSet parmset;
  ParmId parmid = parmset.addParm (*parmtab, parmName);
  // Get domain and values (if existing).
  Box domain = getDomain (kvmap, ostr, 2, 1.);
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
    if (kvmap.isDefined("mask")  ||  size > 1) {
      Block<Bool> bmask = getMask (kvmap, "mask", shape);
      mask.assign (Array<bool>(shape, bmask.storage(), SHARE));
    }
  } else {
    /// Outcomment because old shape is always [1,1]
    /// The columns NX and NY are not filled by ParmDBCasa.
    ///    if (nsize > 0  &&  type != ParmValue::Scalar) {
    ///      ASSERTSTR (shp.isEqual(shape),
    ///                 "Parameter has more domains; new coeff shape " << shp
    ///                 << " mismatches " << shape);
    ///    }
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
  // Set the errors if given.
  if (kvmap.isDefined ("errors")) {
    vector<double> errors = getArray (kvmap, "errors", size);
    Array<double> errs(shape, &(errors[0]), SHARE);
    pval->setErrors (errs);
  }
  // Create the ParmValueSet.
  vector<ParmValue::ShPtr> valvec (1, pval);
  vector<Box> domains(1, domain);
  pvset = ParmValueSet (Grid(domains), valvec, defval,
                        ParmValue::FunkletType(type), pert, pertrel);
  showParm (parmName, *pval, domain, pvset, true, ostr);
  pvset.setDirty();
  cache.flush();
  ostr << "Wrote new record for parameter " << parmName << endl;
}

void newDefParm (const string& parmName, KeyValueMap& kvmap, ostream& ostr)
{
  // Get possible funklet type.
  int type = getType (kvmap.getString("type", ""));
  // Get the shape and the data.
  IPosition shape;
  int size = getShape (kvmap, shape);
  // Get values and possible mask.
  vector<double> values = getArray (kvmap, "values", size);
  Block<bool> mask;
  if (kvmap.isDefined("mask")  ||  size > 1) {
    mask = getMask (kvmap, "mask", shape);
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
  showParm (parmName, pval, pvset.getScaleDomain(), pvset, false, ostr);
  parmtab->putDefValue (parmName, pvset);
  ostr << "Wrote new defaultvalue record for parameter " << parmName << endl;
}

void updateDefParm (const string& parmName, const ParmValueSet& pvset,
                    KeyValueMap& kvmap, ostream& ostr)
{
  ParmValue pval = pvset.getFirstParmValue();
  int type = pvset.getType();
  // Set funklet type with possible constants.
  if (kvmap.isDefined("type")) {
    type = getType (kvmap.getString("type", ""));
  }
  Array<double> values = pval.getValues().copy();
  IPosition oldShape = values.shape();
  if (kvmap.isDefined("values")) {
    IPosition shape;
    // Get the coefficients shape and the data.
    int size = getShape (kvmap, shape);
    vector<double> coeff = getArray (kvmap, "values", size);
    values.assign (Array<double> (shape, &(coeff[0]), SHARE));
  }
  Array<bool> mask = pvset.getSolvableMask().copy();
  if (kvmap.isDefined("mask")  ||  !oldShape.isEqual(values.shape())) {
    Block<bool> bmask = getMask (kvmap, "mask", values.shape());
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
  showParm (parmName, newval, pvset.getScaleDomain(), newset, false, ostr);
  parmtab->putDefValue (parmName, newset);
}

void updateDefParms (ParmMap& parmSet, KeyValueMap& kvmap, ostream& ostr)
{
  for (ParmMap::iterator iter = parmSet.begin();
       iter != parmSet.end();
       iter++) {
    updateDefParm (iter->first, iter->second, kvmap, ostr);
  }
}

// Create a new parm for export.
int exportNewParm (const string& name, int fixedAxis,
                   const ParmValueSet& pset, ParmDB& newtab,ostream& ostr)
{
  Axis::ShPtr infAxis(new RegularAxis(-1e30, 1e30, 1, true));
  // Copy the old values.
  // Check that the fixed axis of all values matches the grid.
  // Clear the rowId because the new values will be in a new row.
  vector<ParmValue::ShPtr> values;
  vector<Box> boxes;
  values.reserve (pset.size());
  boxes.reserve (pset.size());
  const Axis& ax0 = *(pset.getParmValue(0).getGrid()[fixedAxis]);
  for (uint i=0; i<pset.size(); ++i) {
    ParmValue::ShPtr pval(new ParmValue(pset.getParmValue(i)));
    if (*(pval->getGrid()[fixedAxis]) != ax0) {
      return 0;
    }
    pval->clearRowId();
    values.push_back (pval);
    if (fixedAxis == 0) {
      boxes.push_back (Grid(infAxis, pval->getGrid()[1]).getBoundingBox());
    } else {
      boxes.push_back (Grid(pval->getGrid()[0], infAxis).getBoundingBox());
    }
  }
  // Create the set from the values.
  ParmValueSet newSet(Grid(boxes), values, pset.getDefParmValue(),
                      pset.getType(),
                      pset.getPerturbation(), pset.getPertRel());
  newSet.setSolvableMask (pset.getSolvableMask());
  // Write the value with the correct id.
  Int nameId = newtab.getNameId (name);
  newtab.putValues (name, nameId, newSet);
  ostr << "Exported record for parameter " << name << endl;
  return 1;
}

// Copy constant scalar values to default values in the new table.
// Also polynomial values can be exported as default values.
// Copy other values with a constant axis to the new table, but set the
// domain for that axis to infinite.
// In this way one can have a freq-dependent, time-constant calibration
// solution and export it with an infinite time interval.
int exportParms (const ParmMap& parmset, ParmDB& newtab, ostream& ostr)
{
  int ncopy = 0;
  for (ParmMap::const_iterator iter = parmset.begin();
       iter != parmset.end(); ++iter) {
    const string& name = iter->first;
    const ParmValueSet& pset = iter->second;
    if (pset.size() > 0) {
      if (pset.size() == 1) {
        const ParmValue& pval = pset.getParmValue(0);
        if (pval.nx() == 1  &&  pval.ny() == 1) {
          newtab.putDefValue (name, pset);
          ostr << "Exported default scalar record for parameter "
               << name << endl;
          ncopy++;
        } else if (pset.getType() == ParmValue::Polc) {
          ParmValueSet pset1 (pval, ParmValue::Polc,
                              pset.getPerturbation(), pset.getPertRel(),
                              pset.getGrid().getBoundingBox());
          newtab.putDefValue (name, pset1);
          ostr << "Exported default polc record for parameter "
               << name << endl;
          ncopy++;
        } else if (pval.nx() == 1) {
          // Constant in x.
          ncopy += exportNewParm (name, 0, pset, newtab, ostr);
        } else if (pval.ny() == 1) {
          // Constant in y.
          ncopy += exportNewParm (name, 1, pset, newtab, ostr);
        }
      } else {
        // We have a set of ParmValues for which one axis must be fixed,
        // thus have size 1 and equal for all values.
        // Note they can all have shape [1,1], so find the variable axis.
        int fixedAxis = 0;
        const ParmValue* pval0 = &(pset.getParmValue(0));
        const ParmValue* pval1 = &(pset.getParmValue(1));
        if (pval0->nx() == 1  &&  pval0->ny() == 1) {
          if (pval1->nx() == 1  &&  pval1->ny() == 1) {
            if (*(pval0->getGrid()[1]) == *(pval1->getGrid()[1])) {
              fixedAxis = 1;
              pval0 = 0;
            }
          } else {
            // Use pval1 to find the axis with size > 1.
            pval0 = pval1;
          }
        }
        if (pval0) {
          if (pval0->nx() == 1) {
            fixedAxis = 1;
          }
        }
        ncopy += exportNewParm (name, fixedAxis, pset, newtab, ostr);
      }
    }
  }
  return ncopy;
}

void checkShape (const ParmMap& parmset, ostream& ostr)
{
  vector<string> errNames;
  for (ParmMap::const_iterator iter = parmset.begin();
       iter != parmset.end(); ++iter) {
    const string& name = iter->first;
    const ParmValueSet& pset = iter->second;
    // Only check if multiple polcs.
    if (pset.size() > 1  &&  pset.getType() != ParmValue::Scalar) {
      uint nx = pset.getParmValue(0).nx();
      uint ny = pset.getParmValue(0).ny();
      for (uint i=1; i<pset.size(); ++i) {
        if (pset.getParmValue(i).nx() != nx  ||
            pset.getParmValue(i).ny() != ny) {
          errNames.push_back (name);
          break;
        }
      }
    }
  }
  if (errNames.empty()) {
    ostr << "All parameters have consistent value shapes" << endl;
  } else {
    ostr << errNames.size() << " parameter";
    if (errNames.size() == 1) {
      ostr << " has";
    } else {
      ostr << "s have";
    }
    ostr << " non-scalar values with inconsistent shape:" << endl;
    writeVector (ostr, errNames, ", ", "    ", "");
    ostr << endl;
  }
}

void doIt (bool noPrompt, ostream& ostr)
{
  parmtab = 0;
  string line;
  // Loop until stop is given.
  while (true) {
    try {
      if (!noPrompt) {
        cerr << "Command: ";
      }
      if (!readLineSkip (line, "Command: ", "#")) {
        break;
      }
      if (line[0] == '?') {
        showHelp();
      } else {
        PTCommand cmd = getCommand (line);
        ASSERTSTR(cmd!=NOCMD, "invalid command given: " << line);
        if (cmd == QUIT) {
          break;
        }
        string parmName;
        if (cmd == HELP) {
          showHelp();
        } else if (cmd == OPEN) {
          ASSERTSTR(parmtab==0, "OPEN or CREATE already done");
          // Connect to database.
          KeyValueMap kvmap = KeyParser::parse (line);
          string dbUser = kvmap.getString ("user", getUserName());
          string dbHost = kvmap.getString ("host", "dop50.astron.nl");
          string dbName = kvmap.getString ("db", dbUser);
          string dbType = kvmap.getString ("dbtype", "casa");
          string tableName = kvmap.getString ("table", "");
          if (tableName.empty()) {
            tableName = kvmap.getString ("tablename", "");
          }
          ASSERTSTR(!tableName.empty(), "No table name given");
          ParmDBMeta meta (dbType, tableName);
          meta.setSQLMeta (dbName, dbUser, "", dbHost);
          parmtab = new ParmDB (meta);
        } else if (cmd == CREATE)  {
          ASSERTSTR(parmtab==0, "OPEN or CREATE already done");
          // create dataBase
          KeyValueMap kvmap = KeyParser::parse (line);
          string dbUser = kvmap.getString ("user", getUserName());
          string dbHost = kvmap.getString ("host", "dop50.astron.nl");
          string dbName = kvmap.getString ("db", dbUser);
          string dbType = kvmap.getString ("dbtype", "casa");
          string tableName = kvmap.getString ("table", "");
          if (tableName.empty()) {
            tableName = kvmap.getString ("tablename", "MeqParm");
          }
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
          } else if (cmd == SET) {
            KeyValueMap kvmap = KeyParser::parse (line);
            vector<double> defSteps = parmtab->getDefaultSteps();
            defSteps[0] = kvmap.getDouble ("stepx", defSteps[0]);
            defSteps[1] = kvmap.getDouble ("stepy", defSteps[1]);
            parmtab->setDefaultSteps (defSteps);
          } else {
            // Other commands expect a possible parmname and keywords
            parmName = getParmName (line);
            KeyValueMap kvmap = KeyParser::parse (line);
            // For export and list functions the parmname defaults to *.
            // Otherwise a parmname or pattern must be given.
            if (cmd!=RANGE && cmd!=SHOW && cmd!=SHOWDEF &&
                cmd!=NAMES && cmd!=NAMESDEF && cmd!=CHECKSHAPE) {
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
                newDefParm (parmName, kvmap, ostr);
              } else if (cmd == SHOWDEF) {
                showParms (parmset, false, ostr);
              } else if (cmd == UPDDEF) {
                ASSERTSTR (! parmset.empty(), "parameter not found");
                updateDefParms (parmset, kvmap, ostr);
                ostr << "Updated " << nrparm << " parm defaultvalue records"
                     << endl;
              } else if (cmd == DELDEF) {
                ASSERTSTR (! parmset.empty(), "parameter not found");
                parmtab->deleteDefValues (parmName);
                ostr << "Deleted " << nrparm << " parm defaultvalue records"
                     << endl;
              }
            } else if (cmd==NEW || cmd==DEL || cmd==SHOW) {
              // Read the given parameters and domains.
              Box domain = getDomain(kvmap, ostr);
              ParmMap parmset;
              parmtab->getValues (parmset, parmName, domain);
              int nrparm = parmset.size();
              int nrvalrec = countParms (parmset);
              if (cmd == NEW) {
                ASSERTSTR (parmset.empty(),
                           "the parameter/domain already exists");
                newParm (parmName, kvmap, ostr);
              } else if (cmd == SHOW) {
                showParms (parmset, true, ostr);
              } else if (cmd == DEL) {
                ASSERTSTR (! parmset.empty(), "parameter/domain not found");
                parmtab->deleteValues (parmName, domain);
                ostr << "Deleted " << nrvalrec << " value records (of "
                     << nrparm << " parms)" << endl;
              }
            } else if (cmd==CHECKSHAPE) {
              ParmMap parmset;
              parmtab->getValues (parmset, parmName, Box());
              checkShape (parmset, ostr);
            } else if (cmd==EXPORT) {
              // Read the table type and name and append switch.
              KeyValueMap kvmap = KeyParser::parse (line);
              string dbUser = kvmap.getString ("user", getUserName());
              string dbHost = kvmap.getString ("host", "dop50.astron.nl");
              string dbName = kvmap.getString ("db", dbUser);
              string dbType = kvmap.getString ("dbtype", "casa");
              string tableName = kvmap.getString ("table", "");
              if (tableName.empty()) {
                tableName = kvmap.getString ("tablename", "MeqParm");
              }
              ASSERTSTR(!tableName.empty(), "No output table name given");
              int append = kvmap.getInt ("append", 0);
              ParmDBMeta meta (dbType, tableName);
              meta.setSQLMeta (dbName, dbUser, "", dbHost);
              ParmDB newtab(meta, append==0);
              ParmMap parmset;
              parmtab->getValues (parmset, parmName, Box());
              int ncopy = exportParms (parmset, newtab, ostr);
              ostr << "Exported " << ncopy << " parms to "
                   << tableName << endl;
            } else if (cmd==NAMES || cmd==NAMESDEF)  {
              // show names matching the pattern.
              showNames (parmName, cmd, ostr);
            } else if (cmd == RANGE) {
              ostr << "Range: ";
              showDomain (parmtab->getRange (parmName), ostr);
              ostr << endl;
            } else {
              cerr << "Unknown command given" << endl;
            }
          }
        }
      }
    } catch (std::exception& ex) {
      cerr << "Exception: " << ex.what() << endl;
    }
  }
  delete parmtab;
  parmtab = 0;
}

int main (int argc, char *argv[])
{
  TEST_SHOW_VERSION (argc, argv, ParmDB);
  INIT_LOGGER(basename(string(argv[0])));
  
  try {
    if (argc > 1) {
      ofstream ostr(argv[1]);
      doIt (argc > 1, ostr);
    } else {
      doIt (true, cout);
    }
    // Print an extra line to be sure the shell prompt is at a new line.
    cout << endl;
  } catch (Exception& ex) {
    cerr << "Caught exception: " << ex << endl;
    return 1;
  }
  
  return 0;
}
