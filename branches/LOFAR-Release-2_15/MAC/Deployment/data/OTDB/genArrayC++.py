#!/bin/env python
import os
import re

def grep(string,list):
    expr = re.compile(string, re.MULTILINE)
    return [elem for elem in list if expr.search(open(elem).read())]

def lgrep(string,list):
    expr = re.compile(string, re.MULTILINE)
    return [ line for line in list if expr.search(line) ]

def genHeader(file,className):
    print >>file, "#include <lofar_config.h>"
    print >>file, "#include <Common/LofarLogger.h>"
    print >>file, "#include <Common/StringUtil.h>"
    print >>file, "#include <Common/StreamUtil.h>"
    print >>file, '#include "%s.h"' % className
    print >>file
    print >>file, "using namespace pqxx;"
    print >>file, "namespace LOFAR {"
    print >>file, "  using namespace StringUtil;"
    print >>file, "  namespace OTDB {"
    print >>file

def genConstructor(file, className, fieldList):
    print >>file, "// Constructor"
    print >>file, "%s::%s(uint aTreeID, uint aRecordID, const string& aParent, const string& arrayString):" % (className, className)
    print >>file, "  itsTreeID(aTreeID),"
    print >>file, "  itsRecordID(aRecordID),"
    print >>file, "  itsNodename(aParent)"
    print >>file, "{"
    print >>file, "  string input(arrayString);"
    print >>file, '  rtrim(input, "}\\")");'
    print >>file, '  ltrim(input, "(\\"{");'
    print >>file, "  vector<string> fields(split(input, ','));"
    print >>file, '  ASSERTSTR(fields.size() == %d, fields.size() << " fields iso %d");' % (len(fieldList), len(fieldList));
    print >>file
    idx = 0
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, "  %s = fields[%d];" % (args[1], idx)
      if args[3] in tInt:
        print >>file, "  %s = StringToInt32(fields[%d]);" % (args[1], idx)
      if args[3] in tUint:
        print >>file, "  %s = StringToUint32(fields[%d]);" % (args[1], idx)
      if args[3] in tBool:
        print >>file, "  %s = StringToBool(fields[%d]);" % (args[1], idx)
      if args[3] in tFlt:
        print >>file, "  %s = StringToFloat(fields[%d]);" % (args[1], idx)
      idx += 1
    print >>file, "}"
    print >>file
    print >>file, '%s::%s(): itsTreeID(0),itsRecordID(0), itsNodename("")' % (className, className)
    print >>file, "{"
    idx = 0
    for field in fieldList:
      args = field.split()
      if args[3] in tInt + tUint:
        print >>file, "  %s = 0;" % args[1]
      if args[3] in tBool:
        print >>file, "  %s = false;" % args[1]
      if args[3] in tFlt:
        print >>file, "  %s = 0.0;" % args[1]
      idx += 1
    print >>file, "}"
    print >>file

def genGetRecordsFunction1(file,className,fieldList):
    print >>file, "// getRecords(connection, treeID)"
    print >>file, "vector<%s> %s::getRecords(OTDBconnection *conn, uint32 treeID)" % (className, className)
    print >>file, "{"
    print >>file, "  vector<%s>  container;" % className
    print >>file
    print >>file, '  work    xAction(*(conn->getConn()), "getRecord");'
    print >>file, '  string  command(formatString("SELECT * from %sgetRecords(%%d)", treeID));' % className
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  uint32  nrRecs(res.size());"
    print >>file, "  for (uint i = 0; i < nrRecs; i++) {"
    print >>file, "    uint32   recordID;"
    print >>file, '    res[i]["recordid"].to(recordID);'
    print >>file, "    string   nodeName;"
    print >>file, '    res[i]["nodename"].to(nodeName);'
    print >>file, "    container.push_back(%s(treeID,recordID,nodeName,res[i][3].c_str()));" % className
    print >>file, "  }"
    print >>file, "  return(container);"
    print >>file, "}"
    print >>file

def genGetRecordsFunction2(file,className,fieldList):
    print >>file, "// getRecords(connection, treeID, nodename)"
    print >>file, "vector<%s> %s::getRecords(OTDBconnection *conn, uint32 treeID, const string& nodename)" % (className, className)
    print >>file, "{"
    print >>file, "  vector<%s>  container;" % className
    print >>file
    print >>file, '  work    xAction(*(conn->getConn()), "getRecord");'
    print >>file, '  string  command(formatString("SELECT * from %sgetRecords(%%d, \'%%s\')", treeID, nodename.c_str()));' % className
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  uint32  nrRecs(res.size());"
    print >>file, "  for (uint i = 0; i < nrRecs; i++) {"
    print >>file, "    uint32   recordID;"
    print >>file, '    res[i]["recordid"].to(recordID);'
    print >>file, "    string   nodeName;"
    print >>file, '    res[i]["nodename"].to(nodeName);'
    print >>file, "    container.push_back(%s(treeID,recordID,nodeName,res[i][3].c_str()));" % className
    print >>file, "  }"
    print >>file, "  return(container);"
    print >>file, "}"
    print >>file

def genGetRecordFunction1(file,className,fieldList):
    print >>file, "// getRecord(connection, recordID)"
    print >>file, "%s %s::getRecord(OTDBconnection *conn, uint32 recordID)" % (className, className)
    print >>file, "{"
    print >>file, '  work    xAction(*(conn->getConn()), "getRecord");'
    print >>file, '  string  command(formatString("SELECT * from %sgetRecord(%%d)", recordID));' % className
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  if (!res.size()) {"
    print >>file, "     return (%s());" % className
    print >>file, "  }"
    print >>file, "  uint32   treeID;"
    print >>file, '  res[0]["treeid"].to(treeID);'
    print >>file, "  string   nodeName;"
    print >>file, '  res[0]["nodename"].to(nodeName);'
    print >>file, "  return(%s(treeID,recordID,nodeName,res[0][3].c_str()));" % className
    print >>file, "}"
    print >>file

def genGetRecordFunction2(file,className,fieldList):
    print >>file, "// getRecord(connection, treeID, nodename)"
    print >>file, "%s %s::getRecord(OTDBconnection *conn, uint32 treeID, const string& nodename)" % (className, className)
    print >>file, "{"
    print >>file, '  work    xAction(*(conn->getConn()), "getRecord");'
    print >>file, '  string  command(formatString("SELECT * from %sgetRecord(%%d, \'%%s\')", treeID, nodename.c_str()));' % className
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  if (!res.size()) {"
    print >>file, "     return (%s());" % className
    print >>file, "  }"
    print >>file, "  uint32   recordID;"
    print >>file, '  res[0]["recordid"].to(recordID);'
    print >>file, "  string   nodeName;"
    print >>file, '  res[0]["nodename"].to(nodeName);'
    print >>file, "  return(%s(treeID,recordID,nodeName,res[0][3].c_str()));" % className
    print >>file, "}"
    print >>file

def genGetRecordsOnTreeList(file,className,fieldList):
    print >>file, "// getRecordsOnTreeList(connection, vector<treeid>)"
    print >>file, "vector<%s> %s::getRecordsOnTreeList  (OTDBconnection *conn, vector<uint32> treeIDs)" % (className, className)
    print >>file, "{"
    print >>file, "  vector<%s>  container;" % className
    print >>file
    print >>file, "  ostringstream oss;"
    print >>file, '  writeVector(oss, treeIDs, ",", "{", "}");'
    print >>file, '  string  command(formatString("SELECT * from %sgetRecordsOnTreeList(\'%%s\')", oss.str().c_str()));' % className
    print >>file, '  work    xAction(*(conn->getConn()), "getRecordsOnTreeList");'
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  uint32  nrRecs(res.size());"
    print >>file, "  for (uint i = 0; i < nrRecs; i++) {"
    print >>file, "    uint32   treeID;"
    print >>file, '    res[i]["treeid"].to(treeID);'
    print >>file, "    uint32   recordID;"
    print >>file, '    res[i]["recordid"].to(recordID);'
    print >>file, "    string   nodeName;"
    print >>file, '    res[i]["nodename"].to(nodeName);'
    print >>file, "    container.push_back(%s(treeID,recordID,nodeName,res[i][3].c_str()));" % className
    print >>file, "  }"
    print >>file, "  return(container);"
    print >>file, "}"
    print >>file

def genGetRecordsOnRecordList(file,className,fieldList):
    print >>file, "// getRecordsOnRecordList(connection, vector<RecordID>)"
    print >>file, "vector<%s> %s::getRecordsOnRecordList(OTDBconnection *conn, vector<uint32> recordIDs)" % (className, className)
    print >>file, "{"
    print >>file, "  vector<%s>  container;" % className
    print >>file
    print >>file, "  ostringstream oss;"
    print >>file, '  writeVector(oss, recordIDs, ",", "{", "}");'
    print >>file, '  string  command(formatString("SELECT * from %sgetRecordsOnRecordList(\'%%s\')", oss.str().c_str()));' % className
    print >>file, '  work    xAction(*(conn->getConn()), "getRecordsOnRecordList");'
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  uint32  nrRecs(res.size());"
    print >>file, "  for (uint i = 0; i < nrRecs; i++) {"
    print >>file, "    uint32   treeID;"
    print >>file, '    res[i]["treeid"].to(treeID);'
    print >>file, "    uint32   recordID;"
    print >>file, '    res[i]["recordid"].to(recordID);'
    print >>file, "    string   nodeName;"
    print >>file, '    res[i]["nodename"].to(nodeName);'
    print >>file, "    container.push_back(%s(treeID,recordID,nodeName,res[i][3].c_str()));" % className
    print >>file, "  }"
    print >>file, "  return(container);"
    print >>file, "}"
    print >>file

def genGetFieldOnRecordList(file,className,fieldList):
    print >>file, "// getFieldOnRecordList(connection, fieldname, vector<RecordID>)"
    print >>file, "vector<string> %s::getFieldOnRecordList(OTDBconnection *conn, const string& fieldname, vector<uint32> recordIDs)" % className
    print >>file, "{"
    print >>file, "  vector<string>  container;"
    print >>file
    print >>file, "  int	fieldIdx(fieldnameToNumber(fieldname));"
    print >>file, "  if (fieldIdx < 0) {"
    print >>file, '    LOG_FATAL_STR("Field " << fieldname << " is not defined for structure %s");' % className
    print >>file, "    return (container);"
    print >>file, "  }"
    print >>file, "  ostringstream oss;"
    print >>file, '  writeVector(oss, recordIDs, ",", "{", "}");'
    print >>file, '  string  command(formatString("SELECT * from %sgetFieldOnRecordList(%%d, \'%%s\')", fieldIdx, oss.str().c_str()));' % className
    print >>file, '  work    xAction(*(conn->getConn()), "getFieldOnRecordList");'
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  uint32  nrRecs(res.size());"
    print >>file, "  for (uint i = 0; i < nrRecs; i++) {"
    print >>file, '    container.push_back(res[i][0].c_str() ? res[i][0].c_str() : "");'
    print >>file, "  }"
    print >>file, "  return(container);"
    print >>file, "}"
    print >>file

def genSaveRecord(file,className):
    print >>file, "// save(connection)"
    print >>file, "bool %s::save(OTDBconnection *conn)" % className
    print >>file, "{"
    print >>file, '  string  command(formatString("SELECT * from %sSaveRecord(%%d, %%d, %%d, \'%%s\', \'{%%s}\')", conn->getAuthToken(), itsRecordID, itsTreeID, itsNodename.c_str(), fieldValues().c_str()));' % className
    print >>file, '  work    xAction(*(conn->getConn()), "saveRecord%s");' % className
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  bool    updateOK(false);"
    print >>file, '  res[0]["%ssaverecord"].to(updateOK);' % className
    print >>file, "  if (updateOK) {"
    print >>file, "    xAction.commit();"
    print >>file, "  }"
    print >>file, "  return(updateOK);"
    print >>file, "}"
    print >>file

def genSaveField(file,className,fieldList):
    print >>file, "// saveField(connection, fieldIndex)"
    print >>file, "bool %s::saveField(OTDBconnection *conn, uint fieldIndex)" % className
    print >>file, "{"
    print >>file, '  ASSERTSTR(fieldIndex < %d, "%s has only %d fields, not " << fieldIndex);' % (len(fieldList), className, len(fieldList))
    print >>file, '  string  command(formatString("SELECT * from %sSaveField(%%d, %%d, %%d, %%d, \'%%s\')", conn->getAuthToken(), itsRecordID, itsTreeID, fieldIndex+1, fieldValue(fieldIndex).c_str()));' % className
    print >>file, '  work    xAction(*(conn->getConn()), "saveField%s");' % className
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  bool    updateOK(false);"
    print >>file, '  res[0]["%ssavefield"].to(updateOK);' % className
    print >>file, "  if (updateOK) {"
    print >>file, "    xAction.commit();"
    print >>file, "  }"
    print >>file, "  return(updateOK);"
    print >>file, "}"
    print >>file

def genSaveFields(file,className,fieldList):
    print >>file, "// saveFields(connection, fieldIndex, vector<%s>)" % className
    print >>file, "bool %s::saveFields(OTDBconnection *conn, uint fieldIndex, vector<%s>  records)" % (className,className)
    print >>file, "{"
    print >>file, '  ASSERTSTR(fieldIndex < %d, "%s has only %d fields, not " << fieldIndex);' % (len(fieldList), className, len(fieldList))
    print >>file, "  string	recordNrs;"
    print >>file, "  string	fieldValues;"
    print >>file, "  size_t	nrRecs = records.size();"
    print >>file, "  recordNrs.reserve(nrRecs*5);    // speed up things a little"
    print >>file, "  fieldValues.reserve(nrRecs*30);"
    print >>file, "  for (uint i = 0; i < nrRecs; i++) {"
    print >>file, "    recordNrs.append(toString(records[i].recordID()));"
    print >>file, "    fieldValues.append(records[i].fieldValue(fieldIndex));"
    print >>file, "    if (i < nrRecs-1) {"
    print >>file, '      recordNrs.append(",");'
    print >>file, '      fieldValues.append(",");'
    print >>file, "    }"
    print >>file, "  }"
    print >>file
    print >>file, '  string  command(formatString("SELECT * from %sSaveFields(%%d, %%d, \'{%%s}\', \'{%%s}\')", conn->getAuthToken(), fieldIndex+1, recordNrs.c_str(), fieldValues.c_str()));' % className
    print >>file, '  work    xAction(*(conn->getConn()), "saveFields%s");' % className
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  bool    updateOK(false);"
    print >>file, '  res[0]["%ssavefields"].to(updateOK);' % className
    print >>file, "  if (updateOK) {"
    print >>file, "    xAction.commit();"
    print >>file, "  }"
    print >>file, "  return(updateOK);"
    print >>file, "}"
    print >>file

def genFieldNamesFunction(file,className,fieldList):
    print >>file, "// fieldNames()"
    print >>file, "string %s::fieldNames() const" % className
    print >>file, "{"
    print >>file, '  return("'+fieldNameList(fieldList)+'");'
    print >>file, "};"
    print >>file

def genFieldValuesFunction(file,className,fieldList):
    print >>file, "// fieldValues()"
    print >>file, "string %s::fieldValues() const" % className
    print >>file, "{"
    print >>file, "    ostringstream    oss;"
    count = 0
    for field in fieldList:
      args = field.split()
      if count % 3 == 0:
         print >>file, "    oss",
      if count != 0:
         print >>file, '<< ","',
      if args[3] in tText + tInt + tUint + tFlt:
         print >>file, '<< %s' % args[1],
      if args[3] in tBool:
         print >>file, '<< (%s ? "true" : "false")' % args[1],
      count += 1
      if count % 3 == 0:
         print >>file, ";"
    print >>file, ";"
    print >>file
    print >>file, "    return (oss.str());"
    print >>file, "};"
    print >>file
    print >>file, "// fieldValue(fieldIndex)"
    print >>file, "string %s::fieldValue(uint fieldIndex) const" % className
    print >>file, "{"
    print >>file, "  switch(fieldIndex) {"
    count = 0
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
         print >>file, '  case %d: return(%s); break;' % (count, args[1])
      if args[3] in tInt + tUint + tFlt:
         print >>file, '  case %d: return(toString(%s)); break;' % (count, args[1])
      if args[3] in tBool:
         print >>file, '  case %d: return(%s ? "true" : "false"); break;' % (count, args[1])
      count += 1
    print >>file, "  };"
    print >>file, '  return("");'
    print >>file, "};"
    print >>file

def genFieldDictFunction(file,className,fieldList):
    print >>file, "// fieldDict()"
    print >>file, "string %s::fieldDict() const" % className
    print >>file, "{"
    print >>file, "    ostringstream    oss;"
    count = 0
    for field in fieldList:
      args = field.split()
      if count % 3 == 0:
         print >>file, "    oss",
      if count != 0:
         print >>file, '<< ","',
      if args[3] in tText + tInt + tUint + tFlt:
         print >>file, '<< "%s:" << %s' % (args[1], args[1]),
      if args[3] in tBool:
         print >>file, '<< "%s:" << (%s ? "true" : "false")' % (args[1], args[1]),
      count += 1
      if count % 3 == 0:
         print >>file, ";"
    print >>file, ";"
    print >>file
    print >>file, "    return (oss.str());"
    print >>file, "};"
    print >>file

def genPrintFunction(file,className,fieldList):
    print >>file, "// print(os)"
    print >>file, "ostream& %s::print(ostream& os) const" % className
    print >>file, "{"
    print >>file, '  os << "{recordID:" << itsRecordID << ",treeID:" << itsTreeID << ",nodename:" << itsNodename;'
    print >>file, '  os << ",{" << fieldDict() << "}";'
    print >>file, '  return (os);'
    print >>file, "}"
    print >>file

def genCompareFunction(file,className,fieldList):
    print >>file, "// operator=="
    print >>file, "bool %s::operator==(const %s& that) const" % (className, className)
    print >>file, "{"
    print >>file, "  return (",
    count = 0
    for field in fieldList:
      args = field.split()
      if count != 0:
         print >>file, " && ",
      print >>file, "%s==that.%s" % (args[1], args[1]),
      count += 1
    print >>file, ");"
    print >>file, "}"
    print >>file



def genFieldName2Number(file, className, fieldList):
    print >>file, "// fieldnameToNumber(fieldname)"
    print >>file, "int %s::fieldnameToNumber(const string& fieldname)" % className
    print >>file, "{"
    count = 1
    for field in fieldList:
      args = field.split()
      print >>file, '  if (fieldname == "%s") return(%d);' % (args[1], count)
      count += 1
    print >>file, "  return(-1);"
    print >>file, "}"
    print >>file

def genEndOfFile(file):
    print >>file
    print >>file, "  } // namespace OTDB"
    print >>file, "} // namespace LOFAR"
    print >>file

def genHeaderFile(file,className,fieldList):
    print >>file, "#ifndef LOFAR_OTDB_%s_H" % className.upper()
    print >>file, "#define LOFAR_OTDB_%s_H" % className.upper()
    print >>file
    print >>file, "#include <pqxx/pqxx>"
    print >>file, "#include <OTDB/OTDBconnection.h>"
    print >>file, "#include <Common/LofarTypes.h>"
    print >>file, "#include <Common/lofar_string.h>"
    print >>file, "#include <Common/lofar_vector.h>"
    print >>file, "namespace LOFAR {"
    print >>file, "  namespace OTDB {"
    print >>file
    print >>file, "class %s" % className
    print >>file, "{"
    print >>file, "public:"
    print >>file, "  %s(uint aTreeID, uint aRecordID, const string& aParent, const string& arrayString);" % className
    print >>file, "  %s();" % className
    print >>file
    print >>file, "  // get a single record"
    print >>file, "  static %s         getRecord (OTDBconnection *conn, uint32 recordID);" % className
    print >>file, "  static %s         getRecord (OTDBconnection *conn, uint32 treeID, const string& node);" % className
    print >>file, "  // get a all record of 1 tree [and 1 type]"
    print >>file, "  static vector<%s> getRecords(OTDBconnection *conn, uint32 treeID);" % className
    print >>file, "  static vector<%s> getRecords(OTDBconnection *conn, uint32 treeID, const string& node);" % className
    print >>file, "  // get a multiple records of multiple trees"
    print >>file, "  static vector<%s> getRecordsOnTreeList  (OTDBconnection *conn, vector<uint32> treeIDs);" % className
    print >>file, "  static vector<%s> getRecordsOnRecordList(OTDBconnection *conn, vector<uint32> recordIDs);" % className
    print >>file, "  // get a a single field of multiple records"
    print >>file, "  static vector<string> getFieldOnRecordList(OTDBconnection *conn, const string& fieldname, vector<uint32> recordIDs);" 
    print >>file
    print >>file, "  // save this record or 1 field"
    print >>file, "  bool save(OTDBconnection *conn);"
    print >>file, "  bool saveField(OTDBconnection *conn, uint fieldIndex);"
    print >>file, "  // save 1 field of multiple records"
    print >>file, "  static bool saveFields(OTDBconnection *conn, uint fieldIndex, vector<%s>  records);" % className
    print >>file
    print >>file, "  // helper function"
    print >>file, "  static int fieldnameToNumber(const string& fieldname);"
    print >>file, "  string fieldNames () const;"
    print >>file, "  string fieldValues() const;"
    print >>file, "  string fieldDict  () const;"
    print >>file, "  string fieldValue (uint fieldIndex) const;"
    print >>file
    print >>file, "  // data access"
    print >>file, "  uint32 treeID()      const { return (itsTreeID);   }"
    print >>file, "  uint32 recordID()    const { return (itsRecordID); }"
    print >>file, "  string nodeName()    const { return (itsNodename); }"
    print >>file
    print >>file, "  // for operator<<"
    print >>file, "  ostream& print (ostream& os) const;"
    print >>file
    print >>file, "  // operator=="
    print >>file, "  bool operator==(const %s& that) const;" % className
    print >>file
    print >>file, "  // -- datamembers --"
    print >>file, "private:"
    print >>file, "  uint32    itsTreeID;"
    print >>file, "  uint32    itsRecordID;"
    print >>file, "  string    itsNodename;"
    print >>file, "public:"
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, "  string    %s;" % args[1]
      if args[3] in tInt:
        print >>file, "  int32     %s;" % args[1]
      if args[3] in tUint:
        print >>file, "  uint32    %s;" % args[1]
      if args[3] in tBool:
        print >>file, "  bool      %s;" % args[1]
      if args[3] in tFlt:
        print >>file, "  float     %s;" % args[1]
    print >>file, "};"
    print >>file
    print >>file, "// operator<<"
    print >>file, "inline ostream& operator<< (ostream& os, const %s& anObj)" % className
    print >>file, "{ return (anObj.print(os)); }"
    print >>file
    print >>file, "  } // namespace OTDB"
    print >>file, "} // namespace LOFAR"
    print >>file, "#endif"
    print >>file

def fieldNameList(fieldlist):
    result = ""
    for field in fieldlist:
      if result != "":
        result += ","
      result += field.split()[1]
    return result

# MAIN
tText = ["text", "vtext", "ptext" ]
tBool = ["bool", "vbool", "pbool" ]
tInt  = ["int",  "vint",  "pint",  "long", "vlong", "plong" ]
tUint = ["uint", "vuint", "puint", "ulng", "vulng", "pulng" ]
tFlt  = ["flt",  "vflt",  "pflt",  "dbl",  "vdbl",  "pdbl" ]

compfiles = [cf for cf in os.listdir('.') if cf.endswith(".comp")]
DBfiles = grep("^table.",compfiles)
for DBfile in DBfiles:
  tablename = lgrep("^table", open(DBfile).readlines())[0].split()[1]
  print "tablename="+tablename
  fieldLines = lgrep("^field", open(DBfile).readlines())

  file = open(tablename+".cc", "w")
  genHeader                (file, tablename)
  genConstructor           (file, tablename, fieldLines)
  genGetRecordFunction1    (file, tablename, fieldLines)
  genGetRecordFunction2    (file, tablename, fieldLines)
  genGetRecordsFunction1   (file, tablename, fieldLines)
  genGetRecordsFunction2   (file, tablename, fieldLines)
  genGetRecordsOnTreeList  (file, tablename, fieldLines)
  genGetRecordsOnRecordList(file, tablename, fieldLines)
  genGetFieldOnRecordList  (file, tablename, fieldLines)
  genSaveRecord            (file, tablename)
  genSaveField             (file, tablename, fieldLines)
  genSaveFields            (file, tablename, fieldLines)
  genFieldName2Number      (file, tablename, fieldLines)
  genFieldNamesFunction    (file, tablename, fieldLines)
  genFieldValuesFunction   (file, tablename, fieldLines)
  genFieldDictFunction     (file, tablename, fieldLines)
  genPrintFunction         (file, tablename, fieldLines)
  genCompareFunction       (file, tablename, fieldLines)
  genEndOfFile             (file)
  file.close()

  file = open(tablename+".h", "w")
  genHeaderFile(file, tablename, fieldLines)
  file.close()

