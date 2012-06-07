#!/bin/env python
import os
import re

def grep(string,list):
    expr = re.compile(string, re.MULTILINE)
    return [elem for elem in list if expr.search(open(elem).read())]

def lgrep(string,list):
    expr = re.compile(string, re.MULTILINE)
    return [ line for line in list if expr.search(line) ]

def genData(file, className, fieldList):
    print >>file, "  // genDataString - helper function"
    print >>file, "  private String genDataString()"
    print >>file, "  {"
    print >>file, '    String result = "";'
    print >>file, '    String charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";'
    print >>file, "    int    nrChars = charset.length();"
    print >>file, "    Random rand = new Random();"
    print >>file, '    String field;'
    idx = 0
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, '    field = "";'
        print >>file, "    for(int i=0; i<15;i++) { field += charset.charAt(rand.nextInt(nrChars)); } result += field; // %s" % args[1]
      if args[3] in tInt:
        print >>file, "    result += Integer.toString(rand.nextInt()%%2 == 1 ? rand.nextInt() : -rand.nextInt()); // %s" % args[1]
      if args[3] in tLong:
        print >>file, "    result += Long.toString(rand.nextInt()%%2 == 1 ? rand.nextInt() : -rand.nextInt()); // %s" % args[1]
      if args[3] in tUint:
        print >>file, "    result += Integer.toString(rand.nextInt()); // %s" % args[1]
      if args[3] in tULng:
        print >>file, "    result += Long.toString(rand.nextInt()); // %s" % args[1]
      if args[3] in tBool:
        print >>file, '    result += (rand.nextInt()%%2 == 1 ? "true" : "false"); // %s' % args[1]
      if args[3] in tFlt:
        print >>file, "    result += Float.toString((rand.nextFloat() %% 100000) * (float)3.1415926); // %s" % args[1]
      if args[3] in tDbl:
        print >>file, "    result += Double.toString(rand.nextDouble() %% 100000 * 3.1415926); // %s" % args[1]
      idx += 1
      if idx < len(fieldList):
        print >>file, '    result += ",";'
    print >>file, "    return (result);"
    print >>file, "  }"
    print >>file

def genHeader(file,className,fieldList):
    print >>file, "import java.util.Random;"
    print >>file, "import java.util.Vector;"
    print >>file, "import nl.astron.lofar.sas.otb.jotdb3.jInitCPPLogger;"
    print >>file, "import nl.astron.lofar.sas.otb.jotdb3.jOTDBconnection;"
    print >>file, "import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;"
    print >>file, "import nl.astron.lofar.sas.otb.jotdb3.jRecordAccess;"
    print >>file, "import nl.astron.lofar.sas.otb.jotdb3.j%s;" % className
    print >>file
    print >>file, "class t%s" % className
    print >>file, "{"
    print >>file, '  private String ext = "11501_coolen";'
    print >>file, '  static { System.loadLibrary("jotdb3"); }'
    print >>file
    genData(file, className, fieldList)
    print >>file, "  public static void main (String[] args)"
    print >>file, "  {"
    print >>file, "    // setup logging"
    print >>file, '    String logConfig = "t%s.log_prop";' % className
    print >>file, '    try {'
    print >>file, '      jInitCPPLogger aCPPLogger= new jInitCPPLogger(logConfig);'
    print >>file, '    }'
    print >>file, '    catch (Exception ex) {'
    print >>file, '      System.out.println("Error: "+ ex);'
    print >>file, '    }'
    print >>file, '    // Create test object and do tests'
    print >>file, '    t%s tObject = new t%s();' % (className, className)
    print >>file, '    tObject.test ();'
    print >>file, '  }'
    print >>file
    print >>file, '  public void test() {'
    print >>file, '    Random rand = new Random(6863655);'
    print >>file, '    try {'
    print >>file, '      // do the test'
    print >>file, '      System.out.println ("Starting... ");'
    print >>file
    print >>file, '      // create a jOTDBconnection'
    print >>file, '      jOTDBconnection conn = new jOTDBconnection("paulus", "boskabouter", "ArrayTest", "localhost",ext);'
    print >>file, '      assert conn!=null : "Can\'t allocated a connection object to database \'ArrayTest\'";'
    print >>file, '      assert conn.connect() : "Connect failed";'
    print >>file, '      assert conn.isConnected() : "Connection failed";'
    print >>file, '      System.out.println ("Connection succesful!");'
    print >>file
    print >>file, '      System.out.println ("Trying to construct a RecordAccess object");'
    print >>file, '      jRecordAccess itsRecordAccess  = new jRecordAccess(ext);'
    print >>file, '      assert itsRecordAccess!=null : "Creation of recordAccess Failed!";'
    print >>file


def genConstructor(file, className, fieldList):
    print >>file, "      // Test Constructors"
    print >>file, '      System.out.println ("Testing Constructors");'
    print >>file, "      j%s    object1 = new j%s();" % (className, className)
    print >>file, '      assert object1!=null : "Creation of default %s Failed!";' % className
    print >>file, '      System.out.println ("Default constructed object:" + object1.print());'
    print >>file
    print >>file, "      String contents = genDataString();"
    print >>file, '      j%s    object2 = new j%s(25, 625, "theNameOfTheNode", contents);' % (className, className)
    print >>file, '      System.out.println ("%s2:" + object2.print());' % className
    print >>file
    print >>file, '      assert object2.treeID()   == 25 : "treeID not 25";'
    print >>file, '      assert object2.recordID() == 625 : "recordID not 625";'
    print >>file, '      assert object2.nodeName().equals("theNameOfTheNode") : "nodename not \'theNameOfTheNode\'";'
    print >>file
    print >>file, '      String[]   fields = contents.split(",");'
    idx = 0
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, "      assert object2.%s.equals(fields[%d]);" % (args[1], idx)
      if args[3] in tInt:
        print >>file, "      assert object2.%s == Integer.valueOf(fields[%d]);" % (args[1], idx)
      if args[3] in tUint:
        print >>file, "      assert object2.%s == Integer.valueOf(fields[%d]);" % (args[1], idx)
      if args[3] in tLong:
        print >>file, "      assert object2.%s == Long.valueOf(fields[%d]);" % (args[1], idx)
      if args[3] in tULng:
        print >>file, "      assert object2.%s == Long.valueOf(fields[%d]);" % (args[1], idx)
      if args[3] in tBool:
        print >>file, "      assert object2.%s == Boolean.valueOf(fields[%d]);" % (args[1], idx)
      if args[3] in tFlt:
        print >>file, "      assert object2.%s == Float.valueOf(fields[%d]);" % (args[1], idx)
      if args[3] in tDbl:
        print >>file, "      assert object2.%s == Double.valueOf(fields[%d]);" % (args[1], idx)
      idx += 1
    print >>file

def genSaveRecords(file,className):
    print >>file, "      // fill database for tree 25 and 61"
    print >>file, '      System.out.println ("Testing save() by adding records for tree 25 and 61");'
    print >>file, '      Vector<jOTDBtree> treeList = conn.getTreeList ((short)20);'
    print >>file, '      assert treeList.size() >= 2 : "need at least 2 available template trees ";'
    print >>file, '      int firstTree  = 25;'
    print >>file, '      int secondTree = 61;'
    print >>file
    print >>file, "      String  mask;"
    print >>file, "      Vector<j%s>   origRecs = new Vector();" % className
    print >>file, "      for (int i = 0; i < 32; i++) {" 
    print >>file, '        if (((i % 16)/ 8) == 1) mask="secondHalf_"; '
    print >>file, '        else mask="firstHalf_";'
    print >>file, "        j%s aRec = new j%s(firstTree+(i/16)*36, i+1, mask+i, genDataString());" %(className, className)
    print >>file, "        boolean add = origRecs.add(aRec);"
    print >>file, "      }"
    print >>file, "      for (j%s aRec: origRecs) {" % className
    print >>file, "        itsRecordAccess.save%s(aRec);" % className
    print >>file, "      }"
    print >>file

def genGetRecords(file,className,fieldList):
    print >>file, "      // getRecords(connection, treeID)"
    print >>file, '      System.out.println("Testing getRecords(connection, treeID)");'
    print >>file, "      Vector<j%s> container = itsRecordAccess.get%ss(firstTree);" % (className, className)
    print >>file, '      assert container.size() == 16 : "wrong number of records returned for tree 25";'
    print >>file, "      container = itsRecordAccess.get%ss(333);" % className
    print >>file, '      assert container.size() == 0 : "Didn\'t expect to find records for tree 333";'
    print >>file
    print >>file, "      // getRecords(connection, treeID, nodename)"
    print >>file, '      System.out.println("Testing getRecords(connection, treeID, nodeName)");'
    print >>file, '      container = itsRecordAccess.get%ss(25, "firstHalf%%");' % className
    print >>file, '      assert container.size() == 8 : container.size() + " records returned";'
    print >>file, '      container = itsRecordAccess.get%ss(333, "secondHalf_10");' % className
    print >>file, '      assert container.size() == 0 : container.size() + " records returned";'
    print >>file, '      container = itsRecordAccess.get%ss(25, "secondHalf_10");' % className
    print >>file, '      assert container.size() == 1 : container.size() + " records returned";'
    print >>file
    print >>file, "      // getRecord(connection, recordID)"
    print >>file, '      System.out.println("Testing getRecord(connection, recordID)");'
    print >>file, "      j%s record = itsRecordAccess.get%s(container.elementAt(0).recordID());" % (className, className)
    print >>file, '      assert container.elementAt(0).equals(record) : "Record differs";'
    print >>file
    print >>file, "      // getRecord(connection, treeID, nodename)"
    print >>file, '      System.out.println("Testing getRecord(connection, treeID, nodename)");'
    print >>file, "      j%s record2 = itsRecordAccess.get%s(container.elementAt(0).treeID(), container.elementAt(0).nodeName());" % (className, className)
    print >>file, '      assert record.equals(record2) : "Records are different";'
    print >>file
    print >>file, "      // getRecordsOnTreeList(connection, vector<treeid>)"
    print >>file, '      System.out.println("Testing getRecordsOnTreeList(connection, vector<treeID>)");'
    print >>file, "      Vector<Integer>  treeIDs = new Vector();"
    print >>file, "      treeIDs.add(25);"
    print >>file, "      treeIDs.add(61);"
    print >>file, "      container = itsRecordAccess.get%ssOnTreeList(treeIDs);" % className
    print >>file, '      assert container.size() == 32 : "Expected 32 records in the result";'
    print >>file, "      // All the saved records are in the container now, compare them with the original ones."
    print >>file, "      for (int i = 0; i < 32; i++) {"
    print >>file, '        assert container.elementAt(i).equals(origRecs.elementAt(i)) : "Element at "+i+" differs from original";'
    print >>file, "        ((j%s)container.elementAt(i)).print();" % className
    print >>file, "      }"
    print >>file
    print >>file, "      // getRecordsOnRecordList(connection, vector<RecordID>)"
    print >>file, '      System.out.println("Testing getRecordsOnRecordList(connection, vector<recordID>)");'
    print >>file, "      Vector<Integer>  recordIDs = new Vector();"
    print >>file, "      recordIDs.add(container.elementAt(4).recordID());"
    print >>file, "      recordIDs.add(container.elementAt(14).recordID());"
    print >>file, "      recordIDs.add(container.elementAt(24).recordID());"
    print >>file, "      recordIDs.add(container.elementAt(17).recordID());"
    print >>file, "      Vector<j%s> smallContainer = itsRecordAccess.get%ssOnRecordList(recordIDs);" % (className, className)
    print >>file, '      assert smallContainer.size() == 4 : "expected 4 records in the small container";'
    print >>file
    print >>file, "      // getFieldOnRecordList(connection, fieldname, vector<RecordID>)"
    fieldname = fieldList[len(fieldList)-2].split()[1]
    fieldtype = fieldList[len(fieldList)-2].split()[3]
    if fieldtype in tText:
        converter = ""
    if fieldtype in tInt:
        converter = "Integer.toString"
    if fieldtype in tUint:
        converter = "Integer.toString"
    if fieldtype in tLong:
        converter = "Long.toString"
    if fieldtype in tULng:
        converter = "Long.toString"
    if fieldtype in tBool:
        converter = "Boolean.toString"
    if fieldtype in tFlt:
        converter = "Float.toString"
    if fieldtype in tDbl:
        converter = "Double.toString"
    print >>file, '      System.out.println("Testing getFieldOnRecordList(connection, \'%s\', vector<recordID>)");' % fieldname
    print >>file, '      Vector<String> fields2 = itsRecordAccess.get%sFieldOnRecordList("%s", recordIDs);' % (className, fieldname)
    print >>file, '      assert fields2.size() == 4 : "4 records expected";'
    print >>file, '      assert fields2.elementAt(0).equals(%s(container.elementAt(4).%s)) : fields2.elementAt(0)+" ? " + %s(container.elementAt(4).%s);' % (converter, fieldname, converter, fieldname)
    print >>file, '      assert fields2.elementAt(1).equals(%s(container.elementAt(14).%s)) : fields2.elementAt(1)+" ? " + %s(container.elementAt(14).%s);' % (converter, fieldname, converter, fieldname)
    print >>file, '      assert fields2.elementAt(2).equals(%s(container.elementAt(24).%s)) : fields2.elementAt(2)+" ? " + %s(container.elementAt(24).%s);' % (converter, fieldname, converter, fieldname)
    print >>file, '      assert fields2.elementAt(3).equals(%s(container.elementAt(17).%s)) : fields2.elementAt(3)+" ? " + %s(container.elementAt(17).%s);' % (converter, fieldname, converter, fieldname)
    print >>file


def genSaveField(file,className,fieldList):
    print >>file, "      // saveField(connection, fieldIndex)"
    print >>file, '      System.out.println("Testing saveField(connection, fieldIndex)");'
    args = fieldList[1].split()
    if args[3] in tText:
      print >>file, '      String    newValue = "";'
      print >>file, '      String charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";'
      print >>file, "      int    nrChars = charset.length();"
      print >>file, "      for(int i=0; i<15; i++) { newValue += charset.charAt(rand.nextInt(nrChars)); }"
      print >>file, "      container.elementAt(13).%s = newValue;" % args[1]
    if args[3] in tInt + tLong:
      print >>file, "      container.elementAt(13).%s = (rand.nextInt()%%2 == 1 ? rand.nextInt() : -rand.nextInt());" % args[1]
    if args[3] in tUint + tULng:
      print >>file, "      container.elementAt(13).%s = rand.nextInt();" % args[1]
    if args[3] in tBool:
      print >>file, '      container.elementAt(13).%s = (rand.nextInt()%%2 == 1 ? "true" : "false");' % args[1]
    if args[3] in tFlt:
      print >>file, "      container.elementAt(13).%s = (rand.nextFloat() % 100000 * 3.1415926);" % args[1]
    if args[3] in tDbl:
      print >>file, "      container.elementAt(13).%s = (rand.nextDouble() % 100000 * 3.1415926);" % args[1]
    print >>file, '      assert itsRecordAccess.save%sField(container.elementAt(13), 1) : "Saving %s failed";' % (className, className)
    print >>file, "      j%s record13 = itsRecordAccess.get%s(container.elementAt(13).recordID());" % (className, className)
    print >>file, "      assert container.elementAt(13).equals(record13);"
    print >>file

def genSaveFields(file,className,fieldList):
    print >>file, "      // saveFields(connection, fieldIndex, vector<%s>)" % className
    print >>file, '      System.out.println("Testing saveFields(connection, fieldIndex, vector<%s>)");' % className
    print >>file, '      for (j%s aRec: smallContainer) {' % className
    args = fieldList[0].split()
    if args[3] in tText:
      print >>file, '        aRec.%s="";' % args[1]
      print >>file, "        for(int c=0; c<15; c++) { aRec.%s += charset.charAt(rand.nextInt(nrChars)); }" % args[1]
    if args[3] in tInt + tLong:
      print >>file, "        aRec.%s = (rand.nextInt()%%2 == 1 ? rand.nextInt() : -rand.nextInt());" % args[1]
    if args[3] in tUint + tULng:
      print >>file, "        aRec.%s = rand.nextInt();" % args[1]
    if args[3] in tBool:
      print >>file, '        aRec.%s = Boolean.valueOf(rand.nextInt()%%2 == 1 ? "true" : "false");' % args[1]
    if args[3] in tFlt:
      print >>file, "        aRec.%s = (rand.nextFloat() % 100000 * 3.1415926);" % args[1]
    if args[3] in tDbl:
      print >>file, "        aRec.%s = (rand.nextDouble() % 100000 * 3.1415926);" % args[1]
    print >>file, '      }'
    print >>file, '      assert itsRecordAccess.save%sFields(0, smallContainer) : "Saving %s fieldsvector failed";' % (className, className)
    print >>file, "      Vector<j%s> smallContainer2 = itsRecordAccess.get%ssOnRecordList(recordIDs);" % (className, className)
    print >>file, '      assert smallContainer2.size() == smallContainer.size() : "smallContainers differ in size";'
    print >>file, "      for (int i = 0; i < smallContainer.size(); i++) {"
    print >>file, '        assert smallContainer.elementAt(i).equals(smallContainer2.elementAt(i)) : "smallContainer " + i + " not equal to smallContainer2 "+ i;'
    print >>file, "      }"

def genEndOfFile(file):
    print >>file
    print >>file, '      System.out.println("ALL TESTS PASSED SUCCESSFUL");'
    print >>file, "      System.exit(0);"
    print >>file, "    } catch (Exception ex) {"
    print >>file, '      System.out.println("Error: "+ ex);'
    print >>file, "      ex.printStackTrace();"
    print >>file, "    }"
    print >>file, "  }"
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
tInt  = ["int",  "vint",  "pint"]
tUint = ["uint", "vuint", "puint"]
tLong = ["long", "vlong", "plong" ]
tULng = ["ulng", "vulng", "pulng" ]
tDate = ["time", "date", "vtime", "vdate", "ptime", "pdate"]
tFlt  = ["flt",  "vflt",  "pflt"]
tDbl  = ["dbl",  "vdbl",  "pdbl" ]

compfiles = [cf for cf in os.listdir('.') if cf.endswith(".comp")]
DBfiles = grep("^table.",compfiles)
for DBfile in DBfiles:
  tablename = lgrep("^table", open(DBfile).readlines())[0].split()[1]
  print "tablename="+tablename
  fieldLines = lgrep("^field", open(DBfile).readlines())

  file = open("t"+tablename+".java", "w")
  genHeader        (file, tablename, fieldLines)
  genConstructor   (file, tablename, fieldLines)
  genSaveRecords   (file, tablename)
  genGetRecords    (file, tablename, fieldLines)
  genSaveField     (file, tablename, fieldLines)
  genSaveFields    (file, tablename, fieldLines)
  genEndOfFile             (file)
  file.close()
