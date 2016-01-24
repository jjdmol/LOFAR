#!/bin/env python
import os
import re

def grep(string,list):
    expr = re.compile(string, re.MULTILINE)
    return [elem for elem in list if expr.search(open(elem).read())]

def lgrep(string,list):
    expr = re.compile(string, re.MULTILINE)
    return [ line for line in list if expr.search(line) ]

def createTable(file,tablename,fieldlist):
    print >>file, "-- table "+tablename+"Table"
    print >>file, "DROP TABLE "+tablename+"Table   CASCADE;"
    print >>file, "DROP SEQUENCE "+tablename+"ID;"
    print >>file
    print >>file, "CREATE SEQUENCE "+tablename+"ID;"
    print >>file
    print >>file, "CREATE TABLE "+tablename+"Table ("
    print >>file, "    recordID            INT4         NOT NULL DEFAULT nextval('"+tablename+"ID'),"
    print >>file, "    treeID              INT4         NOT NULL,"
    print >>file, "    nodeName            VARCHAR      NOT NULL,"
    print >>file, "    infoArray           VARCHAR[]    DEFAULT '{}',"
    print >>file, "    CONSTRAINT "+tablename+"_PK      PRIMARY KEY(recordID)"
    print >>file, ") WITHOUT OIDS;"
    print >>file, "CREATE INDEX "+tablename+"_treeid ON "+tablename+"Table (treeID);"
    print >>file

def createType(file,tablename,fieldlist):
    print >>file, "-- type "+tablename
    print >>file, "DROP TYPE "+tablename+" CASCADE;"
    print >>file, "CREATE TYPE "+tablename+" AS ("
    print >>file, "  recordID            INT4,"
    print >>file, "  treeID              INT4,"
    print >>file, "  nodename            VARCHAR,"
    print >>file, "  infoArray           VARCHAR[]"
    print >>file, ");"
    print >>file

def getRecord1(file,tablename):
    print >>file, "-- "+tablename+"GetRecord(recordID)"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"GetRecord(INTEGER)"
    print >>file, "RETURNS %s AS $$" % tablename
    print >>file, "  DECLARE"
    print >>file, "    vRecord  RECORD;"
    print >>file, "  BEGIN"
    print >>file, "    SELECT recordid,treeid,nodename,infoarray INTO vRecord"
    print >>file, "      FROM %sTable WHERE recordID = $1;" % tablename
    print >>file, "    RETURN vRecord;"
    print >>file, "  END"
    print >>file, "$$ language plpgsql;"
    print >>file

def getRecord2(file,tablename):
    print >>file, "-- "+tablename+"GetRecord(treeID, nodename)"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"GetRecord(INTEGER, VARCHAR)"
    print >>file, "RETURNS %s AS $$" % tablename
    print >>file, "  DECLARE"
    print >>file, "    vRecord  RECORD;"
    print >>file, "  BEGIN"
    print >>file, "    SELECT recordid,treeid,nodename,infoarray INTO vRecord"
    print >>file, "      FROM %sTable WHERE treeID=$1 AND nodename=$2;" % tablename
    print >>file, "    RETURN vRecord;"
    print >>file, "  END"
    print >>file, "$$ language plpgsql;"
    print >>file

def getRecords1(file,tablename):
    print >>file, "-- "+tablename+"GetRecords(treeID)"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"GetRecords(INTEGER)"
    print >>file, "RETURNS SETOF %s AS $$" % tablename
    print >>file, "  DECLARE"
    print >>file, "    vRecord  RECORD;"
    print >>file, "  BEGIN"
    print >>file, "    FOR vRecord IN SELECT recordid,treeid,nodename,infoarray"
    print >>file, "      FROM %sTable WHERE treeid = $1 ORDER BY recordid" % tablename
    print >>file, "    LOOP"
    print >>file, "      RETURN NEXT vRecord;"
    print >>file, "    END LOOP;"
    print >>file, "    RETURN;"
    print >>file, "  END"
    print >>file, "$$ language plpgsql;"
    print >>file

def getRecords2(file,tablename):
    print >>file, "-- "+tablename+"GetRecords(treeID, nodename)"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"GetRecords(INTEGER, VARCHAR)"
    print >>file, "RETURNS SETOF %s AS $$" % tablename
    print >>file, "  DECLARE"
    print >>file, "    vRecord  RECORD;"
    print >>file, "  BEGIN"
    print >>file, "    FOR vRecord IN SELECT recordid,treeid,nodename,infoarray"
    print >>file, "      FROM %sTable WHERE treeid=$1 AND nodename LIKE $2 ORDER BY recordid" % tablename
    print >>file, "    LOOP"
    print >>file, "      RETURN NEXT vRecord;"
    print >>file, "    END LOOP;"
    print >>file, "    RETURN;"
    print >>file, "  END"
    print >>file, "$$ language plpgsql;"
    print >>file

def getRecordsOnTreeList(file,tablename):
    print >>file, "-- "+tablename+"GetRecordsOnTreeList(treeID[])"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"GetRecordsOnTreeList(INTEGER[])"
    print >>file, "RETURNS SETOF %s AS $$" % tablename
    print >>file, "  DECLARE"
    print >>file, "    vRecord  RECORD;"
    print >>file, "    x        INTEGER;"
    print >>file, "  BEGIN"
    print >>file, "    FOREACH x in ARRAY $1"
    print >>file, "    LOOP"
    print >>file, "      FOR vRecord IN SELECT recordid,treeid,nodename,infoarray"
    print >>file, "          FROM %sTable WHERE treeid = x ORDER BY recordid" % tablename
    print >>file, "      LOOP"
    print >>file, "        RETURN NEXT vRecord;"
    print >>file, "      END LOOP;"
    print >>file, "    END LOOP;"
    print >>file, "    RETURN;"
    print >>file, "  END"
    print >>file, "$$ language plpgsql;"
    print >>file

def getRecordsOnRecordList(file,tablename):
    print >>file, "-- "+tablename+"GetRecordsOnRecordList(treeID[])"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"GetRecordsOnRecordList(INTEGER[])"
    print >>file, "RETURNS SETOF %s AS $$" % tablename
    print >>file, "  DECLARE"
    print >>file, "    vRecord  RECORD;"
    print >>file, "    x        INTEGER;"
    print >>file, "  BEGIN"
    print >>file, "    FOREACH x in ARRAY $1"
    print >>file, "    LOOP"
    print >>file, "      SELECT recordid,treeid,nodename,infoarray INTO vRecord"
    print >>file, "      FROM %sTable WHERE recordid = x;" % tablename
    print >>file, "      RETURN NEXT vRecord;"
    print >>file, "    END LOOP;"
    print >>file, "    RETURN;"
    print >>file, "  END"
    print >>file, "$$ language plpgsql;"
    print >>file

def getFields1(file,tablename):
    print >>file, "-- "+tablename+"GetFieldOnrecordList(fieldnr, recordNrs)"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"GetFieldOnRecordList(INTEGER, INTEGER[])"
    print >>file, "RETURNS SETOF VARCHAR AS $$"
    print >>file, "  DECLARE"
    print >>file, "    vResult  VARCHAR;"
    print >>file, "    recNr    INTEGER;"
    print >>file, "  BEGIN"
    print >>file, "    FOREACH recNr IN ARRAY $2"
    print >>file, "    LOOP"
    print >>file, "      SELECT infoarray[$1] INTO vResult FROM %sTable where recordID=recNr;" % tablename
    print >>file, "      RETURN NEXT vResult;"
    print >>file, "    END LOOP;"
    print >>file, "    RETURN;"
    print >>file, "  END"
    print >>file, "$$ language plpgsql;"
    print >>file

def getFields2(file,tablename):
    print >>file, "-- "+tablename+"GetFieldOnRecordList2(recordNrs, fieldnr)"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"GetFieldOnRecordList2(TEXT, INTEGER)"
    print >>file, "RETURNS SETOF VARCHAR AS $$"
    print >>file, "  DECLARE"
    print >>file, "    vResult  VARCHAR;"
    print >>file, "    recNr    INTEGER;"
    print >>file, "    vQuery   TEXT;"
    print >>file, "  BEGIN"
    print >>file, "    vQuery:='SELECT infoarray['||$1||'] FROM %sTable where recordID in ('||$2||')';" % tablename
    print >>file, "    FOR vResult in EXECUTE vQuery" 
    print >>file, "    LOOP"
    print >>file, "      RETURN NEXT vResult;"
    print >>file, "    END LOOP;"
    print >>file, "    RETURN;"
    print >>file, "  END"
    print >>file, "$$ language plpgsql;"
    print >>file

def saveRecord(file,tablename):
    print >>file, "-- "+tablename+"SaveRecord(auth, recordID, treeID, nodename, array)"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"SaveRecord(INTEGER, INTEGER, INTEGER, VARCHAR, VARCHAR[])"
    print >>file, "RETURNS BOOLEAN AS $$"
    print >>file, "  DECLARE"
    print >>file, "    vFunction    CONSTANT INT2 := 1;"
    print >>file, "    vIsAuth      BOOLEAN;"
    print >>file, "    vAuthToken   ALIAS FOR $1;"
    print >>file, "    vTreeID      %sTable.treeID%%TYPE;" % tablename
    print >>file, "    vRecordID    %sTable.recordID%%TYPE;" % tablename
    print >>file, "  BEGIN"
    checkAuthorisation(file, 3)
    checkTreeExistance(file, 3)
    print >>file, "    SELECT recordID INTO vRecordID from %sTable where recordID=$2;" % tablename
    print >>file, "    IF NOT FOUND THEN"
    print >>file, "      INSERT INTO %sTable (recordID,treeID,nodeName,infoArray) VALUES($2,$3,$4,$5);" % tablename
    print >>file, "    ELSE"
    print >>file, "      UPDATE %sTable set infoarray=$5 where recordID=$2;" % tablename
    print >>file, "    END IF;"
    print >>file, "    RETURN TRUE;"
    print >>file, "  END;"
    print >>file, "$$ language plpgsql;"
    print >>file

def saveField(file,tablename):
    print >>file, "-- "+tablename+"SaveField(auth, recordID, treeID, fieldIndex, stringValue)"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"SaveField(INTEGER, INTEGER, INTEGER, INTEGER, VARCHAR)"
    print >>file, "RETURNS BOOLEAN AS $$"
    print >>file, "  DECLARE"
    print >>file, "    vFunction    CONSTANT INT2 := 1;"
    print >>file, "    vIsAuth      BOOLEAN;"
    print >>file, "    vAuthToken   ALIAS FOR $1;"
    print >>file, "    vTreeID      %sTable.treeID%%TYPE;" % tablename
    print >>file, "    vRecordID    %sTable.recordID%%TYPE;" % tablename
    print >>file, "  BEGIN"
    checkAuthorisation(file, 3)
    checkTreeExistance(file, 3)
    print >>file, "    UPDATE %sTable set infoarray[$4]=$5 where recordID=$2;" % tablename
    print >>file, "    RETURN TRUE;"
    print >>file, "  END;"
    print >>file, "$$ language plpgsql;"
    print >>file

def saveFields(file,tablename):
    print >>file, "-- "+tablename+"SaveFields(auth, fieldIndex, recordID[], stringValue[])"
    print >>file, "CREATE OR REPLACE FUNCTION "+tablename+"SaveFields(INTEGER, INTEGER, INTEGER[], VARCHAR[])"
    print >>file, "RETURNS BOOLEAN AS $$"
    print >>file, "  DECLARE"
    print >>file, "    vFunction    CONSTANT INT2 := 1;"
    print >>file, "    vIsAuth      BOOLEAN;"
    print >>file, "    vAuthToken   ALIAS FOR $1;"
    print >>file, "    i            INTEGER;"
    print >>file, "    x            INTEGER;"
    print >>file, "  BEGIN"
    checkAuthorisation(file, 0)
    print >>file, "    i := 1;"
    print >>file, "    FOREACH x IN ARRAY $3"
    print >>file, "    LOOP"
    print >>file, "      UPDATE %sTable set infoarray[$2]=$4[i] where recordID=x;" % tablename
    print >>file, "      i := i + 1;"
    print >>file, "    END LOOP;"
    print >>file, "    RETURN TRUE;"
    print >>file, "  END;"
    print >>file, "$$ language plpgsql;"
    print >>file

def exportDefinition(file,tablename,fieldlist):
    print >>file, "-- export"+tablename+"Definition()"
    print >>file, "CREATE OR REPLACE FUNCTION export"+tablename+"Definition()"
    print >>file, "RETURNS TEXT AS $$"
    print >>file, "  DECLARE"
    print >>file, "    vResult  TEXT;"
    print >>file, "  BEGIN"
    print >>file, "    vResult:='"+tablename+"<recordID,treeID,nodename"+fieldnames(fieldlist)+">';"
    print >>file, "    RETURN vResult;"
    print >>file, "  END;"
    print >>file, "$$ language plpgsql IMMUTABLE;"
    print >>file

def exportRecord(file,tablename,fieldlist):
    print >>file, "-- export"+tablename+"(recordNr)"
    print >>file, "CREATE OR REPLACE FUNCTION export"+tablename+"(INT4)"
    print >>file, "RETURNS TEXT AS $$"
    print >>file, "  DECLARE"
    print >>file, "    vRec     RECORD;"
    print >>file, "    vResult  TEXT;"
    print >>file, "  BEGIN"
    print >>file, "    SELECT * INTO vRec FROM "+tablename+"Table WHERE recordID=$1;"
    print >>file, "    IF NOT FOUND THEN"
    print >>file, "      RAISE EXCEPTION E'"+tablename+" with recordnr \\'%\\' not found',$1;"
    print >>file, "    END IF;"
    line = "    vResult := '{treeID:' || text(vRec.treeID) || ',recordID:' || text(vRec.recordID) "
    count = 2
    for field in fieldlist:
      line += fieldAsText(count-1, field.split())
      count += 1
      if count %3 == 0:
        print >>file, line+";"
        line = "    vResult := vResult "
    line += "|| '}';"
    print >>file, line
    print >>file, "    RETURN vResult;"
    print >>file, "  END;"
    print >>file, "$$ language plpgsql;"
    print >>file

def fieldAndType(args):
    if args[3] in tInt:
      return args[1].ljust(30)+"INT4        "
    if args[3] in tUint:
      return args[1].ljust(30)+"INT4        "
    if args[3] in tFlt:
      return args[1].ljust(30)+"FLOAT       "
    if args[3] in tBool:
      return args[1].ljust(30)+"BOOLEAN     "
    if args[3] in tText:
      return args[1].ljust(30)+"VARCHAR     "
    return args[1].ljust(30)+"???         "

def fieldAndTypeAndDefault(args):
    if args[3] in tText:
      return fieldAndType(args)+" DEFAULT "+args[7]
    return fieldAndType(args)+" DEFAULT '"+args[7] + "'"

def fieldnames(fieldlist):
    result = ""
    for field in fieldlist:
      result += "," + field.split()[1]
    return result

def fieldAsText(indexNr, args):
    if args[3] in tText:
      return "|| ',%s:' || textValue(vRec.infoArray[%d])" % (args[1], indexNr)
    return "|| ',%s:' || vRec.infoArray[%d]" % (args[1], indexNr)

def checkAuthorisation(file, treeIDIdx):
    print >>file, "    -- check autorisation(authToken, tree, func, parameter)"
    print >>file, "    vIsAuth := FALSE;"
    if treeIDIdx:
      print >>file, "    SELECT isAuthorized(vAuthToken, $%d, vFunction, 0) INTO vIsAuth;" % treeIDIdx
    else:
      print >>file, "    SELECT isAuthorized(vAuthToken, 0, vFunction, 0) INTO vIsAuth;"
    print >>file, "    IF NOT vIsAuth THEN"
    print >>file, "      RAISE EXCEPTION 'Not authorized';"
    print >>file, "    END IF;"
    print >>file

def checkTreeExistance(file, treeIDIdx):
    print >>file, "    -- check tree existance"
    print >>file, "    SELECT treeID INTO vTreeID FROM OTDBtree WHERE treeID=$%d;" % treeIDIdx
    print >>file, "    IF NOT FOUND THEN"
    print >>file, "      RAISE EXCEPTION 'Tree %% does not exist', $%d;" % treeIDIdx
    print >>file, "    END IF;"
    print >>file

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

  file = open("create_"+tablename+".sql", "w")
  createTable           (file, tablename, fieldLines)
  createType            (file, tablename, fieldLines)
  exportDefinition      (file, tablename, fieldLines)
  exportRecord          (file, tablename, fieldLines)
  getRecord1            (file, tablename)
  getRecord2            (file, tablename)
  getRecords1           (file, tablename)
  getRecords2           (file, tablename)
  getRecordsOnTreeList  (file, tablename)
  getRecordsOnRecordList(file, tablename)
  getFields1            (file, tablename)
  getFields2            (file, tablename)
  saveRecord            (file, tablename)
  saveField             (file, tablename)
  saveFields            (file, tablename)
  file.close()

