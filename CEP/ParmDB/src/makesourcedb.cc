//# makesourcedb.cc: Fill a SourceDB from an ASCII file
//#
//# Copyright (C) 2008
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

// This program writes patch and source information into a SourceDB.
// The input is read from an ASCII file that can be formatted in various ways.
//
// The program can be run as:
//   makesourcedb in=inname out=outname format="fmt" append=true/false (or 1/0)
// in      gives the input file name
// out     gives the sourcedb name which will be created or appended
// append  defines if the sourcedb is created or appended (default is appended)
// format  defines the format of the input file
// center  defines the field center (ra and dec) of a search cone or box
// radius  defines the radius if searching using a cone
// width   defines the widths in ra and dec if searching using a box
//
//// Also minflux (skip if < minflux), maxflux (always if >= maxflux),
//// beammodel (only WSRT for time being); if beammodel, convert to app flux. 
//
// The format string can be given as an argument. If its value starts with a <
// it means that it is read from the file following it. No file means from the
// text file (headers) itself. A format string in a file is like a normal
// format string, but preceeded with '#(' and followed by ')=format'.
// Whitespace is allowed between these characters.
//
// A format string looks like:
//    name type ra dec cat
// It defines the fields in the input file and which separator is used between
// the fields (in this example all fields are separated by whitespace).
// Note that both in the format string and input file whitespace is ignored
// unless used as separator.
// The default format string is:
//     "Name,Type,Ra,Dec,I,Q,U,V,SpInx,Major,Minor,Phi"
// thus all fields are separated by commas.
// If the format string contains only one character, the default format is used
// with that one character as separator.
// A field name can consists of alphanumeric characters, underscores and colons.
// However, a colon can not be used as the first character.
// In this way a colon can be used as separator as long as it is surrounded by
// whitespace (otherwise it would be part of the field name).
//
// It is possible to define default values in the format string by giving a
// value to a field. The default value will be used if the field in the
// input file is empty (see below). The value must be enclosed in single
// or double quotes. For example:
//     "Name,Type,Ra,Dec,I,Q,U,V,SpInx='1'
// If no default value is given for empty field values, an appropriate default
// is used (which is usually 0).
//
// In a similar way it is possible to define fixed values by predeeding the
// value with the string 'fixed'. For example:
//     "Name,Type,Ra,Dec,I,Q,U,V,SpInx,Major,Minor,Phi, Category=fixed'2'"
// It means that Category is not a field in the input file, but the given
// value is used for all patches and/or sources in the input file.
// So this example will make all the patches/sources Cat2.
//
// It is possible to ignore a column in the input file by giving an empty name
// or the name 'dummy' in the format string. For example:
//     "Name,Type,Ra,Dec,,dummy,I,Q,U,V
// will ignore the two columns in the input file between Dec and I.

// Each line in the input file represents a patch or source.
// Lines starting with a # (possibly preceeded by whitespace) are ignored.
// The order of the fields in the input file must (of course) match the
// order of the fields in the format string. Fields with fixed values can be
// put anywhere in the format string.
// Each value in the input can be enclosed in single or double quotes. Quotes
// must be used if the value contains the separator character.
// A field in the input can be empty. If whitespace is used as separator, an
// empty field must be represented by "" (or '').
// An input line can contain less values than there are fields in the format
// string. Missing values are empty.
//
// An input line can define a patch or a source.
// A patch is defined by having an empty source name, otherwise it is a source.
// Thus an empty source name and empty patch name is invalid.
//
// Currently only 2 source types are supported (Point and Gaussian).
//
// Ra can be specified in various forms.
// If it is a single value, it must be in the Casacore MVAngle format which is:
// - a value followed by a unit (e.g. 1.2rad). Default unit is deg.
// - a string like hh:mm:ss.s (or dd.mm.ss.s). H and m (or d and m) can
//   be used instead of : (or .).
// However, some catalogues use whitespace to separate hh, mm, and ss.
// They can be seen as individual fields (with whitespace as separator).
// Therefore some extra format fields exist which are Rah, Ram, and Ras.
// They define the hh, mm, and ss parts. Instead of Rah one can use Rad which
// defines it as degrees instead of hours.
// The same is true for Dec (which extra fields Dech, Decd, Decm, and Decs).
// Please note that in a value like '-10 23 45.6' the mm and ss parts are
// also treated negative, thus it is handled as -10:23:45.6

// See the various test/tmakesourcdb files for an example.

#include <lofar_config.h>
#include <ParmDB/SourceDB.h>
#include <Common/StringUtil.h>
#include <Common/StreamUtil.h>
#include <string>                // for getline
#include <iostream>
#include <fstream>
#include <sstream>
#include <casa/Quanta/MVAngle.h>
#include <casa/Inputs/Input.h>
#include <casa/BasicSL/Constants.h>
#include <unistd.h>
#include <libgen.h>

using namespace std;
using namespace casa;
using namespace LOFAR;
using namespace BBS;

// Define the sequence nrs of the various fields.
enum FieldNr {
  // First the standard fields.
  NameNr, TypeNr, RaNr, DecNr, INr, QNr, UNr, VNr, SpInxNr,
  MajorNr, MinorNr, PhiNr,
  NrKnownFields,
  // Now other fields
  CatNr=NrKnownFields, PatchNr,
  RahNr, RadNr, RamNr, RasNr, DechNr, DecdNr, DecmNr, DecsNr,
  // Nr of fields.
  NrFields
};

vector<string> fillKnown()
{
  // The order in which the names are pushed must match exactly with the
  // enum above.
  vector<string> names;
  names.reserve (NrKnownFields);
  names.push_back ("Name");
  names.push_back ("Type");
  names.push_back ("Ra");
  names.push_back ("Dec");
  names.push_back ("I");
  names.push_back ("Q");
  names.push_back ("U");
  names.push_back ("V");
  names.push_back ("SpInx");
  names.push_back ("Major");
  names.push_back ("Minor");
  names.push_back ("Phi");
  names.push_back ("Category");
  names.push_back ("Patch");
  names.push_back ("rah");
  names.push_back ("rad");
  names.push_back ("ram");
  names.push_back ("ras");
  names.push_back ("dech");
  names.push_back ("decd");
  names.push_back ("decm");
  names.push_back ("decs");
  ASSERT (names.size() == NrFields);
  return names;
}
// Define all field names.
vector<string> theFieldNames = fillKnown();

enum FieldType {
  KNOWNFIELD = 1,
  FIXEDVALUE = 2,
  SKIPFIELD  = 4,
  DEFAULTVALUE=8
};

struct SdbFormat
{
  // Define fieldnr of the known fields.
  // -1 means not given.
  vector<int> fieldNrs;
  // Define the separators.
  vector<char> sep;
  // Define the field names.
  vector<string> names;
  // Define the field type.
  vector<int> types;
  // Define the fixed values.
  vector<string> values;
};

void checkRaDec (const SdbFormat& sdbf, int nr,
                 int hnr, int dnr, int mnr, int snr)
{
  int  v = sdbf.fieldNrs[nr];
  int hv = sdbf.fieldNrs[hnr];
  int dv = sdbf.fieldNrs[dnr];
  int mv = sdbf.fieldNrs[mnr];
  int sv = sdbf.fieldNrs[snr];
  ASSERTSTR (!(hv>=0 && dv>=0),
             "rah and rad cannot be used both (same for dec)");
  if (v >= 0) {
    ASSERTSTR(hv<0 && dv<0 && mv<0 && sv<0,
              "rah/d/m/s cannot be used if ra is given (same for dec)");
  }
  ASSERTSTR(v>=0 || hv>=0 || dv>=0 || mv>=0 || sv>=0,
            "No Ra or Dec info given");
}

uint ltrim (const string& value, uint st, uint end)
{
  for (; st<end; ++st) {
    if (value[st] != ' '  &&  value[st] != '\t') {
      break;
    }
  }
  return st;
}
  
uint rtrim (const string& value, uint st, uint end)
{
  for (; end>st; --end) {
    if (value[end-1] != ' '  &&  value[end-1] != '\t') {
      break;
    }
  }
  return end;
}

SdbFormat getFormat (const string& format)
{
  // Fill the map with known names.
  map<string,int> nameMap;
  for (uint i=0; i<theFieldNames.size(); ++i) {
    nameMap[toLower(theFieldNames[i])] = i;
  }
  // Skip possible left and right whitespace.
  uint end = format.size();
  uint st  = ltrim (format, 0, end);
  end      = rtrim (format, st, end);
  // Initialize the format.
  SdbFormat sdbf;
  sdbf.fieldNrs.resize (NrFields);
  for (uint i=0; i<NrFields; ++i) {
    sdbf.fieldNrs[i] = -1;
  }
  // Use default if the string is empty.
  if (st >= end-1) {
    char sep = ',';
    if (st == end-1) sep = format[st];
    for (uint i=0; i<NrKnownFields; ++i) {
      sdbf.fieldNrs[i] = i;
      sdbf.sep.push_back (sep);
      sdbf.names.push_back (theFieldNames[i]);
      sdbf.types.push_back (KNOWNFIELD);
      sdbf.values.push_back ("");
    }
    return sdbf;
  }
  // Parse the format string.
  uint nr = 0;
  uint i = st;
  while (i < end) {
    if ((format[i] >= 'a'  &&  format[i] <= 'z')  ||
        (format[i] >= 'A'  &&  format[i] <= 'Z')  ||
        (format[i] >= '0'  &&  format[i] <= '9')  ||
        format[i] == '_'  ||  (format[i] == ':'  &&  i > st)) {
      ++i;    // part of name
    } else {
      // End of name
      string name  = format.substr(st, i-st);
      string lname = toLower(name);
      int fieldType = 0;
      if (lname.empty()  ||  lname == "dummy") {
        fieldType = SKIPFIELD;
      } else {
        map<string,int>::const_iterator namepos = nameMap.find(lname);
        // Fill in fieldnr of a known field.
        if (namepos != nameMap.end()) {
          fieldType = KNOWNFIELD;
          sdbf.fieldNrs[namepos->second] = nr;
        }
      }
      // See if a default or fixed value is given.
      string fixedValue;
      i = ltrim(format, i, end);
      if (i < end  &&  format[i] == '=') {
        i = ltrim(format, i+1, end);
        // See if it is a fixed value.
        bool isDefault = True;
        if (i+5 < end   &&  toLower(format.substr(i,5)) == "fixed") {
          isDefault = False;
          i += 5;
        }
        ASSERTSTR (i<end, "No value given after " << name << '=');
        ASSERTSTR (format[i] == '"'  ||  format[i] == '\'',
                   "value after " << name << "= must start with a quote");
        // Find the ending quote.
        size_t pos = format.find (format[i], i+1);
        ASSERTSTR (pos!=string::npos,
                   "No closing value quote given after " << name << '=');
        fixedValue = format.substr(i+1, pos-i-1);
        i = ltrim(format, pos+1, end);
        if (isDefault) {
          fieldType |= DEFAULTVALUE;
        } else {
          fieldType |= FIXEDVALUE;
        }
      }
      // Now look for a separator.
      char sep = ' ';
      if (!((format[i] >= 'a'  &&  format[i] <= 'z')  ||
            (format[i] >= 'A'  &&  format[i] <= 'Z')  ||
            (format[i] >= '0'  &&  format[i] <= '9')  ||
            format[i] == '_')) {
        sep = format[i];
        i = ltrim(format, i+1, end);
      }
      sdbf.sep.push_back (sep);
      sdbf.names.push_back (name);
      sdbf.types.push_back (fieldType);
      sdbf.values.push_back (fixedValue);
      nr++;
      st = i;
    }
  }
  if (st < end) {
    // The last item was just a name which has to be processed.
    string name = format.substr(st, end-st);
    string lname = toLower(name);
    int fieldType = 0;
    if (lname.empty()  ||  lname == "dummy") {
      fieldType = SKIPFIELD;
    } else {
      map<string,int>::const_iterator namepos = nameMap.find(lname);
      // Fill in fieldnr of a known field.
      if (namepos != nameMap.end()) {
        sdbf.fieldNrs[namepos->second] = nr;
        fieldType = KNOWNFIELD;
      }
    }
    sdbf.names.push_back (name);
    sdbf.types.push_back (fieldType);
    sdbf.values.push_back ("");
  }
  // Make sure Ra and Dec are given correctly.
  checkRaDec (sdbf, RaNr, RahNr, RadNr, RamNr, RasNr);
  checkRaDec (sdbf, DecNr, DechNr, DecdNr, DecmNr, DecsNr);
  return sdbf;
}

SourceInfo::Type string2type (const string& str)
{
  string s = toLower(str);
  if (s == "point"  ||  s.empty()) {
    return SourceInfo::POINT;
  } else if (s == "gaussian") {
    return SourceInfo::GAUSSIAN;
  } else if (s == "disk") {
    return SourceInfo::DISK;
  } else if (s == "shapelet") {
    return SourceInfo::SHAPELET;
  } else if (s == "sun") {
    return SourceInfo::SUN;
  } else if (s == "moon") {
    return SourceInfo::MOON;
  } else if (s == "jupiter") {
    return SourceInfo::JUPITER;
  } else if (s == "mars") {
    return SourceInfo::MARS;
  } else if (s == "venus") {
    return SourceInfo::VENUS;
  }
  ASSERTSTR (false, str << " is an invalid source type");
}

string unquote (const string& value)
{
  string res(value);
  if (res.size() > 1) {
    int last = res.size() - 1;
    if (last >= 1  &&  ((res[0] == '"'  && res[last] == '"')  ||
                        (res[0] == '\'' && res[last] == '\''))) {
      res = res.substr(1,last-1);
    }
  }
  return res;
}

string getValue (const vector<string>& values, int nr,
                 const string& defVal=string())
{
  if (nr < 0) {
    return defVal;
  }
  return unquote (values[nr]);
}

int string2int (const vector<string>& values, int nr, int defVal)
{
  string value = getValue (values, nr);
  if (value.empty()) {
    return defVal;
  }
  istringstream istr(value);
  int val;
  istr >> val;
  return val;
}

double string2real (const string& value, double defVal)
{
  if (value.empty()) {
    return defVal;
  }
  istringstream istr(value);
  double val;
  istr >> val;
  return val;
}

double string2real (const vector<string>& values, int nr, double defVal)
{
  return string2real (getValue (values, nr), defVal);
}

double string2pos (const vector<string>& values, int pnr, int hnr, int dnr,
                   int mnr, int snr, bool canUseColon)
{
  double deg = 0;
  bool fnd = false;
  if (pnr >= 0) {
    string value = getValue(values, pnr);
    if (! value.empty()) {
      if (!canUseColon) {
        ASSERTSTR (value.find(':') == string::npos,
                   "Colons cannot be used in declination value " << value);
      }
      Quantity q;
      ASSERT (MVAngle::read (q, values[pnr]));
      deg = q.getValue ("deg");
      fnd = true;
    }
  } else {
    if (hnr >= 0) {
      string value = getValue(values, hnr);
      if (! value.empty()) {
        deg = string2real (values[hnr], 0);
        fnd = true;
      }
    } else if (dnr >= 0) {
      string value = getValue(values, dnr);
      if (! value.empty()) {
        deg = string2real (values[dnr], 0);
        fnd = true;
      }
    }
    double ms = 0;
    if (mnr >= 0) {
      string value = getValue(values, mnr);
      if (! value.empty()) {
        ms = string2real (values[mnr], 0);
        fnd = true;
      }
    }
    if (snr >= 0) {
      string value = getValue(values, snr);
      if (! value.empty()) {
        ms += string2real (values[snr], 0) / 60;
        fnd = true;
      }
    }
    if (deg < 0) {
      deg -= ms/60;
    } else {
      deg += ms/60;
    }
    if (hnr >= 0) {
      deg *= 15;
    }
  }
  if (fnd) {
    Quantity q (deg, "deg");
    return q.getValue ("rad");
  }
  return 1e-9;
}

struct SearchInfo
{
  double ra, dec, sinDec, cosDec, cosRadius, raStart, raEnd, decStart, decEnd;
  bool   search;        // false no search info given
  bool   asCone;        // true is search in a cone, otherwise a box
};

// Get the search cone or box values.
SearchInfo getSearchInfo (const string& center, const string& radius,
                          const string& width)
{
  SearchInfo searchInfo;
  if (center.empty()) {
    searchInfo.search = false;
  } else {
    searchInfo.search = true;
    vector<string> pos = StringUtil::split (center, ',');
    ASSERTSTR (pos.size() == 2, "center not specified as ra,dec");
    searchInfo.ra  = string2pos (pos, 0, -1, -1, -1, -1, true);
    searchInfo.dec = string2pos (pos, 1, -1, -1, -1, -1, false);
    searchInfo.sinDec = sin(searchInfo.dec);
    searchInfo.cosDec = cos(searchInfo.dec);
    ASSERTSTR (radius.empty() != width.empty(),
               "radius OR width must be given if center is given (not both)");
    if (radius.empty()) {
      double raw, decw;
      searchInfo.asCone = false;
      pos = StringUtil::split(width, ',');
      ASSERTSTR (pos.size() == 1  ||  pos.size() == 2,
                 "width should be specified as 1 or 2 values");
      raw = string2pos (pos, 0, -1, -1, -1, -1, true);
      if (pos.size() > 1) {
        decw = string2pos (pos, 1, -1, -1, -1, -1, false);
      } else {
        decw = raw;
      }
      searchInfo.raStart  = searchInfo.ra  - raw/2;
      searchInfo.raEnd    = searchInfo.ra  + raw/2;
      searchInfo.decStart = searchInfo.dec - decw/2;
      searchInfo.decEnd   = searchInfo.dec + decw/2;
    } else {
      searchInfo.asCone = true;
      pos[0] = radius;
      searchInfo.cosRadius = cos (string2pos (pos, 0, -1, -1, -1, -1, false));
    }
  }
  return searchInfo;
}

bool matchSearchInfo (double ra, double dec, const SearchInfo& si)
{
  if (!si.search) {
    return true;
  }
  bool match = false;
  if (si.asCone) {
    match = (si.cosRadius <=
             si.sinDec * sin(dec) + si.cosDec * cos(dec) * cos(si.ra - ra));
  } else {
    // Ra can be around 0 or 360 degrees, so make sure all cases are handled.
    ra -= C::_2pi;
    for (int i=0; i<4; ++i) {
      if (ra >= si.raStart  &&  ra <= si.raEnd) {
        match = true;
        break;
      }
      ra += C::_2pi;
    }
    if (match) {
      match = (dec >= si.decStart  &&  dec <= si.decEnd);
    }
  }
  return match;
}

void add (ParmMap& defVal, const string& name, double value)
{
  defVal.define (name, ParmValueSet(ParmValue(value)));
}

void process (const string& line, SourceDB& pdb, const SdbFormat& sdbf,
              bool check, int& nrpatch, int& nrsource,
              int& nrpatchfnd, int& nrsourcefnd, const SearchInfo& searchInfo)
{
  //  cout << line << endl;
  // Hold the values.
  ParmMap defValues;
  vector<string> values;
  // Process the line.
  uint end = line.size();
  uint st  = ltrim(line, 0, end);
  for (uint i=0; i<sdbf.names.size(); ++i) {
    string value;
    if ((sdbf.types[i] & FIXEDVALUE) == FIXEDVALUE) {
      value = sdbf.values[i];
    } else if (st < end) {
      size_t pos = line.find (sdbf.sep[i], st);
      if (pos == string::npos) {
        pos = end;
      }
      value = line.substr(st, rtrim(line, st, pos) - st);
      st = ltrim (line, pos+1, end);
    }
    if (value.empty()  &&  (sdbf.types[i] & DEFAULTVALUE) == DEFAULTVALUE) {
      value = sdbf.values[i];
    }
    values.push_back (value);
    if ((sdbf.types[i] & SKIPFIELD) != SKIPFIELD) {
      if ((sdbf.types[i] & KNOWNFIELD) != KNOWNFIELD) {
        add (defValues, sdbf.names[i], string2real(unquote(value), 0));
      }
    }
  }
  // Now handle the standard fields.
  string srcName = getValue(values, sdbf.fieldNrs[NameNr]);
  SourceInfo::Type srctype = string2type (getValue(values, sdbf.fieldNrs[TypeNr]));
  double ra = string2pos (values, sdbf.fieldNrs[RaNr],
                          sdbf.fieldNrs[RahNr],
                          sdbf.fieldNrs[RadNr],
                          sdbf.fieldNrs[RamNr],
                          sdbf.fieldNrs[RasNr],
                          true);
  double dec = string2pos (values, sdbf.fieldNrs[DecNr],
                           sdbf.fieldNrs[DechNr],
                           sdbf.fieldNrs[DecdNr],
                           sdbf.fieldNrs[DecmNr],
                           sdbf.fieldNrs[DecsNr],
                           false);
  int cat = string2int (values, sdbf.fieldNrs[CatNr], 2);
  double fluxI = string2real (values, sdbf.fieldNrs[INr], 1);
  add (defValues, "I", fluxI);
  add (defValues, "Q", string2real (values, sdbf.fieldNrs[QNr], 0));
  add (defValues, "U", string2real (values, sdbf.fieldNrs[UNr], 0));
  add (defValues, "V", string2real (values, sdbf.fieldNrs[VNr], 0));
  add (defValues, "SpInx", string2real (values, sdbf.fieldNrs[SpInxNr], 1));
  string patch = getValue (values, sdbf.fieldNrs[PatchNr]);

//   double ra = string2pos (fields, 2, -1e9);
  if (srctype == SourceInfo::GAUSSIAN) {
    add (defValues, "Major", string2real (values, sdbf.fieldNrs[MajorNr], 1));
    add (defValues, "Minor", string2real (values, sdbf.fieldNrs[MinorNr], 1));
    add (defValues, "Phi",   string2real (values, sdbf.fieldNrs[PhiNr], 1));
  }
  // Add the source.
  // Do not check for duplicates yet.
  if (srcName.empty()) {
    ASSERTSTR (!patch.empty(), "Source and/or patch name must be filled in");
    if (matchSearchInfo (ra, dec, searchInfo)) {
      pdb.addPatch (patch, cat, fluxI, ra, dec, check);
      nrpatchfnd++;
    }
    nrpatch++;
  } else {
    if (matchSearchInfo (ra, dec, searchInfo)) {
      if (patch.empty()) {
        pdb.addSource (srcName, cat, fluxI, srctype, defValues, ra, dec, check);
      } else {
        pdb.addSource (patch, srcName, srctype, defValues, ra, dec, check);
      }
      nrsourcefnd++;
    }
    nrsource++;
  }
}

void make (const string& in, const string& out,
           const string& format, bool append, bool check,
           const SearchInfo& searchInfo)
{
  // Analyze the format string.
  SdbFormat sdbf = getFormat (format);
  // Create/open the sourcedb and lock it for write.
  ParmDBMeta ptm("casa", out);
  SourceDB pdb(ptm, !append);
  pdb.lock (true);
  int nrpatch     = 0;
  int nrsource    = 0;
  int nrpatchfnd  = 0;
  int nrsourcefnd = 0;
  if (in.empty()) {
    process (string(), pdb, sdbf, check, nrpatch, nrsource,
             nrpatchfnd, nrsourcefnd, searchInfo);
  } else {
    ifstream infile(in.c_str());
    ASSERTSTR (infile, "File " << in << " could not be opened");
    string line;
    // Read first line.
    getline (infile, line);
    while (infile) {
      // Remove comments.
      bool skip = true;
      for (uint i=0; i<line.size(); ++i) {
        if (line[i] == '#') {
          break;
        }
        if (line[i] != ' '  &&  line[i] != '\t') {
          skip = false;
          break;
        }
      }
      if (!skip) {
        process (line, pdb, sdbf, check, nrpatch, nrsource,
                 nrpatchfnd, nrsourcefnd, searchInfo);
      }
      // Read next line
      getline (infile, line);
    }
  }
  cout << "Wrote " << nrpatchfnd << " patches (out of " << nrpatch << ") and "
       << nrsourcefnd << " sources (out of " << nrsource << ") into "
       << pdb.getParmDBMeta().getTableName() << endl;
  vector<string> dp(pdb.findDuplicatePatches());
  if (dp.size() > 0) {
    cout << "Duplicate patches: " << dp << endl;
  }
  vector<string> ds(pdb.findDuplicateSources());
  if (ds.size() > 0) {
    cout << "Duplicate sources: " << ds << endl;
  }
}

// Read the format from the file.
// It should be contained in a line like # format = .
string readFormat (string file, const string& catFile)
{
  // Use catalog itself if needed.
  if (file.empty()) {
    file = catFile;
  }
  // Read file until format line is found or until non-comment is found.
  ifstream infile(file.c_str());
  ASSERTSTR (infile, "File " << file
             << " containing format string could not be opened");
  string line;
  getline (infile, line);
  casa::Regex regex("^ *# *\\( *.* *\\) *= *format *$");
  casa::Regex regexs1("^ *# *\\( *");
  casa::Regex regexs2("\\) *= *format *$");
  while (infile) {
    uInt st=0;
    st = lskipws(line, st, line.size());   // skip whitespace
    if (st < line.size()) {
      if (line[st] != '#') {
        break;                             // data line
      }
      casa::String sline(line);
      if (sline.matches (regex)) {
        sline.gsub (regexs1, string());
        sline.gsub (regexs2, string());
        return sline;
      }
    }
    getline (infile, line);
  }
  // No format line found, so use default.
  return string();
}

int main (int argc, char* argv[])
{
  // Not all versions of basename accept a const char.
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);
  try {
    // Get the inputs.
    Input inputs(1);
    inputs.version ("GvD 2009-Jan-26");
    inputs.create("in", "",
                  "Input file name", "string");
    inputs.create("out", "",
                  "Output sourcedb name", "string");
    inputs.create("format", "Name,Type,Ra,Dec,I,Q,U,V,SpInx,Major,Minor,Phi",
                  "Format of the input lines or name of file containing format",
                  "string");
    inputs.create("append", "true",
                  "Append to possibly existing sourcedb?", "bool");
    inputs.create("check", "false",
                  "Check immediately for duplicate entries?", "bool");
    inputs.create("center", "",
                  "Field center as ra,dec", "string");
    inputs.create("radius", "",
                  "Cone search radius", "string");
    inputs.create("width", "",
                  "Search box width as 1 or 2 values (e.g. 2deg,3deg)",
                  "string");
    inputs.create("minflux", "",
                  "Only select sources >= minflux Jy", "string");
    inputs.create("maxflux", "",
                  "Always select sources >= maxflux Jy", "string");
    inputs.create("beammodel", "",
                  "If given, apply beammodel to make fluxes apparent",
                  "WSRT or LOFAR");
    inputs.readArguments(argc, argv);
    string in = inputs.getString("in");
    string out = inputs.getString("out");
    ASSERTSTR (!out.empty(), "no output sourcedb name given");
    string format = inputs.getString("format");
    bool append = inputs.getBool("append");
    bool check  = inputs.getBool("check");
    string center = inputs.getString ("center");
    string radius = inputs.getString ("radius");
    string width  = inputs.getString ("width");
//    double minFlux = inputs.getDouble ("minflux");
//    double maxFlux = inputs.getDouble ("maxflux");
    string beamModel = inputs.getString ("beammodel");
    // Check if the format has to be read from a file.
    // It is if it starts with a <. Te filename should follow it. An empty
    // filename means reading from the catalog file itself.
    if (! format.empty()  &&  format[0] == '<') {
      // Skip optional whitespace.
      uInt st=1;
      st = lskipws (format, st, format.size());
      // Read format from file.
      format = readFormat (format.substr(st), in);
    }
    make (in, out, format, append, check,
          getSearchInfo (center, radius, width));
  } catch (std::exception& x) {
    std::cerr << "Caught exception: " << x.what() << std::endl;
    return 1;
  }
  
  return 0;
}
