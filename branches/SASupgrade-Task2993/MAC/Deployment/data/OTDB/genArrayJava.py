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
    print >>file, "package nl.astron.lofar.sas.otb.jotdb3;"
    print >>file
    print >>file, "public class j%s implements java.io.Serializable {" % className
    print >>file, '  private String itsName = "";'
    print >>file

def genConstructor(file, className, fieldList):
    print >>file, "  // Constructor"
    print >>file, "  public j%s ()" % className
    print >>file, "  {"
    print >>file, "    itsTreeID   = 0;"
    print >>file, "    itsRecordID = 0;"
    print >>file, '    itsNodename = "";'
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, '    %s = "";' % args[1]
      if args[3] in tInt + tUint + tLong + tULng + tFlt + tDbl:
        print >>file, "    %s = 0;" % args[1]
      if args[3] in tBool:
        print >>file, "    %s = false;" % args[1]
    print >>file, "  }"
    print >>file
    print >>file, "  public j%s (int aTreeID, int aRecordID, String aParent, String arrayList)" % className
    print >>file, "  {"
    print >>file, "    itsTreeID   = aTreeID;"
    print >>file, "    itsRecordID = aRecordID;"
    print >>file, "    itsNodename = aParent;"
    print >>file, '    String fields[] = arrayList.replace("{","").replace("}","").split(",");'
    print >>file, '    assert fields.length == %d : fields.length + " fields iso %d";' % (len(fieldList), len(fieldList));
    print >>file
    idx = 0
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, "    %s = fields[%d];" % (args[1], idx)
      if args[3] in tInt + tUint + tLong + tULng:
        print >>file, "    %s = Integer.valueOf(fields[%d]);" % (args[1], idx)
      if args[3] in tBool:
        print >>file, "    %s = Boolean.parseBoolean(fields[%d]);" % (args[1], idx)
      if args[3] in tFlt:
        print >>file, "    %s = Float.valueOf(fields[%d]);" % (args[1], idx)
      if args[3] in tDbl:
        print >>file, "    %s = Double.valueOf(fields[%d]);" % (args[1], idx)
      idx += 1
    print >>file, "  }"
    print >>file
    print >>file, "  // data access"
    print >>file, "  public int treeID()   { return itsTreeID; };"
    print >>file, "  public int recordID() { return itsRecordID; };"
    print >>file, "  public String nodeName() { return itsNodename; };"
    print >>file

def genCompareFunction(file,className,fieldList):
    print >>file, "  @Override"
    print >>file, "  public boolean equals(Object obj) {"
    print >>file, "    // if 2 objects are equal in reference, they are equal"
    print >>file, "    if (this == obj)"
    print >>file, "      return true;"
    print >>file, "    // type of object must match"
    print >>file, "    if (!(obj instanceof j%s))" % className
    print >>file, "      return false;"
    print >>file, "    j%s that = (j%s) obj;" % (className, className)
    print >>file, "    return",
    count = 0
    for field in fieldList:
      if count != 0:
         print >>file, "&&"
         print >>file, "          ",
      args = field.split()
      if args[3] in tText:
         print >>file, "that.%s.equals(this.%s)" % (args[1], args[1]),
      if args[3] in tInt + tUint + tLong + tULng + tFlt + tDbl + tBool:
         print >>file, "that.%s == this.%s" % (args[1], args[1]),
      count += 1
    print >>file, ";"
    print >>file, "  }"
    print >>file

def genFieldDictFunction(file,className,fieldList):
    print >>file, "  // fieldDict()"
    print >>file, "  public String fieldDict() {"
    file.write('    return "{')
    count = 0
    for field in fieldList:
      args = field.split()
      if count != 0:
         print >>file, '+ ",',
      print >>file, '%s: "+%s' % (args[1], args[1]),
      count += 1
      if count % 3 == 0:
         print >>file
         print >>file, "         ",
    print >>file, '+"}";'
    print >>file, '  }'
    print >>file

def genPrintFunction(file,className,fieldList):
    print >>file, "  // print()"
    print >>file, "  public String print() {"
    print >>file, '    return "{recordID: "+itsRecordID+", treeID: "+itsTreeID+", nodename: "+itsNodename + ","+ fieldDict()+"}";'
    print >>file, "  }"
    print >>file

def genDatamembers(file, className, fieldList):
    print >>file, "  // -- datamembers --"
    print >>file, "  private int      itsTreeID;"
    print >>file, "  private int      itsRecordID;"
    print >>file, "  private String   itsNodename;"
    print >>file
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, "  public String    %s;" % args[1]
      if args[3] in tInt + tUint:
        print >>file, "  public int       %s;" % args[1]
      if args[3] in tLong + tULng:
        print >>file, "  public long      %s;" % args[1]
      if args[3] in tBool:
        print >>file, "  public boolean   %s;" % args[1]
      if args[3] in tFlt:
        print >>file, "  public float     %s;" % args[1]
      if args[3] in tDbl:
        print >>file, "  public double    %s;" % args[1]
    print >>file, "}"
    print >>file

# jRecordAccessInterface.java
def genInterfaceHeader(file):
    print >>file, "package nl.astron.lofar.sas.otb.jotdb3;"
    print >>file, "import java.rmi.Remote;"
    print >>file, "import java.rmi.RemoteException;"
    print >>file, "import java.util.Vector;"
    print >>file
    print >>file, "public interface jRecordAccessInterface extends Remote"
    print >>file, "{"
    print >>file, "  // Constants"
    print >>file, '  public static final String SERVICENAME="jRecordAccess";'
    print >>file

def genRAInterface(file, tablename):
    print >>file, "  //--- j%s ---" % tablename
    print >>file, "  // get a single record"
    print >>file, "  public j%s get%s (int recordID) throws RemoteException;" % (tablename, tablename)
    print >>file, "  public j%s get%s (int treeID, String node) throws RemoteException;" % (tablename, tablename)
    print >>file, "  // get all records of one tree [and 1 type]"
    print >>file, "  public Vector<j%s> get%ss (int treeID) throws RemoteException;" % (tablename, tablename)
    print >>file, "  public Vector<j%s> get%ss (int treeID, String node) throws RemoteException;" % (tablename, tablename)
    print >>file, "  // get multiple records of multiple trees"
    print >>file, "  public Vector<j%s> get%ssOnTreeList (Vector<Integer> treeIDs) throws RemoteException;" % (tablename, tablename)
    print >>file, "  public Vector<j%s> get%ssOnRecordList (Vector<Integer> recordIDs) throws RemoteException;" % (tablename, tablename)
    print >>file, "  // get a single field of multiple records"
    print >>file, "  public Vector<String> get%sFieldOnRecordList (String fieldname, Vector<Integer> recordIDs) throws RemoteException;" % tablename
    print >>file, "  // save this record or 1 field of this record"
    print >>file, "  public boolean save%s(j%s aRec) throws RemoteException;" % (tablename, tablename)
    print >>file, "  public boolean save%sField(j%s aRec, int fieldIndex) throws RemoteException;" % (tablename, tablename)
    print >>file, "  // save 1 field of multiple records"
    print >>file, "  public boolean save%sFields(int fieldIndex, Vector<j%s> records) throws RemoteException;" % (tablename, tablename)

# jRecordAccess.java
def genRAHeader(file):
    print >>file, "package nl.astron.lofar.sas.otb.jotdb3;"
    print >>file, "import java.rmi.RemoteException;"
    print >>file, "import java.util.Vector;"
    print >>file
    print >>file, "public class jRecordAccess implements jRecordAccessInterface"
    print >>file, "{"
    print >>file, '  private String itsName = "";'
    print >>file, "  public jRecordAccess(String ext) {"
    print >>file, "    try {"
    print >>file, "      itsName = ext;"
    print >>file, "      initRecordAccess();"
    print >>file, "    } catch (Exception ex) {"
    print >>file, '      System.out.println("Error during jRecordAccess init : " +ex);'
    print >>file, "    }"
    print >>file, "  }"
    print >>file
    print >>file, "  // init jRecordAccess"
    print >>file, "  private native void initRecordAccess () throws RemoteException;"
    print >>file

def genRAFunctions(file, tablename):
    print >>file, "  //--- j%s ---" % tablename
    print >>file, "  // get a single record"
    print >>file, "  @Override"
    print >>file, "  public native j%s get%s (int recordID) throws RemoteException;" % (tablename, tablename)
    print >>file, "  @Override"
    print >>file, "  public native j%s get%s (int treeID, String node) throws RemoteException;" % (tablename, tablename)
    print >>file, "  // get all records of one tree [and 1 type]"
    print >>file, "  @Override"
    print >>file, "  public native Vector<j%s> get%ss (int treeID) throws RemoteException;" % (tablename, tablename)
    print >>file, "  @Override"
    print >>file, "  public native Vector<j%s> get%ss (int treeID, String node) throws RemoteException;" % (tablename, tablename)
    print >>file, "  // get multiple records of multiple trees"
    print >>file, "  @Override"
    print >>file, "  public native Vector<j%s> get%ssOnTreeList (Vector<Integer> treeIDs) throws RemoteException;" % (tablename, tablename)
    print >>file, "  @Override"
    print >>file, "  public native Vector<j%s> get%ssOnRecordList (Vector<Integer> recordIDs) throws RemoteException;" % (tablename, tablename)
    print >>file, "  // get a single field of multiple records"
    print >>file, "  @Override"
    print >>file, "  public native Vector<String> get%sFieldOnRecordList (String fieldname, Vector<Integer> recordIDs) throws RemoteException;" % tablename
    print >>file, "  // save this record or 1 field of this record"
    print >>file, "  @Override"
    print >>file, "  public native boolean save%s(j%s aRec) throws RemoteException;" % (tablename, tablename)
    print >>file, "  @Override"
    print >>file, "  public native boolean save%sField(j%s aRec, int fieldIndex) throws RemoteException;" % (tablename, tablename)
    print >>file, "  // save 1 field of multiple records"
    print >>file, "  @Override"
    print >>file, "  public native boolean save%sFields(int fieldIndex, Vector<j%s> records) throws RemoteException;" % (tablename, tablename)

# jRecordAccess.h
def genRAdotHfileHeader(file):
    print >>file, "#ifndef __nl_astron_lofar_sas_otb_jotdb3_jRecordAccess__"
    print >>file, "#define __nl_astron_lofar_sas_otb_jotdb3_jRecordAccess__"
    print >>file
    print >>file, "#include <jni.h>"
    print >>file
    print >>file, "#ifdef __cplusplus"
    print >>file, 'extern "C"'
    print >>file, "{"
    print >>file, "#endif"
    print >>file
    print >>file, "JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_initRecordAccess (JNIEnv *env, jobject);"
    print >>file

def genRAdotHFileFunctions(file, tablename):
    # When tablename contains a _ replace this with _1 to satisfy Java.
    functionname = tablename.replace('_', '_1')
    print >>file, "//--- j%s ---" % tablename
    print >>file, "// get a single record"
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%s__I (JNIEnv *env, jobject, jint);" % functionname
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%s__ILjava_lang_String_2 (JNIEnv *env, jobject, jint, jstring);" % functionname
    print >>file, "// get all records of one tree [and 1 type]"
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%ss__I (JNIEnv *env, jobject, jint);" % functionname
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%ss__ILjava_lang_String_2 (JNIEnv *env, jobject, jint, jstring);" % functionname
    print >>file, "// get multiple records of multiple trees"
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%ssOnTreeList (JNIEnv *env, jobject, jobject);" % functionname
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%ssOnRecordList (JNIEnv *env, jobject, jobject);" % functionname
    print >>file, "// get a single field of multiple records"
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%sFieldOnRecordList (JNIEnv *env, jobject, jstring, jobject);" % functionname
    print >>file, "// save this record or 1 field of this record"
    print >>file, "JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_save%s(JNIEnv *env, jobject, jobject);" % functionname
    print >>file, "JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_save%sField(JNIEnv *env, jobject, jobject, jint);" % functionname
    print >>file, "// save 1 field of multiple records"
    print >>file, "JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_save%sFields(JNIEnv *env, jobject, jint, jobject);" % functionname
    print >>file

# jRecordAccess.cc
def genRAdotCCheader(file, tablename,fieldList):
    print >>file, "#include <lofar_config.h>"
    print >>file, "#include <Common/LofarLogger.h>"
    print >>file, "#include <Common/StringUtil.h>"
    print >>file, "#include <jni.h>"
    print >>file, "#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jRecordAccess.h>"
    print >>file, "#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommonRec.h>"
    print >>file, "#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jOTDBconnection.h>"
    print >>file, "#include <iostream>"
    print >>file, "#include <string>"
    print >>file
    print >>file, "using namespace LOFAR::OTDB;"
    print >>file, "using namespace std;"
    print >>file
    print >>file, "JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_initRecordAccess (JNIEnv *env, jobject jRecordAccess) {"
    print >>file, "  string name = getOwnerExt(env, jRecordAccess);"
    print >>file, "  try {"
    print >>file, "    OTDBconnection* aConn=getConnection(name);"
    print >>file, '    theirC_ObjectMap.erase(name+"_RecordAccess");'
    print >>file, '    theirC_ObjectMap[name+"_RecordAccess"]=(void*)aConn;'
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during new RecordAccess "<< ex.what() << endl;'
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "  }"
    print >>file, "}"
    print >>file

def genOpenConnection():
    print >>file, "    // Get information"
    print >>file, "    string name = getOwnerExt(env, jRecordAccess);"
    print >>file, '    OTDBconnection* aConn=getConnection(name);'
    print >>file, '    ASSERTSTR(aConn->isConnected(),"Connnection flag failed");'

def genRAgetRecord1Function(file, tablename,fieldList):
    print >>file, "// ---- %s ----" % tablename
    print >>file, "#include <OTDB/%s.h>" % tablename
    print >>file, "// get%s(recordID)" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%s__I (JNIEnv *env, jobject jRecordAccess, jint recordID) {" % functionname
    print >>file, "  %s aRec;" % tablename
    print >>file, "  try {"
    genOpenConnection()
    print >>file, "    aRec= %s::getRecord (aConn,recordID);" % tablename
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::getRecord(" << recordID << ") " << ex.what() << endl;' % tablename
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "  }"
    print >>file, "  return convert%s(env, aRec);" % tablename
    print >>file, "}"
    print >>file

def genRAgetRecord2Function(file, tablename,fieldList):
    print >>file, "// get%s(treeID, parentname)" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%s__ILjava_lang_String_2 (JNIEnv *env, jobject jRecordAccess, jint treeID, jstring node) {" % functionname
    print >>file, "  %s aRec;" % tablename
    print >>file, "  const char* nodeName;"
    print >>file, "  jboolean isCopy;"
    print >>file, "  try {"
    genOpenConnection()
    print >>file, "    nodeName = env->GetStringUTFChars (node, &isCopy);"
    print >>file, "    aRec= %s::getRecord (aConn,treeID, nodeName);" % tablename
    print >>file, "    env->ReleaseStringUTFChars (node, nodeName);"
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::getRecord(" << treeID << "," << node <<") " << ex.what() << endl;' % tablename
    print >>file, "    env->ReleaseStringUTFChars (node, nodeName);"
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "  }"
    print >>file, "  return convert%s(env, aRec);" % tablename
    print >>file, "}"
    print >>file

def constructJavaVector():
    print >>file
    print >>file, "    // Construct Java vector"
    print >>file, '    jclass class_Vector = env->FindClass("java/util/Vector");'
    print >>file, '    jclass class_Integer = env->FindClass("java/lang/Integer");'
    print >>file, '    jmethodID mid_Vector_cons = env->GetMethodID(class_Vector, "<init>", "()V");'
    print >>file, '    jmethodID mid_Vector_size = env->GetMethodID(class_Vector, "size", "()I");'
    print >>file, '    jmethodID mid_Vector_add  = env->GetMethodID(class_Vector, "add", "(Ljava/lang/Object;)Z");'
    print >>file, '    jmethodID mid_Vector_elementAt = env->GetMethodID (class_Vector, "elementAt", "(I)Ljava/lang/Object;");'
    print >>file, '    jmethodID mid_Integer_intValue = env->GetMethodID (class_Integer, "intValue", "()I");'
    print >>file, '    itemVector = env->NewObject(class_Vector, mid_Vector_cons);'

def fillJavaVector(tablename):
    print >>file
    print >>file, "    // Copy C++ vector to Java vector"
    print >>file, "    vector<%s>::iterator  iter = itemList.begin();" % tablename
    print >>file, "    vector<%s>::iterator  end  = itemList.end();" % tablename
    print >>file, '    for ( ; iter != end; iter++) {'
    print >>file, '      env->CallObjectMethod(itemVector, mid_Vector_add, convert%s (env, *iter));' % tablename
    print >>file, '    }'

def genRAgetRecords1Function(file, tablename,fieldList):
    print >>file, "// get%s(treeID)" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%ss__I (JNIEnv *env, jobject jRecordAccess, jint treeID) {" % functionname
    print >>file, "  jobject itemVector;"
    print >>file, "  try {"
    genOpenConnection()
    print >>file, "    vector<%s> itemList = %s::getRecords (aConn,treeID);" % (tablename, tablename)
    constructJavaVector()
    fillJavaVector(tablename)
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::getRecords(" << treeID << ") " << ex.what() << endl;' % tablename
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "  }"
    print >>file, "  return itemVector;"
    print >>file, "}"
    print >>file

def genRAgetRecords2Function(file, tablename,fieldList):
    print >>file, "// get%ss(treeID, nodename)" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%ss__ILjava_lang_String_2 (JNIEnv *env, jobject jRecordAccess, jint treeID, jstring node) {" % functionname
    print >>file, "  jobject itemVector;"
    print >>file, "  const char* nodeName;"
    print >>file, "  jboolean isCopy;"
    print >>file, "  try {"
    genOpenConnection()
    print >>file, "    nodeName = env->GetStringUTFChars (node, &isCopy);"
    print >>file, "    vector<%s> itemList = %s::getRecords (aConn,treeID,nodeName);" % (tablename, tablename)
    print >>file, "    env->ReleaseStringUTFChars (node, nodeName);"
    constructJavaVector()
    fillJavaVector(tablename)
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::getRecords(" << treeID << "," << node <<") " << ex.what() << endl;' % tablename
    print >>file, "    env->ReleaseStringUTFChars (node, nodeName);"
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "  }"
    print >>file, "  return itemVector;"
    print >>file, "}"
    print >>file

def genRAgetOnTreelistFunction(file, tablename,fieldList):
    print >>file, "// get%ssOnTreeList(treeIDs)" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%ssOnTreeList (JNIEnv *env, jobject jRecordAccess, jobject jTreeIDs) {" % functionname
    print >>file, "  jobject itemVector;"
    print >>file, "  try {"
    constructJavaVector()
    print >>file, "    // copy C++ vector with IDs to Java"
    print >>file, "    vector<uint> ids;"
    print >>file, "    int nrIDs = env->CallIntMethod (jTreeIDs, mid_Vector_size);"
    print >>file, "    for (int i = 0; i < nrIDs; i++) {"
    print >>file, "      jobject anInt = env->CallObjectMethod (jTreeIDs, mid_Vector_elementAt, i);"
    print >>file, "      ids.push_back((uint)env->CallIntMethod (anInt, mid_Integer_intValue));"
    print >>file, "    }"
    genOpenConnection()
    print >>file, "    vector<%s> itemList = %s::getRecordsOnTreeList (aConn,ids);" % (tablename, tablename)
    fillJavaVector(tablename)
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::getRecordsOnTreeList() " << ex.what() << endl;' % tablename
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "  }"
    print >>file, "  return itemVector;"
    print >>file, "}"
    print >>file

def genRAgetOnRecordlistFunction(file, tablename,fieldList):
    print >>file, "// get%ssOnRecordList(RecordIDs)" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%ssOnRecordList (JNIEnv *env, jobject jRecordAccess, jobject jRecordIDs) {" % functionname
    print >>file, "  jobject itemVector;"
    print >>file, "  try {"
    constructJavaVector()
    print >>file, "    // copy C++ vector with IDs to Java"
    print >>file, "    vector<uint> ids;"
    print >>file, "    int nrIDs = env->CallIntMethod (jRecordIDs, mid_Vector_size);"
    print >>file, "    for (int i = 0; i < nrIDs; i++) {"
    print >>file, "      jobject anInt = env->CallObjectMethod (jRecordIDs, mid_Vector_elementAt, i);"
    print >>file, "      ids.push_back((uint)env->CallIntMethod(anInt,mid_Integer_intValue));"
    print >>file, "    }"
    genOpenConnection()
    print >>file, "    vector<%s> itemList = %s::getRecordsOnRecordList (aConn,ids);" % (tablename, tablename)
    fillJavaVector(tablename)
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::getRecordsOnRecordList() " << ex.what() << endl;' % tablename
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "  }"
    print >>file, "  return itemVector;"
    print >>file, "}"
    print >>file

def genRAgetFieldOnRecordlistFunction(file, tablename,fieldList):
    print >>file, "// get%sFieldOnRecordList(fieldname, RecordIDs)" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_get%sFieldOnRecordList (JNIEnv *env, jobject jRecordAccess, jstring jFieldname, jobject jRecordIDs) {" % functionname
    print >>file, "  jobject itemVector;"
    print >>file, "  const char* fieldname;"
    print >>file, "  jboolean isCopy;"
    print >>file, "  try {"
    constructJavaVector()
    print >>file, "    // copy C++ vector with IDs to Java"
    print >>file, "    vector<uint> ids;"
    print >>file, "    int nrIDs = env->CallIntMethod (jRecordIDs, mid_Vector_size);"
    print >>file, "    for (int i = 0; i < nrIDs; i++) {"
    print >>file, "      jobject anInt = env->CallObjectMethod (jRecordIDs, mid_Vector_elementAt, i);"
    print >>file, "      ids.push_back((uint)env->CallIntMethod(anInt,mid_Integer_intValue));"
    print >>file, "    }"
    genOpenConnection()
    print >>file, "    fieldname = env->GetStringUTFChars (jFieldname, &isCopy);"
    print >>file, "    vector<string> itemList = %s::getFieldOnRecordList (aConn,fieldname,ids);" % tablename
    print >>file, "    env->ReleaseStringUTFChars (jFieldname, fieldname);"
    print >>file
    print >>file, "    // Copy C++ vector to Java vector"
    print >>file, "    vector<string>::iterator  iter = itemList.begin();"
    print >>file, "    vector<string>::iterator  end  = itemList.end();"
    print >>file, '    for ( ; iter != end; iter++) {'
    print >>file, '      env->CallObjectMethod(itemVector, mid_Vector_add, env->NewStringUTF(((string)*iter).c_str()));'
    print >>file, '    }'
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::getFieldOnRecordList() " << ex.what() << endl;' % tablename
    print >>file, "    env->ReleaseStringUTFChars (jFieldname, fieldname);"
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "  }"
    print >>file, "  return itemVector;"
    print >>file, "}"
    print >>file

def genRAsaveRecordFunction(file, tablename,fieldList):
    print >>file, "// save%s()" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_save%s (JNIEnv *env, jobject jRecordAccess, jobject jRec) {" % functionname
    print >>file, "  jboolean success=false;"
    print >>file, "  %s aRec = convertj%s(env, jRec);" % (tablename, tablename)
    print >>file, "  try {"
    genOpenConnection()
    print >>file, "    success = aRec.save(aConn);"
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::save() " << ex.what() << endl;' % tablename
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "    success = false;"
    print >>file, "  }"
    print >>file, "  return success;"
    print >>file, "}"
    print >>file

def genRAsaveRecordFieldFunction(file, tablename,fieldList):
    print >>file, "// save%sField(fieldIndex)" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_save%sField (JNIEnv *env, jobject jRecordAccess, jobject jRec, jint fieldIndex) {" % functionname
    print >>file, "  jboolean success=false;"
    print >>file, "  %s aRec = convertj%s(env, jRec);" % (tablename, tablename)
    print >>file, "  try {"
    genOpenConnection()
    print >>file, "    success = aRec.saveField(aConn, fieldIndex);"
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::saveField("<< fieldIndex <<")" << ex.what() << endl;' % tablename
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "    success = false;"
    print >>file, "  }"
    print >>file, "  return success;"
    print >>file, "}"
    print >>file

def genRAsaveRecordFieldsFunction(file, tablename,fieldList):
    print >>file, "// save%sFields(fieldIndex,records)" % tablename
    functionname = tablename.replace('_', '_1')
    print >>file, "JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jRecordAccess_save%sFields (JNIEnv *env, jobject jRecordAccess, jint fieldIndex, jobject records) {" % functionname
    print >>file, "  jboolean success=false;"
    print >>file, "  jobject itemVector;"
    print >>file, "  try {"
    constructJavaVector()
    print >>file, "    vector<%s> pRecs;" % tablename
    print >>file, "    int nrRecs = env->CallIntMethod (records, mid_Vector_size);"
    print >>file, "    for (int i = 0; i < nrRecs; i++) {"
    print >>file, "      %s aRec = convertj%s(env,env->CallObjectMethod (records, mid_Vector_elementAt, i));" % (tablename, tablename)
    print >>file, "      pRecs.push_back(aRec);"
    print >>file, "    }"
    genOpenConnection()
    print >>file, "    success = %s::saveFields(aConn, fieldIndex,pRecs);" % tablename
    print >>file, "  } catch (exception &ex) {"
    print >>file, '    cout << "Exception during %s::saveFields("<< fieldIndex <<", %s Vector)" << ex.what() << endl;' % (tablename, tablename)
    print >>file, '    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());'
    print >>file, "    success = false;"
    print >>file, "  }"
    print >>file, "  return success;"
    print >>file, "}"
    print >>file

# jCommonRec.h
def genCRdotHfileHeader(file):
    print >>file, "#ifndef LOFAR_JOTDB_COMMONREC_H"
    print >>file, "#define LOFAR_JOTDB_COMMONREC_H"
    print >>file
    print >>file, "#include <jni.h>"
    print >>file, "#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommon.h>"
    print >>file, "#include <string>"
    print >>file, "#include <map>"
    print >>file

def genCRdotHFileFunctions(file, tablename):
    print >>file, "//--- j%s ---" % tablename
    print >>file, "#include <OTDB/%s.h>" % tablename
    print >>file, "jobject convert%s (JNIEnv *env, LOFAR::OTDB::%s aRec);" % (tablename, tablename)
    print >>file, "LOFAR::OTDB::%s convertj%s (JNIEnv *env, jobject jRec);" % (tablename, tablename)
    print >>file

# jCommonRec.cc
def genCRdotCCheader(file, tablename,fieldList):
    print >>file, "#include <lofar_config.h>"
    print >>file, "#include <Common/LofarLogger.h>"
    print >>file, "#include <Common/StringUtil.h>"
    print >>file, "#include <jOTDB3/nl_astron_lofar_sas_otb_jotdb3_jCommonRec.h>"
    print >>file, "#include <string>"
    print >>file, "#include <iostream>"
    print >>file, "#include <map>"
    print >>file
    print >>file, "using namespace LOFAR::OTDB;"
    print >>file, "using namespace std;"
    print >>file

def genCRtoJavaFunction(file, tablename,fieldList):
    print >>file, "// c++ --> java"
    print >>file, "jobject convert%s (JNIEnv *env, %s aRec)" % (tablename, tablename)
    print >>file, "{"
    print >>file, "  jobject   jRec;"
    print >>file, '  jclass    class_j%s    = env->FindClass("nl/astron/lofar/sas/otb/jotdb3/j%s");' % (tablename, tablename)
    print >>file, '  jmethodID mid_j%s_cons = env->GetMethodID(class_j%s, "<init>", "(IILjava/lang/String;Ljava/lang/String;)V");' % (tablename, tablename)
    print >>file
    print >>file, "  stringstream ss (stringstream::in | stringstream::out);"
    for field in fieldList:
      args = field.split()
      if (args[3] not in tText):
         print >>file, '  ss << aRec.%s;' % args[1]
         print >>file, '  string c%s = ss.str();' % args[1]
         print >>file, '  ss.seekp(0);'
    print >>file
    print >>file, '  string arrayList = string("{") +',
    count = 0
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, "aRec.%s + " % args[1],
      else:
        print >>file, "c%s + " % args[1],
      count += 1
      if count != len(fieldList):
        print >>file, '"," +',
    print >>file, '"}";'
    print >>file
    print >>file, "  jstring jArrayList = env->NewStringUTF(arrayList.c_str());"
    print >>file, "  jstring jNodeName  = env->NewStringUTF(aRec.nodeName().c_str());"
    print >>file, "  jRec = env->NewObject (class_j%s, mid_j%s_cons, aRec.treeID(),aRec.recordID(),jNodeName,jArrayList);" % (tablename, tablename)
    print >>file, "  return jRec;"
    print >>file, "}"
    print >>file

def J2Sstring(tablename, fieldname):
    print >>file, "  // %s" % fieldname
    print >>file, "  jstring %sStr  = (jstring)env->GetObjectField(jRec, fid_j%s_%s);" % (fieldname, tablename, fieldname)
    print >>file, "  const char* %sPtr = env->GetStringUTFChars(%sStr, 0);" % (fieldname, fieldname)
    print >>file, "  const string %s (%sPtr);" % (fieldname, fieldname)
    print >>file, "  env->ReleaseStringUTFChars(%sStr, %sPtr);" % (fieldname, fieldname)
    print >>file

def J2Sinteger(tablename, fieldname):
    print >>file, "  // %s" % fieldname
    print >>file, "  int %sInt  = env->GetIntField(jRec, fid_j%s_%s);" % (fieldname, tablename, fieldname)
    print >>file, "  ss << %sInt;" % fieldname
    print >>file, "  string %s = ss.str();" % fieldname
    print >>file, "  ss.seekp(0);"
    print >>file

def J2Slong(tablename, fieldname):
    print >>file, "  // %s" % fieldname
    print >>file, "  long %sLong  = env->GetLongField(jRec, fid_j%s_%s);" % (fieldname, tablename, fieldname)
    print >>file, "  ss << %sLong;" % fieldname
    print >>file, "  string %s = ss.str();" % fieldname
    print >>file, "  ss.seekp(0);"
    print >>file

def J2Sboolean(tablename, fieldname):
    print >>file, "  // %s" % fieldname
    print >>file, "  bool %sBool  = env->GetBooleanField(jRec, fid_j%s_%s);" % (fieldname, tablename, fieldname)
    print >>file, "  ss << %sBool;" % fieldname
    print >>file, "  string %s = ss.str();" % fieldname
    print >>file, "  ss.seekp(0);"
    print >>file

def J2Sfloat(tablename, fieldname):
    print >>file, "  // %s" % fieldname
    print >>file, "  float %sFlt  = env->GetFloatField(jRec, fid_j%s_%s);" % (fieldname, tablename, fieldname)
    print >>file, "  ss << %sFlt;" % fieldname
    print >>file, "  string %s = ss.str();" % fieldname
    print >>file, "  ss.seekp(0);"
    print >>file

def J2Sdouble(tablename, fieldname):
    print >>file, "  // %s" % fieldname
    print >>file, "  double %sDbl  = env->GetDoubleField(jRec, fid_j%s_%s);" % (fieldname, tablename, fieldname)
    print >>file, "  ss << %sDbl;" % fieldname
    print >>file, "  string %s = ss.str();" % fieldname
    print >>file, "  ss.seekp(0);"
    print >>file

def genCRtoCppFunction(file, tablename,fieldList):
    print >>file, "// java --> c++"
    print >>file, "%s convertj%s (JNIEnv *env, jobject jRec)" % (tablename, tablename)
    print >>file, "{"
    print >>file, "  jclass    class_j%s = env->GetObjectClass(jRec);" % tablename
    print >>file, '  jmethodID mid_j%s_treeID   = env->GetMethodID(class_j%s, "treeID", "()I");' % (tablename, tablename)
    print >>file, '  jmethodID mid_j%s_recordID = env->GetMethodID(class_j%s, "recordID", "()I");' % (tablename, tablename)
    print >>file, '  jmethodID mid_j%s_nodeName = env->GetMethodID(class_j%s, "nodeName", "()Ljava/lang/String;");' % (tablename, tablename)
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        print >>file, '  jfieldID fid_j%s_%s = env->GetFieldID(class_j%s, "%s", "Ljava/lang/String;");' % (tablename, args[1], tablename, args[1])
      if args[3] in tInt + tUint:
        print >>file, '  jfieldID fid_j%s_%s = env->GetFieldID(class_j%s, "%s", "I");' % (tablename, args[1], tablename, args[1])
      if args[3] in tLong + tULng:
        print >>file, '  jfieldID fid_j%s_%s = env->GetFieldID(class_j%s, "%s", "J");' % (tablename, args[1], tablename, args[1])
      if args[3] in tBool:
        print >>file, '  jfieldID fid_j%s_%s = env->GetFieldID(class_j%s, "%s", "Z");' % (tablename, args[1], tablename, args[1])
      if args[3] in tFlt:
        print >>file, '  jfieldID fid_j%s_%s = env->GetFieldID(class_j%s, "%s", "F");' % (tablename, args[1], tablename, args[1])
      if args[3] in tDbl:
        print >>file, '  jfieldID fid_j%s_%s = env->GetFieldID(class_j%s, "%s", "D");' % (tablename, args[1], tablename, args[1])
    print >>file
    print >>file, "  // nodeName"
    print >>file, "  jstring nodeNamestr  = (jstring)env->CallObjectMethod(jRec, mid_j%s_nodeName);" % tablename
    print >>file, "  const char* n = env->GetStringUTFChars(nodeNamestr, 0);"
    print >>file, "  const string nodeName (n);"
    print >>file, "  env->ReleaseStringUTFChars(nodeNamestr, n);"
    print >>file
    print >>file, "  stringstream ss (stringstream::in | stringstream::out);"
    print >>file
    for field in fieldList:
      args = field.split()
      if args[3] in tText:
        J2Sstring(tablename, args[1])
      if args[3] in tInt + tUint:
        J2Sinteger(tablename, args[1])
      if args[3] in tLong + tULng:
        J2Slong(tablename, args[1])
      if args[3] in tBool:
        J2Sboolean(tablename, args[1])
      if args[3] in tFlt:
        J2Sfloat(tablename, args[1])
      if args[3] in tDbl:
        J2Sdouble(tablename, args[1])
    print >>file
    print >>file, '  string arrayList = string("{") +',
    count = 0
    for field in fieldList:
      args = field.split()
      print >>file, "%s + " % args[1],
      count += 1
      if count != len(fieldList):
        print >>file, '"," +',
    print >>file, '"}";'
    print >>file
    print >>file, "  // Get original %s" % tablename
    print >>file, "  %s aRec = %s((int)env->CallIntMethod (jRec, mid_j%s_treeID)," % (tablename, tablename, tablename)
    print >>file, "               (int)env->CallIntMethod (jRec, mid_j%s_recordID)," % (tablename)
    print >>file, "               nodeName, arrayList);"
    print >>file, "  return aRec;"
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
      if args[3] in tText + tInt + tUint + tLong + tULng + tFlt + tDbl:
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
      if args[3] in tInt + tUint + tLong + tULng + tFlt + tDbl:
         print >>file, '  case %d: return(toString(%s)); break;' % (count, args[1])
      if args[3] in tBool:
         print >>file, '  case %d: return(%s ? "true" : "false"); break;' % (count, args[1])
      count += 1
    print >>file, "  };"
    print >>file, '  return("");'
    print >>file, "};"
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

# Every table has its own java file with the Java equivalent of the C++ class.
for DBfile in DBfiles:
  tablename = lgrep("^table", open(DBfile).readlines())[0].split()[1]
  fieldLines = lgrep("^field", open(DBfile).readlines())

  print "j"+tablename+".java"
  file = open("j"+tablename+".java", "w")
  genHeader                (file, tablename)
  genConstructor           (file, tablename, fieldLines)
  genCompareFunction       (file, tablename, fieldLines)
  genFieldDictFunction     (file, tablename, fieldLines)
  genPrintFunction         (file, tablename, fieldLines)
  genDatamembers           (file, tablename, fieldLines)
  file.close()

# The rest of the files contain the collection of all record-types
print "jRecordAccessInterface.java"
file = open("jRecordAccessInterface.java", "w")
genInterfaceHeader(file)
for DBfile in DBfiles:
  tablename = lgrep("^table", open(DBfile).readlines())[0].split()[1]
  fieldLines = lgrep("^field", open(DBfile).readlines())
  genRAInterface(file, tablename)
print >>file, "}"
file.close()

print "jRecordAccess.java"
file = open("jRecordAccess.java", "w")
genRAHeader(file)
for DBfile in DBfiles:
  tablename = lgrep("^table", open(DBfile).readlines())[0].split()[1]
  fieldLines = lgrep("^field", open(DBfile).readlines())
  genRAFunctions(file, tablename)
print >>file, "}"
file.close()

print "nl_astron_lofar_sas_otb_jotdb3_jRecordAccess.h"
file = open("nl_astron_lofar_sas_otb_jotdb3_jRecordAccess.h", "w")
genRAdotHfileHeader(file)
for DBfile in DBfiles:
  tablename = lgrep("^table", open(DBfile).readlines())[0].split()[1]
  fieldLines = lgrep("^field", open(DBfile).readlines())
  genRAdotHFileFunctions(file, tablename)
print >>file, "#ifdef __cplusplus"
print >>file, "}"
print >>file, "#endif"
print >>file, "#endif /* __nl_astron_lofar_sas_otb_jotdb3_jRecordAccess__ */"
file.close()

print "nl_astron_lofar_sas_otb_jotdb3_jRecordAccess.cc"
file = open("nl_astron_lofar_sas_otb_jotdb3_jRecordAccess.cc", "w")
genRAdotCCheader(file, tablename, fieldLines)
for DBfile in DBfiles:
  tablename = lgrep("^table", open(DBfile).readlines())[0].split()[1]
  fieldLines = lgrep("^field", open(DBfile).readlines())
  genRAgetRecord1Function(file, tablename,fieldLines)
  genRAgetRecord2Function(file, tablename,fieldLines)
  genRAgetRecords1Function(file, tablename,fieldLines)
  genRAgetRecords2Function(file, tablename,fieldLines)
  genRAgetOnTreelistFunction(file, tablename,fieldLines)
  genRAgetOnRecordlistFunction(file, tablename,fieldLines)
  genRAgetFieldOnRecordlistFunction(file, tablename,fieldLines)
  genRAsaveRecordFunction(file, tablename,fieldLines)
  genRAsaveRecordFieldFunction(file, tablename,fieldLines)
  genRAsaveRecordFieldsFunction(file, tablename,fieldLines)
file.close()

print "nl_astron_lofar_sas_otb_jotdb3_jCommonRec.h"
file = open("nl_astron_lofar_sas_otb_jotdb3_jCommonRec.h", "w")
genCRdotHfileHeader(file)
for DBfile in DBfiles:
  tablename = lgrep("^table", open(DBfile).readlines())[0].split()[1]
  fieldLines = lgrep("^field", open(DBfile).readlines())
  genCRdotHFileFunctions(file, tablename)
print >>file, "#endif"
file.close()

print "nl_astron_lofar_sas_otb_jotdb3_jCommonRec.cc"
file = open("nl_astron_lofar_sas_otb_jotdb3_jCommonRec.cc", "w")
genCRdotCCheader(file, tablename, fieldLines)
for DBfile in DBfiles:
  tablename = lgrep("^table", open(DBfile).readlines())[0].split()[1]
  fieldLines = lgrep("^field", open(DBfile).readlines())
  genCRtoJavaFunction(file, tablename, fieldLines)
  genCRtoCppFunction(file, tablename, fieldLines)
file.close()

