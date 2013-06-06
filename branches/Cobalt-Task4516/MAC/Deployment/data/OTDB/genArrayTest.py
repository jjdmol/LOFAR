#!/bin/env python
import os
import re

def grep(string,list):
    expr = re.compile(string, re.MULTILINE)
    return [elem for elem in list if expr.search(open(elem).read())]

def lgrep(string,list):
    expr = re.compile(string, re.MULTILINE)
    return [ line for line in list if expr.search(line) ]

def genHeader(file,className,fieldList):
#    print >>file, "#include <pqxx/pqxx>"
    print >>file, "#include <lofar_config.h>"
    print >>file, "#include <Common/LofarLogger.h>"
    print >>file, "#include <Common/StringUtil.h>"
    print >>file, "#include <Common/StreamUtil.h>"
    print >>file, "#include <OTDB/OTDBconnection.h>"
    print >>file, '#include "%s.h"' % className
    print >>file
    print >>file, "using namespace pqxx;"
    print >>file, "using namespace LOFAR;"
    print >>file, "using namespace StringUtil;"
    print >>file, "using namespace OTDB;"
    print >>file
    genData(file, className, fieldList)
    print >>file, "int main() {"
    print >>file, "  srand(6863655);"
    print >>file
    print >>file, '  OTDBconnection*	otdbConn = new OTDBconnection("paulus", "boskabouter", "ArrayTest", "localhost");'
    print >>file, '  ASSERTSTR(otdbConn, "Can\'t allocated a connection object to database \'ArrayTest\'");'
    print >>file, '  ASSERTSTR(otdbConn->connect(), "Connect failed");'
    print >>file, '  ASSERTSTR(otdbConn->isConnected(), "Connection failed");'
    print >>file

def genData(file, className, fieldList):
    print >>file, "// genDataString - helper function"
    print >>file, "string genDataString()"
    print >>file, "{"
    print >>file, "  string result;"
    print >>file, '  string charset("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");'
    print >>file, "  int    nrChars(charset.length());"
    print >>file, "  string field;"
    print >>file, "  field.resize(15);"
    idx = 0
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, "  for(int i=0; i<15;i++) { field[i]=charset[rand()%%nrChars]; }; result += field; // %s" % args[1]
      if args[3] in tInt:
        print >>file, "  result += toString(rand()%%2 ? rand() : -rand()); // %s" % args[1]
      if args[3] in tUint:
        print >>file, "  result += toString(rand()); // %s" % args[1]
      if args[3] in tBool:
        print >>file, '  result += (rand()%%2 ? "true" : "false"); // %s' % args[1]
      if args[3] in tFlt:
        print >>file, "  result += toString(rand() %% 100000 * 3.1415926); // %s" % args[1]
      idx += 1
      if idx < len(fieldList):
        print >>file, '  result.append(",");'
    print >>file, "  return (result);"
    print >>file, "}"
    print >>file


def genConstructor(file, className, fieldList):
    print >>file, "  // Test Constructors"
    print >>file, '  cout << "Testing Constructors" << endl;'
    print >>file, "  %s    object1;" % className
    print >>file, '  cout << "Default constructed object:" << object1 << endl;'
    print >>file
    print >>file, "  string contents(genDataString());"
    print >>file, '  %s    object2(25, 625, "theNameOfTheNode", contents);' % className
    print >>file, '  cout << object2 << endl;'
    print >>file, "  ASSERT(object2.treeID()   == 25);"
    print >>file, "  ASSERT(object2.recordID() == 625);"
    print >>file, '  ASSERT(object2.nodeName() == "theNameOfTheNode");'
    print >>file, "  vector<string>   fields(split(contents, ','));"
    idx = 0
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, "  ASSERT(object2.%s == fields[%d]);" % (args[1], idx)
      if args[3] in tInt:
        print >>file, "  ASSERT(object2.%s == StringToInt32(fields[%d]));" % (args[1], idx)
      if args[3] in tUint:
        print >>file, "  ASSERT(object2.%s == StringToUint32(fields[%d]));" % (args[1], idx)
      if args[3] in tBool:
        print >>file, "  ASSERT(object2.%s == StringToBool(fields[%d]));" % (args[1], idx)
      if args[3] in tFlt:
        print >>file, "  ASSERT(object2.%s == StringToFloat(fields[%d]));" % (args[1], idx)
      idx += 1
    print >>file

def genGetRecords(file,className,fieldList):
    print >>file, "  // getRecords(connection, treeID)"
    print >>file, '  cout << "Testing getRecords(connection, treeID)" << endl;'
    print >>file, "  vector<%s> container(%s::getRecords(otdbConn, 25));" % (className, className)
    print >>file, "  ASSERT(container.size() == 16);"
    print >>file, "  container = %s::getRecords(otdbConn, 333);" % className
    print >>file, "  ASSERT(container.size() == 0);"
    print >>file
    print >>file, "  // getRecords(connection, treeID, nodename)"
    print >>file, '  cout << "Testing getRecords(connection, treeID, nodeName)" << endl;'
    print >>file, '  container = %s::getRecords(otdbConn, 25, "firstHalf%%");' % className
    print >>file, '  ASSERTSTR(container.size() == 8, container.size() << " records returned");'
    print >>file, '  container = %s::getRecords(otdbConn, 333, "secondHalf_10");' % className
    print >>file, '  ASSERTSTR(container.size() == 0, container.size() << " records returned");'
    print >>file, '  container = %s::getRecords(otdbConn, 25, "secondHalf_10");' % className
    print >>file, '  ASSERTSTR(container.size() == 1, container.size() << " records returned");'
    print >>file
    print >>file, "  // getRecord(connection, recordID)"
    print >>file, '  cout << "Testing getRecord(connection, recordID)" << endl;'
    print >>file, "  %s record(%s::getRecord(otdbConn, container[0].recordID()));" % (className, className)
    print >>file, "  ASSERT(container[0] == record);"
    print >>file
    print >>file, "  // getRecord(connection, treeID, nodename)"
    print >>file, '  cout << "Testing getRecord(connection, treeID, nodename)" << endl;'
    print >>file, "  %s record2(%s::getRecord(otdbConn, container[0].treeID(), container[0].nodeName()));" % (className, className)
    print >>file, "  ASSERT(record == record2);"
    print >>file
    print >>file, "  // getRecordsOnTreeList(connection, vector<treeid>)"
    print >>file, '  cout << "Testing getRecordsOnTreeList(connection, vector<treeID>)" << endl;'
    print >>file, "  vector<uint>  treeIDs;"
    print >>file, "  treeIDs.push_back(25);"
    print >>file, "  treeIDs.push_back(61);"
    print >>file, "  container = %s::getRecordsOnTreeList(otdbConn, treeIDs);" % className
    print >>file, "  ASSERT(container.size() == 32);"
    print >>file, "  // All the saved records are in the container now, compare them with the original ones."
    print >>file, "  for (uint i = 0; i < 32; i++) {"
    print >>file, "    ASSERT(container[i] == origRecs[i]);"
    print >>file, "  }"
    print >>file
    print >>file, "  // getRecordsOnRecordList(connection, vector<RecordID>)"
    print >>file, '  cout << "Testing getRecordsOnRecordList(connection, vector<recordID>)" << endl;'
    print >>file, "  vector<uint>  recordIDs;"
    print >>file, "  recordIDs.push_back(container[4].recordID());"
    print >>file, "  recordIDs.push_back(container[14].recordID());"
    print >>file, "  recordIDs.push_back(container[24].recordID());"
    print >>file, "  recordIDs.push_back(container[17].recordID());"
    print >>file, "  vector<%s> smallContainer = %s::getRecordsOnRecordList(otdbConn, recordIDs);" % (className, className)
    print >>file, "  ASSERT(smallContainer.size() == 4);"
    print >>file
    print >>file, "  // getFieldOnRecordList(connection, fieldname, vector<RecordID>)"
    fieldname = fieldList[5].split()[1]
    print >>file, '  cout << "Testing getFieldOnRecordList(connection, \'%s\', vector<recordID>)" << endl;' % fieldname
    print >>file, "  fields.clear();"
    print >>file, '  fields = %s::getFieldOnRecordList(otdbConn, "%s", recordIDs);' % (className, fieldname)
    print >>file, "  ASSERT(fields.size() == 4);"
    print >>file, '  ASSERTSTR(fields[0] == toString(container[4].%s), fields[0] << " ? " << toString(container[4].%s));' % (fieldname, fieldname)
    print >>file, '  ASSERTSTR(fields[1] == toString(container[14].%s), fields[1] << " ? " << toString(container[14].%s));' % (fieldname, fieldname)
    print >>file, '  ASSERTSTR(fields[2] == toString(container[24].%s), fields[2] << " ? " << toString(container[24].%s));' % (fieldname, fieldname)
    print >>file, '  ASSERTSTR(fields[3] == toString(container[17].%s), fields[3] << " ? " << toString(container[17].%s));' % (fieldname, fieldname)
    print >>file


def genSaveRecords(file,className):
    print >>file, "  // fill database for tree 25 and 61"
    print >>file, '  cout << "Testing save() by adding records for tree 25 and 61" << endl;'
    print >>file, "  // First make sure that these trees exist in the database"
    print >>file, "  try {"
    print >>file, '    work    xAction(*(otdbConn->getConn()), "newTree");'
    print >>file, '    result  res(xAction.exec("insert into OTDBtree (treeID,originid,momID,classif,treetype,state,creator) values (25,1,0,3,20,300,1);"));'
    print >>file, "    xAction.commit();"
    print >>file, "  } catch (...) {};"
    print >>file, "  try {"
    print >>file, '    work    xAction(*(otdbConn->getConn()), "newTree");'
    print >>file, '    result  res(xAction.exec("insert into OTDBtree (treeID,originid,momID,classif,treetype,state,creator) values (61,1,0,3,20,300,1);"));'
    print >>file, "    xAction.commit();"
    print >>file, "  } catch (...) {};"
    print >>file, "  string  mask;"
    print >>file, "  vector<%s>   origRecs;" % className
    print >>file, "  for (int i = 0; i < 32; i++) {" 
    print >>file, '    if ((i % 16)/ 8) mask="secondHalf_%d"; '
    print >>file, '    else mask="firstHalf_%d";'
    print >>file, "    origRecs.push_back(%s(25+(i/16)*36, i+1, formatString(mask.c_str(), i), genDataString()));" % className
    print >>file, "  }"
    print >>file, "  for (int i = 0; i < 32; i++) {" 
    print >>file, "    origRecs[i].save(otdbConn);"
    print >>file, "  }"
    print >>file

def genSaveField(file,className,fieldList):
    print >>file, "  // saveField(connection, fieldIndex)"
    print >>file, '  cout << "Testing saveField(connection, fieldIndex)" << endl;'
    print >>file, '  string    newValue;'
    args = fieldList[1].split()
    if args[3] in tText:
      print >>file, "  newValue.resize(15);"
      print >>file, '  string charset("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");'
      print >>file, "  int    nrChars(charset.length());"
      print >>file, "  for(int i=0; i<15; i++) { newValue[i]=charset[rand()%nrChars]; };"
    if args[3] in tInt:
      print >>file, "  newValue = toString(rand()%2 ? rand() : -rand());"
    if args[3] in tUint:
      print >>file, "  newValue = toString(rand());"
    if args[3] in tBool:
      print >>file, '  newValue = (rand()%2 ? "true" : "false");'
    if args[3] in tFlt:
      print >>file, "  newValue = toString(rand() % 100000 * 3.1415926);"
    print >>file, "  container[13].%s = newValue;" % args[1]
    print >>file, "  ASSERT(container[13].saveField(otdbConn, 1));"
    print >>file, "  %s record13(%s::getRecord(otdbConn, container[13].recordID()));" % (className, className)
    print >>file, "  ASSERT(container[13] == record13);"
    print >>file

def genSaveFields(file,className,fieldList):
    print >>file, "  // saveFields(connection, fieldIndex, vector<%s>)" % className
    print >>file, '  cout << "Testing saveFields(connection, fieldIndex, vector<%s>)" << endl;' % className
    print >>file, '  vector<%s>::iterator    iter = smallContainer.begin();' % className
    print >>file, '  vector<%s>::iterator    end  = smallContainer.end();' % className
    print >>file, '  while(iter != end) {'
    args = fieldList[0].split()
    if args[3] in tText:
      print >>file, "  iter->%s.resize(15);" % args[1]
      print >>file, '  string charset("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");'
      print >>file, "  int    nrChars(charset.length());"
      print >>file, "  for(int c=0; c<15; c++) { iter->%s[c]=charset[rand()%%nrChars]; };" % args[1]
    if args[3] in tInt:
      print >>file, "  iter->%s = toString(rand()%2 ? rand() : -rand());" % args[1]
    if args[3] in tUint:
      print >>file, "  iter->%s = toString(rand());" % args[1]
    if args[3] in tBool:
      print >>file, '  iter->%s = (rand()%2 ? "true" : "false");' % args[1]
    if args[3] in tFlt:
      print >>file, "  iter->%s = toString(rand() % 100000 * 3.1415926);" % args[1]
    print >>file, '    iter++;'
    print >>file, '  }'
    print >>file, "  ASSERT(%s::saveFields(otdbConn, 0, smallContainer));" % className
    print >>file, "  vector<%s> smallContainer2 = %s::getRecordsOnRecordList(otdbConn, recordIDs);" % (className, className)
    print >>file, "  ASSERT(smallContainer2.size() == smallContainer.size());"
    print >>file, "  for (uint i = 0; i < smallContainer.size(); i++) {"
    print >>file, "    ASSERT(smallContainer[i] == smallContainer2[i]);"
    print >>file, "  }"


def nbnbnbnb():
    print >>file, "	 string	recordNrs;"
    print >>file, "	 string	fieldValues;"
    print >>file, "  size_t	nrRecs = records.size();"
    print >>file, "	 recordNrs.reserve(nrRecs*5);    // speed up things a little"
    print >>file, "	 fieldValues.reserve(nrRecs*30);"
    print >>file, "  for (uint i = 0; i < nrRecs; i++) {"
    print >>file, "    recordNrs.append(toString(records[i].recordID()));"
    print >>file, "    fieldValues.append(records[i].fieldValue(fieldIndex));"
    print >>file, "    if (i < nrRecs-1) {"
    print >>file, '      recordNrs.append(",");'
    print >>file, '      fieldValues.append(",");'
    print >>file, "    }"
    print >>file, "  }"
    print >>file
    print >>file, '  string  command(formatString("SELECT * from %sSaveFields(%%d, %%d, \'{%%s}\', \'{%%s}\')", conn->getAuthToken(), fieldIndex, recordNrs.c_str(), fieldValues.c_str()));' % className
    print >>file, '  work    xAction(*(conn->getConn()), "saveFields%s");' % className
    print >>file, "  result  res(xAction.exec(command));"
    print >>file, "  bool    updateOK(false);"
    print >>file, '  res[0]["%ssaverecord"].to(updateOK);' % className
    print >>file, "  if (updateOK) {"
    print >>file, "    xAction.commit();"
    print >>file, "  }"
    print >>file, "  return(updateOK);"
    print >>file, "}"
    print >>file

def genEndOfFile(file):
    print >>file
    print >>file, '  cout << "ALL TESTS PASSED SUCCESSFUL" << endl;'
    print >>file, "  return(1);"
    print >>file, "}"
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

  file = open("t"+tablename+".cc", "w")
  genHeader        (file, tablename, fieldLines)
  genConstructor   (file, tablename, fieldLines)
  genSaveRecords   (file, tablename)
  genGetRecords    (file, tablename, fieldLines)
  genSaveField     (file, tablename, fieldLines)
  genSaveFields    (file, tablename, fieldLines)
  genEndOfFile             (file)
  file.close()

