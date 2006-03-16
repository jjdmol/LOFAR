#include "nl_astron_lofar_mac_apl_gui_jrsp_Board.h"

/**
 * Implementation of the JNI method declared in the java file (nl.astron.lofar.mac.apl.gui.jrsp.Board). * This function fills the StatusBoard object, that is passed with this method, with data from RSPIO.
 * @param	env	The Java environment
 * @param	board	The board class object that called this function.
 * @param	status	A StatusBoard object that is passed with this function.
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_mac_apl_gui_jrsp_Board_retrieveStatus(JNIEnv * env, jobject board, jobject status)
{
	// Get a refrence to the class of status (StatusBoard).
	jclass clsStatus = env->GetObjectClass(status);

	// Get method identifiers for the methods of the StatusBoard class.
	jfieldID fidVoltage1V2 = env->GetFieldID(clsStatus, "voltage1V2", "I");
        jfieldID fidVoltage2V5 = env->GetFieldID(clsStatus, "voltage2V5", "I");
        jfieldID fidVoltage3V3 = env->GetFieldID(clsStatus, "voltage3V3", "I");
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
        jfieldID fidLcuErrors = env->GetFieldID(clsStatus, "lcuErrors", "I");
        jfieldID fidCepErrors = env->GetFieldID(clsStatus, "cepErrors", "I");
        jfieldID fidSerdesErrors = env->GetFieldID(clsStatus, "serdesErrors", "I");
        jfieldID fidAp0RiErrors = env->GetFieldID(clsStatus, "ap0RiErrors", "I");
        jfieldID fidAp1RiErrors = env->GetFieldID(clsStatus, "ap1RiErrors", "I");
        jfieldID fidAp2RiErrors = env->GetFieldID(clsStatus, "ap2RiErrors", "I");
        jfieldID fidAp3RiErrors = env->GetFieldID(clsStatus, "ap3RiErrors", "I");
        jfieldID fidBlp0Sync = env->GetFieldID(clsStatus, "blp0Sync", "I");
        jfieldID fidBlp1Sync = env->GetFieldID(clsStatus, "blp1Sync", "I");
        jfieldID fidBlp2Sync = env->GetFieldID(clsStatus, "blp2Sync", "I");
        jfieldID fidBlp3Sync = env->GetFieldID(clsStatus, "blp3Sync", "I");
        jfieldID fidBlp0Rcu = env->GetFieldID(clsStatus, "blp0Rcu", "I");
        jfieldID fidBlp1Rcu = env->GetFieldID(clsStatus, "blp1Rcu", "I");
        jfieldID fidBlp2Rcu = env->GetFieldID(clsStatus, "blp2Rcu", "I");
        jfieldID fidBlp3Rcu = env->GetFieldID(clsStatus, "blp3Rcu", "I");
        jfieldID fidCpStatus = env->GetFieldID(clsStatus, "cpStatus", "I");
        jfieldID fidBlp0AdcOffset = env->GetFieldID(clsStatus, "blp0AdcOffset", "I");
        jfieldID fidBlp1AdcOffset = env->GetFieldID(clsStatus, "blp1AdcOffset", "I");
        jfieldID fidBlp2AdcOffset = env->GetFieldID(clsStatus, "blp2AdcOffset", "I");
        jfieldID fidBlp3AdcOffset = env->GetFieldID(clsStatus, "blp3AdcOffset", "I");

	jint testData = 2;

	// Access field and fill them
        if(fidVoltage1V2 != 0)
        {
                env->SetIntField(status, fidVoltage1V2, testData);
        }
        if(fidVoltage2V5 != 0)
        {
                env->SetIntField(status, fidVoltage2V5, testData);
        }
        if(fidVoltage3V3 != 0)
        {
                env->SetIntField(status, fidVoltage3V3, testData);
        }
        if(fidPcbTemp != 0)
        {
                env->SetIntField(status, fidPcbTemp, testData);
        }
        if(fidBpTemp != 0)
        {
                env->SetIntField(status, fidBpTemp, testData);
        }
        if(fidAp0Temp != 0)
        {
                env->SetIntField(status, fidAp0Temp, testData);
        }
        if(fidAp1Temp != 0)
        {
                env->SetIntField(status, fidAp1Temp, testData);
        }
        if(fidAp2Temp != 0)
        {
                env->SetIntField(status, fidAp2Temp, testData);
        }
        if(fidAp3Temp != 0)
        {
                env->SetIntField(status, fidAp3Temp, testData);
        }
        if(fidBpClock != 0)
        {
                env->SetIntField(status, fidBpClock, testData);
        }
        if(fidNofFrames != 0)
        {
                env->SetIntField(status, fidNofFrames, testData);
        }
        if(fidNofErrors != 0)
        {
                env->SetIntField(status, fidNofErrors, testData);
        }
        if(fidLastError != 0)
        {
                env->SetIntField(status, fidLastError, testData);
        }
        if(fidSeqNr != 0)
        {
                env->SetIntField(status, fidSeqNr, testData);
        }
        if(fidError != 0)
        {
                env->SetIntField(status, fidError, testData);
        }
        if(fidIfUnderTest != 0)
        {
                env->SetIntField(status, fidIfUnderTest, testData);
        }
        if(fidMode != 0)
        {
                env->SetIntField(status, fidMode, testData);
        }
        if(fidRiErrors != 0)
        {
                env->SetIntField(status, fidRiErrors, testData);
        }
        if(fidRcuxErrors != 0)
        {
                env->SetIntField(status, fidRcuxErrors, testData);
        }
        if(fidLcuErrors != 0)
        {
                env->SetIntField(status, fidLcuErrors, testData);
        }
        if(fidCepErrors != 0)
        {
                env->SetIntField(status, fidCepErrors, testData);
        }
        if(fidSerdesErrors != 0)
        {
                env->SetIntField(status, fidSerdesErrors, testData);
        }
        if(fidAp0RiErrors != 0)
        {
                env->SetIntField(status, fidAp0RiErrors, testData);
        }
        if(fidAp1RiErrors != 0)
        {
                env->SetIntField(status, fidAp1RiErrors, testData);
        }
        if(fidAp2RiErrors != 0)
        {
                env->SetIntField(status, fidAp2RiErrors, testData);
        }
        if(fidAp3RiErrors != 0)
        {
                env->SetIntField(status, fidAp3RiErrors, testData);
        }
        if(fidBlp0Sync != 0)
        {
                env->SetIntField(status, fidBlp0Sync, testData);
        }
        if(fidBlp1Sync != 0)
        {
                env->SetIntField(status, fidBlp1Sync, testData);
        }
        if(fidBlp2Sync != 0)
        {
                env->SetIntField(status, fidBlp2Sync, testData);
        }
        if(fidBlp3Sync != 0)
        {
                env->SetIntField(status, fidBlp3Sync, testData);
        }
        if(fidBlp0Rcu != 0)
        {
                env->SetIntField(status, fidBlp0Rcu, testData);
        }
        if(fidBlp1Rcu != 0)
        {
                env->SetIntField(status, fidBlp1Rcu, testData);
        }
        if(fidBlp2Rcu != 0)
        {
                env->SetIntField(status, fidBlp2Rcu, testData);
        }
        if(fidBlp3Rcu != 0)
        {
                env->SetIntField(status, fidBlp3Rcu, testData);
        }
        if(fidCpStatus != 0)
        {
                env->SetIntField(status, fidCpStatus, testData);
        }
        if(fidBlp0AdcOffset != 0)
        {
                env->SetIntField(status, fidBlp0AdcOffset, testData);
        }
        if(fidBlp1AdcOffset != 0)
        {
                env->SetIntField(status, fidBlp1AdcOffset, testData);
        }
        if(fidBlp2AdcOffset != 0)
        {
                env->SetIntField(status, fidBlp2AdcOffset, testData);
        }
        if(fidBlp3AdcOffset != 0)
        {
                env->SetIntField(status, fidBlp3AdcOffset, testData);
        }
}
