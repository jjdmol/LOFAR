// Always include lofar_config.h
#include <lofar_config.h>

// Includes
#include <jRSP/nl_astron_lofar_java_mac_jrsp_Board.h>
#include <RSP/RSPport.h>
using namespace LOFAR;
using namespace LOFAR::RSP;

#include <cstdio>
#include <iostream>
using namespace std;

// Define function's
jobject ConvertBoardStatus(JNIEnv*, BoardStatus&);

/**
 * Returns a pointer to a new instance of RSPport. The returned pointer can be
 * used by the other functions to call functions from RSPport without using
 * a different RSPport.
 * Every RSPport makes a connection with the RSP driver, so we only want one
 * RSPport.
 * @param	env		The java environment interface pointer.
 * @param	obj		The "this" pointer.
 * @return	ptrRSPport	Pointer to the RSPport object.
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_init(JNIEnv * env, jobject, jstring hostname)
{
  // Convert the jstring hostname to a C++ string.
  const char * charsHostname = env->GetStringUTFChars(hostname, 0);
  string strHostname (charsHostname);
  
  cout << "Entering init hostname: " << hostname << endl;
  RSPport * IOport;
  jint ptrRSPport = 0; // the return value, pointer to RSPport.
  
  try {
    IOport = new RSPport(strHostname);
    ptrRSPport = (jint)IOport; // set return value to pointer.
  } catch (exception &ex) {
    cout << "Error during init hostname: " << hostname << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
  
  // Free local references.	
  env->ReleaseStringUTFChars(hostname, charsHostname);
  
  cout << "Leaving init hostname: " << hostname << endl;
  return ptrRSPport;
}

/**
 * Deletes the instance of RSPport we have created in init() based on the jint
 * we have passed with this method.
 * @param	env		The Java environment interface pointer.
 * @param	obj		The "this" pointer.
 * @param	ptrRSPport	Pointer to the RSPport that will be deleted.
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_delete(JNIEnv * , jobject, jint ptrRSPport) {
  cout << "Entering delete" << endl;
  RSPport * IOport = (RSPport*)ptrRSPport;
  delete(IOport);
  cout <<"Leaving delete" << endl;
}

/**
 * Implementation of the JNI method declared in the java file (nl.astron.lofar.java.mac.jrsp.Board).
 * This function fills a BoardStatus object, that is returned by this method, with data from RSPIO.
 * @param	env		The Java environment interface pointer.
 * @param	obj		The "this" pointer.
 * @param	ptrRSPport	Pointer to the RSPport that should be used.
 * @return 	status		A array of StatusBoard class instances filled with information.
 */
JNIEXPORT jobjectArray JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_retrieveStatus(JNIEnv * env, jobject , jint rcuMask, jint ptrRSPport)
{
  cout << "Entering retrieveStatus rcuMask: " << rcuMask << endl;

  // Use RSPport to get a vector with BoardStatus.
  vector<BoardStatus> vecBoardStatus;
  jobjectArray arrBoardStatus;
  RSPport * IOport = (RSPport*)ptrRSPport;
  try {
    vecBoardStatus = IOport->getBoardStatus(rcuMask);
	
    // The jobjectArray that is going to be returned.
    arrBoardStatus = (jobjectArray)env->NewObjectArray(vecBoardStatus.size(), env->FindClass("nl/astron/lofar/java/mac/jrsp/BoardStatus"), NULL);
    
    for(uint i=0; i<vecBoardStatus.size(); i++)
      {
	env->SetObjectArrayElement(arrBoardStatus, i, ConvertBoardStatus(env, vecBoardStatus[i]));
      }
  } catch (exception &ex) {
    cout << "Error during retrieveStatus rcuMask: " << rcuMask << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }

  cout << "Leaving retrieveStatus rcuMask: " << rcuMask << endl;
  
  return arrBoardStatus;
}

/**
 * This method converts an C++ BoardStatus to a Java BoardStatus.
 * @param	BoardStatus	C++ BoardStatus
 * @return 	jobject		Java BoardStatus
 */
jobject ConvertBoardStatus(JNIEnv *env, BoardStatus &boardStatus)
{
  cout << "Entering ConvertBoardStatus" << endl;
  // TODO: Deze code buiten deze functie plaatsen zodat het niet vaker dan een keer aangeroepen hoeft te worden.
  // Get a reference to the class of the status (BoardStatus).
  jclass clsStatus = env->FindClass("nl/astron/lofar/java/mac/jrsp/BoardStatus");
  if(clsStatus == NULL)
    {
      return NULL;
    }
  static jmethodID sbConstructorId = env->GetMethodID(clsStatus, "<init>", "()V");
  if(sbConstructorId == NULL)
    {
      return NULL;
    }
  jobject status = env->NewObject(clsStatus, sbConstructorId, NULL);
  
  // Get field identifiers for the member variables  of the StatusBoard class.
  jfieldID fidVoltage1V2 = env->GetFieldID(clsStatus, "voltage1V2", "D");
  jfieldID fidVoltage2V5 = env->GetFieldID(clsStatus, "voltage2V5", "D");
  jfieldID fidVoltage3V3 = env->GetFieldID(clsStatus, "voltage3V3", "D");
  jfieldID fidPcbTemp = env->GetFieldID(clsStatus, "pcbTemp", "S");
  jfieldID fidBpTemp = env->GetFieldID(clsStatus, "bpTemp", "S");
  jfieldID fidAp0Temp = env->GetFieldID(clsStatus, "ap0Temp", "S");
  jfieldID fidAp1Temp = env->GetFieldID(clsStatus, "ap1Temp", "S");
  jfieldID fidAp2Temp = env->GetFieldID(clsStatus, "ap2Temp", "S");
  jfieldID fidAp3Temp = env->GetFieldID(clsStatus, "ap3Temp", "S");
  jfieldID fidBpClock = env->GetFieldID(clsStatus, "bpClock", "S");
  jfieldID fidNofFrames = env->GetFieldID(clsStatus, "nofFrames", "I");
  jfieldID fidNofErrors = env->GetFieldID(clsStatus, "nofErrors", "I");
  jfieldID fidLastError = env->GetFieldID(clsStatus, "lastError", "S");
  jfieldID fidSeqNr = env->GetFieldID(clsStatus, "seqNr", "I");
  jfieldID fidError = env->GetFieldID(clsStatus, "error", "S");
  jfieldID fidIfUnderTest = env->GetFieldID(clsStatus, "ifUnderTest", "S");
  jfieldID fidMode = env->GetFieldID(clsStatus, "mode", "S");
  jfieldID fidRiErrors = env->GetFieldID(clsStatus, "riErrors", "I");
  jfieldID fidRcuxErrors = env->GetFieldID(clsStatus, "rcuxErrors", "I");
  jfieldID fidRcuyErrors = env->GetFieldID(clsStatus, "rcuyErrors", "I");
  jfieldID fidLcuErrors = env->GetFieldID(clsStatus, "lcuErrors", "I");
  jfieldID fidCepErrors = env->GetFieldID(clsStatus, "cepErrors", "I");
  jfieldID fidSerdesErrors = env->GetFieldID(clsStatus, "serdesErrors", "I");
  jfieldID fidAp0RiErrors = env->GetFieldID(clsStatus, "ap0RiErrors", "I");
  jfieldID fidAp1RiErrors = env->GetFieldID(clsStatus, "ap1RiErrors", "I");
  jfieldID fidAp2RiErrors = env->GetFieldID(clsStatus, "ap2RiErrors", "I");
  jfieldID fidAp3RiErrors = env->GetFieldID(clsStatus, "ap3RiErrors", "I");
  jfieldID fidBlp0Sync = env->GetFieldID(clsStatus, "blp0Sync", "Lnl/astron/lofar/java/mac/jrsp/SyncStatus;");
  jfieldID fidBlp1Sync = env->GetFieldID(clsStatus, "blp1Sync", "Lnl/astron/lofar/java/mac/jrsp/SyncStatus;");
  jfieldID fidBlp2Sync = env->GetFieldID(clsStatus, "blp2Sync", "Lnl/astron/lofar/java/mac/jrsp/SyncStatus;");
  jfieldID fidBlp3Sync = env->GetFieldID(clsStatus, "blp3Sync", "Lnl/astron/lofar/java/mac/jrsp/SyncStatus;");
  jfieldID fidBlp0Rcu = env->GetFieldID(clsStatus, "blp0Rcu", "Lnl/astron/lofar/java/mac/jrsp/RCUStatus;");
  jfieldID fidBlp1Rcu = env->GetFieldID(clsStatus, "blp1Rcu", "Lnl/astron/lofar/java/mac/jrsp/RCUStatus;");
  jfieldID fidBlp2Rcu = env->GetFieldID(clsStatus, "blp2Rcu", "Lnl/astron/lofar/java/mac/jrsp/RCUStatus;");
  jfieldID fidBlp3Rcu = env->GetFieldID(clsStatus, "blp3Rcu", "Lnl/astron/lofar/java/mac/jrsp/RCUStatus;");
  jfieldID fidCpRdy = env->GetFieldID(clsStatus, "cpRdy", "Z");
  jfieldID fidCpErr = env->GetFieldID(clsStatus, "cpErr", "Z");
  jfieldID fidCpFpga = env->GetFieldID(clsStatus, "cpFpga", "Z");
  jfieldID fidCpIm = env->GetFieldID(clsStatus, "cpIm", "Z");
  jfieldID fidCpTrig = env->GetFieldID(clsStatus, "cpTrig", "S");
  jfieldID fidBlp0AdcOffset = env->GetFieldID(clsStatus, "blp0AdcOffset", "Lnl/astron/lofar/java/mac/jrsp/ADOStatus;");
  jfieldID fidBlp1AdcOffset = env->GetFieldID(clsStatus, "blp1AdcOffset", "Lnl/astron/lofar/java/mac/jrsp/ADOStatus;");
  jfieldID fidBlp2AdcOffset = env->GetFieldID(clsStatus, "blp2AdcOffset", "Lnl/astron/lofar/java/mac/jrsp/ADOStatus;");
  jfieldID fidBlp3AdcOffset = env->GetFieldID(clsStatus, "blp3AdcOffset", "Lnl/astron/lofar/java/mac/jrsp/ADOStatus;");
  
  // Access fields and fill them
  if(fidVoltage1V2 != 0)
    {
      env->SetDoubleField(status, fidVoltage1V2, boardStatus.rsp.voltage_1_2 / 192.0 * 2.5);
    }
  if(fidVoltage2V5 != 0)
    {
      env->SetDoubleField(status, fidVoltage2V5, boardStatus.rsp.voltage_2_5 / 192.0 * 3.3);
    }
  if(fidVoltage3V3 != 0)
    {
      env->SetDoubleField(status, fidVoltage3V3, boardStatus.rsp.voltage_3_3 / 192.0 * 5.0);
    }
  if(fidPcbTemp != 0)
    {
      env->SetShortField(status, fidPcbTemp, boardStatus.rsp.pcb_temp);
    }
  if(fidBpTemp != 0)
    {
      env->SetShortField(status, fidBpTemp, boardStatus.rsp.bp_temp);
    }
  if(fidAp0Temp != 0)
    {
      env->SetShortField(status, fidAp0Temp, boardStatus.rsp.ap0_temp);
    }
  if(fidAp1Temp != 0)
    {
      env->SetShortField(status, fidAp1Temp, boardStatus.rsp.ap1_temp);
    }
  if(fidAp2Temp != 0)
    {
      env->SetShortField(status, fidAp2Temp, boardStatus.rsp.ap2_temp);
    }
  if(fidAp3Temp != 0)
    {
      env->SetShortField(status, fidAp3Temp, boardStatus.rsp.ap3_temp);
    }
  if(fidBpClock != 0)
    {
      env->SetShortField(status, fidBpClock, boardStatus.rsp.bp_clock);
    }
  if(fidNofFrames != 0)
    {
      env->SetIntField(status, fidNofFrames, boardStatus.eth.nof_frames);
    }
  if(fidNofErrors != 0)
    {
      env->SetIntField(status, fidNofErrors, boardStatus.eth.nof_errors);
    }
  if(fidLastError != 0)
    {
      env->SetShortField(status, fidLastError, boardStatus.eth.last_error);
    }
  if(fidSeqNr != 0)
    {
      env->SetIntField(status, fidSeqNr, boardStatus.mep.seqnr);
    }
  if(fidError != 0)
    {
      env->SetShortField(status, fidError, boardStatus.mep.error);
    }
  if(fidIfUnderTest != 0)
    {
      env->SetShortField(status, fidIfUnderTest, boardStatus.diag.interface);
    }
  if(fidMode != 0)
    {
      env->SetShortField(status, fidMode, boardStatus.diag.mode);
    }
  if(fidRiErrors != 0)
    {
      env->SetIntField(status, fidRiErrors, boardStatus.diag.ri_errors);
    }
  if(fidRcuxErrors != 0)
    {
      env->SetIntField(status, fidRcuxErrors, boardStatus.diag.rcux_errors);
    }
  if(fidRcuyErrors != 0)
    {
      env->SetIntField(status, fidRcuyErrors, boardStatus.diag.rcuy_errors);
    }
  if(fidLcuErrors != 0)
    {
      env->SetIntField(status, fidLcuErrors, boardStatus.diag.lcu_errors);
    }
  if(fidCepErrors != 0)
    {
      env->SetIntField(status, fidCepErrors, boardStatus.diag.cep_errors);
    }
  if(fidSerdesErrors != 0)
    {
      env->SetIntField(status, fidSerdesErrors, boardStatus.diag.serdes_errors);
    }
  if(fidAp0RiErrors != 0)
    {
      env->SetIntField(status, fidAp0RiErrors, boardStatus.diag.ap0_ri_errors);
    }
  if(fidAp1RiErrors != 0)
    {
      env->SetIntField(status, fidAp1RiErrors, boardStatus.diag.ap1_ri_errors);
    }
  if(fidAp2RiErrors != 0)
    {
      env->SetIntField(status, fidAp2RiErrors, boardStatus.diag.ap2_ri_errors);
    }
  if(fidAp3RiErrors != 0)
    {
      env->SetIntField(status, fidAp3RiErrors, boardStatus.diag.ap3_ri_errors);
    }
  
  // SyncStatus: blp0Sync - blp3Sync
  jclass clsSyncStatus = env->FindClass("nl/astron/lofar/java/mac/jrsp/SyncStatus");
  jfieldID fidExtCount = env->GetFieldID(clsSyncStatus, "extCount", "I");
  jfieldID fidSyncCount = env->GetFieldID(clsSyncStatus, "syncCount", "I");
  jfieldID fidSampleOffset = env->GetFieldID(clsSyncStatus, "sampleOffset", "I");
  jfieldID fidSliceCount = env->GetFieldID(clsSyncStatus, "sliceCount", "I");
  
  if(fidBlp0Sync != 0)
    {
      jobject objBlp0Sync = env->GetObjectField(status, fidBlp0Sync);
      if(fidExtCount != 0 && fidSyncCount != 0 && fidSampleOffset != 0 && fidSliceCount != 0)
	{
	  env->SetIntField(objBlp0Sync, fidExtCount, boardStatus.ap0_sync.ext_count);
	  env->SetIntField(objBlp0Sync, fidSyncCount, boardStatus.ap0_sync.sync_count);
	  env->SetIntField(objBlp0Sync, fidSampleOffset, boardStatus.ap0_sync.sample_offset);
	  env->SetIntField(objBlp0Sync, fidSliceCount, boardStatus.ap0_sync.slice_count);
	}
    }
  
  if(fidBlp1Sync != 0)
    {
      jobject objBlp1Sync = env->GetObjectField(status, fidBlp1Sync);
      if(fidExtCount != 0 && fidSyncCount != 0 && fidSampleOffset != 0 && fidSliceCount != 0)
	{
	  env->SetIntField(objBlp1Sync, fidExtCount, boardStatus.ap1_sync.ext_count);
	  env->SetIntField(objBlp1Sync, fidSyncCount, boardStatus.ap1_sync.sync_count);
	  env->SetIntField(objBlp1Sync, fidSampleOffset, boardStatus.ap1_sync.sample_offset);
	  env->SetIntField(objBlp1Sync, fidSliceCount, boardStatus.ap1_sync.slice_count);
	}
    }
  
  if(fidBlp2Sync != 0)
    {
      jobject objBlp2Sync = env->GetObjectField(status, fidBlp2Sync);
      if(fidExtCount != 0 && fidSyncCount != 0 && fidSampleOffset != 0 && fidSliceCount != 0)
	{
	  env->SetIntField(objBlp2Sync, fidExtCount, boardStatus.ap2_sync.ext_count);
	  env->SetIntField(objBlp2Sync, fidSyncCount, boardStatus.ap2_sync.sync_count);
	  env->SetIntField(objBlp2Sync, fidSampleOffset, boardStatus.ap2_sync.sample_offset);
	  env->SetIntField(objBlp2Sync, fidSliceCount, boardStatus.ap2_sync.slice_count);
	}
    }
  
  if(fidBlp3Sync != 0)
    {
      jobject objBlp3Sync = env->GetObjectField(status, fidBlp3Sync);
      if(fidExtCount != 0 && fidSyncCount != 0 && fidSampleOffset != 0 && fidSliceCount != 0)
	{
	  env->SetIntField(objBlp3Sync, fidExtCount, boardStatus.ap3_sync.ext_count);
	  env->SetIntField(objBlp3Sync, fidSyncCount, boardStatus.ap3_sync.sync_count);
	  env->SetIntField(objBlp3Sync, fidSampleOffset, boardStatus.ap3_sync.sample_offset);
	  env->SetIntField(objBlp3Sync, fidSliceCount, boardStatus.ap3_sync.slice_count);
	}
    }
  
  // RCUStatus: blp0Rcu - blp3Rcu
  jclass clsRCUStatus = env->FindClass("nl/astron/lofar/java/mac/jrsp/RCUStatus");
  jfieldID fidNofOverflowX = env->GetFieldID(clsRCUStatus, "nofOverflowX", "I");
  jfieldID fidNofOverflowY = env->GetFieldID(clsRCUStatus, "nofOverflowY", "I");
  
  if(fidBlp0Rcu != 0)
    {
      jobject objBlp0Rcu = env->GetObjectField(status, fidBlp0Rcu);
      if(fidNofOverflowX != 0 && fidNofOverflowY != 0)
	{
	  env->SetIntField(objBlp0Rcu, fidNofOverflowX, boardStatus.blp0_rcu.nof_overflowx);
	  env->SetIntField(objBlp0Rcu, fidNofOverflowY, boardStatus.blp0_rcu.nof_overflowy);
	}
    }
  
  if(fidBlp1Rcu != 0)
    {
      jobject objBlp1Rcu = env->GetObjectField(status, fidBlp1Rcu);
      if(fidNofOverflowX != 0 && fidNofOverflowY != 0)
	{
	  env->SetIntField(objBlp1Rcu, fidNofOverflowX, boardStatus.blp1_rcu.nof_overflowx);
	  env->SetIntField(objBlp1Rcu, fidNofOverflowY, boardStatus.blp1_rcu.nof_overflowy);
	}
    }
  
  if(fidBlp2Rcu != 0)
    {
      jobject objBlp2Rcu = env->GetObjectField(status, fidBlp2Rcu);
      if(fidNofOverflowX != 0 && fidNofOverflowY != 0)
	{
	  env->SetIntField(objBlp2Rcu, fidNofOverflowX, boardStatus.blp2_rcu.nof_overflowx);
	  env->SetIntField(objBlp2Rcu, fidNofOverflowY, boardStatus.blp2_rcu.nof_overflowy);
	}
    }
  
  if(fidBlp3Rcu != 0)
    {
      jobject objBlp3Rcu = env->GetObjectField(status, fidBlp3Rcu);
      if(fidNofOverflowX != 0 && fidNofOverflowY != 0)
	{
	  env->SetIntField(objBlp3Rcu, fidNofOverflowX, boardStatus.blp3_rcu.nof_overflowx);
	  env->SetIntField(objBlp3Rcu, fidNofOverflowY, boardStatus.blp3_rcu.nof_overflowy);
	}
    }
  
  if(fidCpRdy != 0)
    {
      env->SetBooleanField(status, fidCpRdy, boardStatus.cp_status.rdy);
    }
  if(fidCpErr != 0)
    {
      env->SetBooleanField(status, fidCpErr, boardStatus.cp_status.err);
    }
  if(fidCpFpga != 0)
    {
      env->SetBooleanField(status, fidCpFpga, boardStatus.cp_status.fpga);
    }
  if(fidCpIm != 0)
    {
      env->SetBooleanField(status, fidCpIm, boardStatus.cp_status.im);
    }
  if(fidCpTrig != 0)
    {
      env->SetShortField(status, fidCpTrig, boardStatus.cp_status.trig);
    }
  
  // ADOStatus: blp0AdcOffset - blp3AdcOffset
  jclass clsADOStatus = env->FindClass("nl/astron/lofar/java/mac/jrsp/ADOStatus");
  jfieldID fidAdcOffsetX = env->GetFieldID(clsADOStatus, "adcOffsetX", "I");
  jfieldID fidAdcOffsetY = env->GetFieldID(clsADOStatus, "adcOffsetY", "I");
  
  if(fidBlp0AdcOffset != 0)
    {
      jobject objBlp0AdcOffset = env->GetObjectField(status, fidBlp0AdcOffset);
      if(fidAdcOffsetX != 0 && fidAdcOffsetY != 0)
	{
	  env->SetIntField(objBlp0AdcOffset, fidAdcOffsetX, boardStatus.blp0_adc_offset.adc_offset_x);
	  env->SetIntField(objBlp0AdcOffset, fidAdcOffsetY, boardStatus.blp0_adc_offset.adc_offset_y);
	}
    }
  if(fidBlp1AdcOffset != 0)
    {
      jobject objBlp1AdcOffset = env->GetObjectField(status, fidBlp1AdcOffset);
      if(fidAdcOffsetX != 0 && fidAdcOffsetY != 0)
	{
	  env->SetIntField(objBlp1AdcOffset, fidAdcOffsetX, boardStatus.blp1_adc_offset.adc_offset_x);
	  env->SetIntField(objBlp1AdcOffset, fidAdcOffsetY, boardStatus.blp1_adc_offset.adc_offset_y);
	}
    }
  if(fidBlp2AdcOffset != 0)
    {
      jobject objBlp2AdcOffset = env->GetObjectField(status, fidBlp2AdcOffset);
      if(fidAdcOffsetX != 0 && fidAdcOffsetY != 0)
	{
	  env->SetIntField(objBlp2AdcOffset, fidAdcOffsetX, boardStatus.blp2_adc_offset.adc_offset_x);
	  env->SetIntField(objBlp2AdcOffset, fidAdcOffsetY, boardStatus.blp2_adc_offset.adc_offset_y);
	}
    }
  if(fidBlp3AdcOffset != 0)
    {
      jobject objBlp3AdcOffset = env->GetObjectField(status, fidBlp3AdcOffset);
      if(fidAdcOffsetX != 0 && fidAdcOffsetY != 0)
	{
	  env->SetIntField(objBlp3AdcOffset, fidAdcOffsetX, boardStatus.blp3_adc_offset.adc_offset_x);
	  env->SetIntField(objBlp3AdcOffset, fidAdcOffsetY, boardStatus.blp3_adc_offset.adc_offset_y);
	}
    }
  
  // Free local references. 
  env->DeleteLocalRef(clsStatus);
  
  cout << "Leaving ConvertStatus" << endl; 
  // Return status.
  return status;
}

/**
 * Sets the waveform settings.
 * @param	env
 * @param	obj
 * @param	rcuMask
 * @param	mode
 * @param	frequency
 * @param	amplitude
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_setWaveformSettings(JNIEnv *env , jobject , jint rcuMask, jint mode, jdouble frequency, jshort phase, jint amplitude, jint ptrRSPport)
{
  cout << "Entering setWaveformSettings rcuMask: " << rcuMask << " mode: " << mode << " frequency: " <<
      frequency << " phase: " << phase << " amplitude: " << amplitude << endl;
  jboolean result=false;

  RSPport * IOport = (RSPport*)ptrRSPport;
  try {
    result=IOport->setWaveformSettings(rcuMask, mode, frequency, phase, amplitude);
  } catch (exception &ex) {
    cout << "Error during setWaveformSettings rcuMask: " << rcuMask << " mode: " << mode << " frequency: " <<
      frequency << " phase: " << phase << " amplitude: " << amplitude << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
  cout << "Leaving setWaveformSettings rcuMask: " << rcuMask << " mode: " << mode << " frequency: " <<
      frequency << " phase: " << phase << " amplitude: " << amplitude << endl;
  return result;
}

/**
 * Returns the subband stats.
 * @param	env
 * @param	obj
 * @param	rcuMask
 * @param	ptrRSPport
 * @return			jdoubleArray
 */
JNIEXPORT jdoubleArray JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_getSubbandStats(JNIEnv * env, jobject , jint rcuMask, jint ptrRSPport)
{
  cout << "Entering getSubbandStats rcuMask: " << rcuMask << endl;
  jdoubleArray ret;
  RSPport * IOport = (RSPport *) ptrRSPport;

  try {
    // get a vector with the subband stats
    vector<double> vecData = IOport->getSubbandStats(rcuMask);
    
    // make a jdouble array to return to Java
    ret = env->NewDoubleArray(vecData.size());

    // move vecData elements to ret
    for (uint i = 0; i < vecData.size(); i++)
      {
	env->SetDoubleArrayRegion(ret, i, 1, &vecData[i]);
      }
  } catch (exception &ex) {
    cout << "Error during getSubbandStats rcuMask: " << rcuMask << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }

  cout << "Leaving getSubbandStats rcuMask: " << rcuMask << endl;
  
  // return array
  return ret;
}

/**
 * Returns a array of nl.astron.lofar.java.mac.jrsp.WGRegisterType elements.
 */
JNIEXPORT jobjectArray JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_getWaveformSettings(JNIEnv * env, jobject , jint rcuMask, jint ptrRSPport)
{
  cout << "Entering getWaveformSettings rcuMask: " << rcuMask << endl;
  jobjectArray arrResults;
  // get pointer to RSPport
  RSPport * IOport = (RSPport *) ptrRSPport;

  try {
    // get vector with register types
    vector<struct WGSettings::WGRegisterType> vecData = IOport->getWaveformSettings(rcuMask);
    
    /*
     * JNI part
     * get class, constructor and field identifiers.
     */
    
    // jni: class
    jclass clsWGRegisterType = env->FindClass("nl/astron/lofar/java/mac/jrsp/WGRegisterType");
    if (clsWGRegisterType == NULL) {
      return NULL;
    }
    
    // jni: constructor
    static jmethodID midConstructor = env->GetMethodID(clsWGRegisterType, "<init>", "()V");
    if (midConstructor == NULL) {
      return NULL;
    }
    
    // jni: fields
    jfieldID fidMode = env->GetFieldID(clsWGRegisterType, "mode", "S");
    jfieldID fidPhase = env->GetFieldID(clsWGRegisterType, "phase", "S");
    jfieldID fidNofSamples = env->GetFieldID(clsWGRegisterType, "nofSamples", "I");
    jfieldID fidFrequency = env->GetFieldID(clsWGRegisterType, "frequency", "D");
    jfieldID fidAmplitude = env->GetFieldID(clsWGRegisterType, "amplitude", "J");
    if (fidMode == 0 || fidPhase == 0 || fidNofSamples == 0 || fidFrequency == 0 || fidAmplitude == 0) {
      return NULL;
    }
    
    // construct return array
    arrResults = (jobjectArray) env->NewObjectArray(vecData.size(), clsWGRegisterType, NULL);
    
    // process vector and add itens to array
    for (uint i = 0; i < vecData.size(); i++) {
      // make new (j)WGRegisterType
      jobject temp = env->NewObject(clsWGRegisterType, midConstructor, NULL);
      
      env->SetShortField(temp, fidMode, vecData[i].mode);
      env->SetShortField(temp, fidPhase, vecData[i].phase);
      env->SetIntField(temp, fidNofSamples, vecData[i].nof_samples);
      env->SetDoubleField(temp, fidFrequency, vecData[i].freq);
      env->SetLongField(temp, fidAmplitude, vecData[i].ampl);
      
      // add object to return array
      env->SetObjectArrayElement(arrResults, i, temp);
    }
  } catch (exception &ex) {
    cout << "Error during getWaveformSettings rcuMask: " << rcuMask << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
	
  cout << "Leaving getWaveformSettings rcuMask: " << rcuMask << endl;
  // return array
  return arrResults;
} // getWaveformSettings()

/**
 * Returns the number of connected RCU's.
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_getNrRCUs(JNIEnv *env, jobject , jint ptrRSPport)
{
  cout << "Entering getNrRCUs" << endl;
  jint result;
  // get pointer to RSPport
  RSPport * IOport = (RSPport *) ptrRSPport;
  try {
    result= IOport->getNrRCUs();
  } catch (exception &ex) {
    cout << "Error during getNrRCUs" << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
  cout << "Leaving getNrRCUs" << endl;
  return result;
}

/**
 * Returns the number of connected boards.
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_getNrRSPBoards(JNIEnv *env, jobject , jint ptrRSPport)
{
  cout << "Entering getNrRSPBoards" << endl;
  jint result;
  // get pointer to RSPport
  RSPport * IOport = (RSPport *) ptrRSPport;
  try {
    result=IOport->getNrRSPboards();
  } catch (exception &ex) {
    cout << "Error during getNrRSPBoards" << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
  cout << "Leaving getNrRSPBoards" << endl;
  return result;
}

/**
 * Returns the maximum number of boards that could be connected.
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_getMaxRSPBoards(JNIEnv *env , jobject , jint ptrRSPport)
{
  cout << "Entering getMaxRSPBoards" << endl;
  jint result;
  // get pointer to RSPport
  RSPport * IOport = (RSPport *) ptrRSPport;
  try {
    result=IOport->getMaxRSPboards();
  } catch (exception &ex) {
    cout << "Error during getMaxRSPBoards" << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
  cout << "Leaving getMaxRSPBoards" << endl;
  return result;
}

/**
 * Sets filter.
 * @param env
 * @param obj
 * @param rcuMask
 * @param filterNr    The number of the filter to apply.
 * @param ptrRSPport  Pointer to the RSPport instance.
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_setFilter(JNIEnv *env , jobject, jint rcuMask, jint filterNr, jint ptrRSPport)
{
  cout << "Entering setFilterrcuMask: " << rcuMask << endl;
  jboolean result=false;
  // get pointer to RSPport
  RSPport * IOport = (RSPport *) ptrRSPport;
  try {
    result=IOport->setFilter(rcuMask, filterNr);
  } catch (exception &ex) {
    cout << "Error during setFilterrcuMask: " << rcuMask << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
  cout << "Leaving setFilterrcuMask: " << rcuMask << endl;
  return result;
}

/**
 * Sends clear signal to the selected board.
 * @param env
 * @param obj
 * @param rcuMask     A mask holding the selected board.
 * @param ptrRSPport  Pointer to the RSPport instance
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_sendClear(JNIEnv *env, jobject, jint rcuMask, jint ptrRSPport)
{
  cout << "Entering sendClear rcuMask: " << rcuMask << endl;
  jboolean result=false;
  // get pointer to RSPport
  RSPport * IOport = (RSPport *) ptrRSPport;
  try {
    result=IOport->sendClear(rcuMask);
  } catch (exception &ex) {
    cout << "Error during sendClear rcuMask: " << rcuMask << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
  cout << "Leaving sendClear rcuMask: " << rcuMask << endl;
  return result;
}

/**
 * Sends reset signal to the selected board.
 * @param env
 * @param obj
 * @param rcuMask     A mask holding the selected board.
 * @param ptrRSPport  Pointer to the RSPport instance
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_sendReset(JNIEnv *env , jobject, jint rcuMask, jint ptrRSPport)
{
  
  cout << "Entering sendReset rcuMask: " << rcuMask << endl;
  jboolean result=false;
  // get pointer to RSPport
  RSPport * IOport = (RSPport *) ptrRSPport;
  try {
    result=IOport->sendReset(rcuMask);
  } catch (exception &ex) {
    cout << "Error during sendReset rcuMask: " << rcuMask << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
  cout << "Leaving sendReset rcuMask: " << rcuMask << endl;
  return result;
}

/**
 * Sends sync signal to the selected board.
 * @param env
 * @param obj
 * @param rcuMask     A mask holding the selected board.
 * @param ptrRSPport  Pointer to the RSPport instance
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_sendSync(JNIEnv *env , jobject , jint rcuMask, jint ptrRSPport)
{
  cout << "Entering sendSync rcuMask: " << rcuMask << endl;
  jboolean result=false;
  // get pointer to RSPport
  RSPport * IOport = (RSPport *) ptrRSPport;
  try {
    result=IOport->sendSync(rcuMask);
  } catch (exception &ex) {
    cout << "Error during sendSync rcuMask: " << rcuMask << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }
  cout << "Leaving sendSync rcuMask: " << rcuMask << endl;
  return result;
}

/**
 * @param env
 * @param obj
 * @param mask
 * @param ptrRSPport
 */
JNIEXPORT jdoubleArray JNICALL Java_nl_astron_lofar_java_mac_jrsp_Board_getBeamletStats(JNIEnv *env , jobject , jint rcuMask, jint ptrRSPport)
{
  cout << "entering getBeamletStats rcuMask: " << rcuMask << endl;
  jdoubleArray ret;
  RSPport * IOport = (RSPport *) ptrRSPport;
  
  try {
    // get a vector with the subband stats
    vector<double> vecData = IOport->getBeamletStats(rcuMask);
    
    // make a jdouble array to return to Java
    jdoubleArray ret = env->NewDoubleArray(vecData.size());
    
    // move vecData elements to ret
    for (uint i = 0; i < vecData.size(); i++)
      {
	env->SetDoubleArrayRegion(ret, i, 1, &vecData[i]);
      }
  } catch (exception &ex) {
    cout << "Error during getBeamletStats rcuMask: " << rcuMask << endl;
    cout << "What: " << ex.what() << endl;
    env->ThrowNew(env->FindClass("java/lang/Exception"), ex.what());
  }

  
  cout << "leaving getBeamletStats rcuMask: " << rcuMask << endl;
  // return array
  return ret;
}
