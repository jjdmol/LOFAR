// Always include lofar_config.h
#include <lofar_config.h>

// Includes
#include <jRSP/nl_astron_lofar_mac_apl_gui_jrsp_Board.h>
#include <RSP/RSPport.h>
using namespace LOFAR;
using namespace LOFAR::RSP;

// Define function's
jobject ConvertBoardStatus(JNIEnv*, BoardStatus&);
BoardStatus GetDummyBoardStatus(); // Function for testing.

/**
 * Implementation of the JNI method declared in the java file (nl.astron.lofar.mac.apl.gui.jrsp.Board).
 * This function fills a BoardStatus object, that is returned by this method, with data from RSPIO.
 * @param	env		The Java environment interface pointer.
 * @param	obj		The "this" pointer.
 * @param	hostname	The name of the RSPBoard that will be connected.
 * @return 	status		A array of StatusBoard class instances filled with information.
 */
JNIEXPORT jobjectArray JNICALL Java_nl_astron_lofar_mac_apl_gui_jrsp_Board_retrieveStatus(JNIEnv * env, jobject obj, jstring hostname) 
{
	// Convert the jstring hostname to a C++ string.
	const char * charsHostname = env->GetStringUTFChars(hostname, 0);
	string strHostname (charsHostname);
	
	// Use RSPport to get a vector with BoardStatus.
	vector<BoardStatus> vecBoardStatus;

	if(0)
	{
		RSPport IOport(strHostname);
		uint32 rcuMask = 0;
		vecBoardStatus = IOport.getBoardStatus(rcuMask);
	}
	else
	{
		// Add dummy BoardStatus to vector.
		vecBoardStatus.push_back(GetDummyBoardStatus());
	}
	
	// The jobjectArray that is going to be returned.
	jobjectArray arrBoardStatus = (jobjectArray)env->NewObjectArray(vecBoardStatus.size(), env->FindClass("nl/astron/lofar/mac/apl/gui/jrsp/BoardStatus"), NULL);

	for(uint i=0; i<vecBoardStatus.size(); i++)
	{
		env->SetObjectArrayElement(arrBoardStatus, i, ConvertBoardStatus(env, vecBoardStatus[i]));
	}

	// Free local references.	
	env->ReleaseStringUTFChars(hostname, charsHostname);
	
	return arrBoardStatus;
}

/**
 * This method converts an C++ BoardStatus to a Java BoardStatus.
 * @param	BoardStatus	C++ BoardStatus
 * @return 	jobject		Java BoardStatus
 */
jobject ConvertBoardStatus(JNIEnv * env, BoardStatus &boardStatus)
{
	// TODO: Deze code buiten deze functie plaatsen zodat het niet vaker dan een keer aangeroepen hoeft te worden.
	// Get a reference to the class of the status (BoardStatus).
	jclass clsStatus = env->FindClass("nl/astron/lofar/mac/apl/gui/jrsp/BoardStatus");
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
        jfieldID fidPcbTemp = env->GetFieldID(clsStatus, "pcbTemp", "I");
        jfieldID fidBpTemp = env->GetFieldID(clsStatus, "bpTemp", "I");
        jfieldID fidAp0Temp = env->GetFieldID(clsStatus, "ap0Temp", "I");
        jfieldID fidAp1Temp = env->GetFieldID(clsStatus, "ap1Temp", "I");
        jfieldID fidAp2Temp = env->GetFieldID(clsStatus, "ap2Temp", "I");
        jfieldID fidAp3Temp = env->GetFieldID(clsStatus, "ap3Temp", "I");
        jfieldID fidBpClock = env->GetFieldID(clsStatus, "bpClock", "I");
        jfieldID fidNofFrames = env->GetFieldID(clsStatus, "nofFrames", "I");
        jfieldID fidNofErrors = env->GetFieldID(clsStatus, "nofErrors", "I");
        jfieldID fidLastError = env->GetFieldID(clsStatus, "lastError", "I");
        jfieldID fidSeqNr = env->GetFieldID(clsStatus, "seqNr", "I");
        jfieldID fidError = env->GetFieldID(clsStatus, "error", "I");
        jfieldID fidIfUnderTest = env->GetFieldID(clsStatus, "ifUnderTest", "I");
        jfieldID fidMode = env->GetFieldID(clsStatus, "mode", "I");
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
	jfieldID fidBlp0Sync = env->GetFieldID(clsStatus, "blp0Sync", "Lnl/astron/lofar/mac/apl/gui/jrsp/SyncStatus;");
	jfieldID fidBlp1Sync = env->GetFieldID(clsStatus, "blp1Sync", "Lnl/astron/lofar/mac/apl/gui/jrsp/SyncStatus;");
	jfieldID fidBlp2Sync = env->GetFieldID(clsStatus, "blp2Sync", "Lnl/astron/lofar/mac/apl/gui/jrsp/SyncStatus;");
	jfieldID fidBlp3Sync = env->GetFieldID(clsStatus, "blp3Sync", "Lnl/astron/lofar/mac/apl/gui/jrsp/SyncStatus;");
        jfieldID fidBlp0Rcu = env->GetFieldID(clsStatus, "blp0Rcu", "Lnl/astron/lofar/mac/apl/gui/jrsp/RCUStatus;");
        jfieldID fidBlp1Rcu = env->GetFieldID(clsStatus, "blp1Rcu", "Lnl/astron/lofar/mac/apl/gui/jrsp/RCUStatus;");
        jfieldID fidBlp2Rcu = env->GetFieldID(clsStatus, "blp2Rcu", "Lnl/astron/lofar/mac/apl/gui/jrsp/RCUStatus;");
        jfieldID fidBlp3Rcu = env->GetFieldID(clsStatus, "blp3Rcu", "Lnl/astron/lofar/mac/apl/gui/jrsp/RCUStatus;");
	jfieldID fidCpRdy = env->GetFieldID(clsStatus, "cpRdy", "I");
	jfieldID fidCpErr = env->GetFieldID(clsStatus, "cpErr", "I");
	jfieldID fidCpFpga = env->GetFieldID(clsStatus, "cpFpga", "I");
	jfieldID fidCpIm = env->GetFieldID(clsStatus, "cpIm", "I");
	jfieldID fidCpTrig = env->GetFieldID(clsStatus, "cpTrig", "I");
	jfieldID fidBlp0AdcOffset = env->GetFieldID(clsStatus, "blp0AdcOffset", "Lnl/astron/lofar/mac/apl/gui/jrsp/ADOStatus;");
	jfieldID fidBlp1AdcOffset = env->GetFieldID(clsStatus, "blp1AdcOffset", "Lnl/astron/lofar/mac/apl/gui/jrsp/ADOStatus;");
	jfieldID fidBlp2AdcOffset = env->GetFieldID(clsStatus, "blp2AdcOffset", "Lnl/astron/lofar/mac/apl/gui/jrsp/ADOStatus;");
	jfieldID fidBlp3AdcOffset = env->GetFieldID(clsStatus, "blp3AdcOffset", "Lnl/astron/lofar/mac/apl/gui/jrsp/ADOStatus;");

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
                env->SetIntField(status, fidPcbTemp, boardStatus.rsp.pcb_temp);
        }
        if(fidBpTemp != 0)
        {
                env->SetIntField(status, fidBpTemp, boardStatus.rsp.bp_temp);
        }
        if(fidAp0Temp != 0)
        {
                env->SetIntField(status, fidAp0Temp, boardStatus.rsp.ap0_temp);
        }
        if(fidAp1Temp != 0)
        {
                env->SetIntField(status, fidAp1Temp, boardStatus.rsp.ap1_temp);
        }
        if(fidAp2Temp != 0)
        {
                env->SetIntField(status, fidAp2Temp, boardStatus.rsp.ap2_temp);
        }
        if(fidAp3Temp != 0)
        {
                env->SetIntField(status, fidAp3Temp, boardStatus.rsp.ap3_temp);
        }
        if(fidBpClock != 0)
        {
                env->SetIntField(status, fidBpClock, boardStatus.rsp.bp_clock);
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
                env->SetIntField(status, fidLastError, boardStatus.eth.last_error);
        }
        if(fidSeqNr != 0)
        {
                env->SetIntField(status, fidSeqNr, boardStatus.mep.seqnr);
        }
        if(fidError != 0)
        {
                env->SetIntField(status, fidError, boardStatus.mep.error);
        }
        if(fidIfUnderTest != 0)
        {
                env->SetIntField(status, fidIfUnderTest, boardStatus.diag.interface);
        }
        if(fidMode != 0)
        {
                env->SetIntField(status, fidMode, boardStatus.diag.mode);
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
	jclass clsSyncStatus = env->FindClass("nl/astron/lofar/mac/apl/gui/jrsp/SyncStatus");
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
	jclass clsRCUStatus = env->FindClass("nl/astron/lofar/mac/apl/gui/jrsp/RCUStatus");
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
		env->SetIntField(status, fidCpRdy, boardStatus.cp_status.rdy);
	}
	if(fidCpErr != 0)
	{
		env->SetIntField(status, fidCpErr, boardStatus.cp_status.err);
	}
	if(fidCpFpga != 0)
	{
		env->SetIntField(status, fidCpFpga, boardStatus.cp_status.fpga);
	}
	if(fidCpIm != 0)
	{
		env->SetIntField(status, fidCpIm, boardStatus.cp_status.im);
	}
	if(fidCpTrig != 0)
	{
		env->SetIntField(status, fidCpTrig, boardStatus.cp_status.trig);
	}

	// ADOStatus: blp0AdcOffset - blp3AdcOffset
	jclass clsADOStatus = env->FindClass("nl/astron/lofar/mac/apl/gui/jrsp/ADOStatus");
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
	
	// Return status.
	return status;
}

BoardStatus GetDummyBoardStatus()
{
	BoardStatus bs;

	bs.rsp.voltage_1_2 = 3;
	bs.rsp.voltage_2_5 = 3;
	bs.rsp.voltage_3_3 = 3;
	bs.rsp.pcb_temp = 3;
	bs.rsp.bp_temp = 3;
	bs.rsp.ap0_temp = 3;
	bs.rsp.ap1_temp = 3;
	bs.rsp.ap2_temp = 3;
	bs.rsp.ap3_temp = 3;
	bs.rsp.bp_clock = 3;

	bs.eth.nof_frames = 3;
	bs.eth.nof_errors = 3;
	bs.eth.last_error = 3;
	
	bs.mep.seqnr = 3;
	bs.mep.error = 3;

	bs.diag.interface = 3;
	bs.diag.mode = 3;
	bs.diag.ri_errors = 3;
	bs.diag.rcux_errors = 3;
	bs.diag.rcuy_errors = 3;
	bs.diag.lcu_errors = 3;
	bs.diag.cep_errors = 3;
	bs.diag.serdes_errors = 3;
	bs.diag.ap0_ri_errors = 3;
	bs.diag.ap1_ri_errors = 3;
	bs.diag.ap2_ri_errors = 3;
	bs.diag.ap3_ri_errors = 3;

	bs.ap0_sync.ext_count = 3;
	bs.ap0_sync.sync_count = 3;
	bs.ap0_sync.sample_offset = 3;
	bs.ap0_sync.slice_count = 3;
	bs.ap1_sync.ext_count = 3;
	bs.ap1_sync.sync_count = 3;
	bs.ap1_sync.sample_offset = 3;
	bs.ap1_sync.slice_count = 3;
	bs.ap2_sync.ext_count = 3;
	bs.ap2_sync.sync_count = 3;
	bs.ap2_sync.sample_offset = 3;
	bs.ap2_sync.slice_count = 3;
	bs.ap3_sync.ext_count = 3;
	bs.ap3_sync.sync_count = 3;
	bs.ap3_sync.sample_offset = 3;
	bs.ap3_sync.slice_count = 3;

	bs.blp0_rcu.nof_overflowx = 3;
	bs.blp0_rcu.nof_overflowy = 3;
	bs.blp1_rcu.nof_overflowx = 3;
	bs.blp1_rcu.nof_overflowy = 3;
	bs.blp2_rcu.nof_overflowx = 3;
	bs.blp2_rcu.nof_overflowy = 3;
	bs.blp3_rcu.nof_overflowx = 3;
	bs.blp3_rcu.nof_overflowy = 3;

	bs.cp_status.rdy = 1;
	bs.cp_status.err = 1;
	bs.cp_status.fpga = 1;
	bs.cp_status.im = 1;
	bs.cp_status.trig = 3;

	bs.blp0_adc_offset.adc_offset_x = 3;
	bs.blp0_adc_offset.adc_offset_y = 3;
	bs.blp1_adc_offset.adc_offset_x = 3;
	bs.blp1_adc_offset.adc_offset_y = 3;
	bs.blp2_adc_offset.adc_offset_x = 3;
	bs.blp2_adc_offset.adc_offset_y = 3;
	bs.blp3_adc_offset.adc_offset_x = 3;
	bs.blp3_adc_offset.adc_offset_y = 3;
	
	return bs;
}
