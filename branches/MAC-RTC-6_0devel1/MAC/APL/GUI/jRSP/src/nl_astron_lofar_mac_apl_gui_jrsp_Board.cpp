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
	jmethodID midSetVoltage1V2 = env->GetMethodID(clsStatus, "setVoltage1V2", "(I)V");
	jmethodID midSetVoltage2V5 = env->GetMethodID(clsStatus, "setVoltage2V5", "(I)V");
	jmethodID midSetVoltage3V3 = env->GetMethodID(clsStatus, "setVoltage3V3", "(I)V");
	jmethodID midSetPcbTemp = env->GetMethodID(clsStatus, "setPcbTemp", "(I)V");
	jmethodID midSetBpTemp = env->GetMethodID(clsStatus, "setBpTemp", "(I)V");
	jmethodID midSetAp0Temp = env->GetMethodID(clsStatus, "setAp0Temp", "(I)V");
	jmethodID midSetAp1Temp = env->GetMethodID(clsStatus, "setAp1Temp", "(I)V");
	jmethodID midSetAp2Temp = env->GetMethodID(clsStatus, "setAp2Temp", "(I)V");
	jmethodID midSetAp3Temp = env->GetMethodID(clsStatus, "setAp3Temp", "(I)V");
	jmethodID midSetBpClock = env->GetMethodID(clsStatus, "setBpClock", "(I)V");
	jmethodID midSetNofFrames = env->GetMethodID(clsStatus, "setNofFrames", "(I)V");
	jmethodID midSetNofErrors = env->GetMethodID(clsStatus, "setNofErrors", "(I)V");
	jmethodID midSetLastError = env->GetMethodID(clsStatus, "setLastError", "(I)V");
        jmethodID midSetSeqnr = env->GetMethodID(clsStatus, "setSeqnr", "(I)V");
        jmethodID midSetError = env->GetMethodID(clsStatus, "setError", "(I)V");
        jmethodID midSetInterface = env->GetMethodID(clsStatus, "setInterface", "(I)V");
        jmethodID midSetMode = env->GetMethodID(clsStatus, "setMode", "(I)V");
        jmethodID midSetRiErrors = env->GetMethodID(clsStatus, "setRiErrors", "(I)V");
        jmethodID midSetRcuxErrors = env->GetMethodID(clsStatus, "setRcuxErrors", "(I)V");
        jmethodID midSetLcuErrors = env->GetMethodID(clsStatus, "setLcuErrors", "(I)V");
        jmethodID midSetCepErrors = env->GetMethodID(clsStatus, "setCepErrors", "(I)V");
        jmethodID midSetSerdesErrors = env->GetMethodID(clsStatus, "setSerdesErrors", "(I)V");
        jmethodID midSetAp0RiErrors = env->GetMethodID(clsStatus, "setAp0RiErrors", "(I)V");
        jmethodID midSetAp1RiErrors = env->GetMethodID(clsStatus, "setAp1RiErrors", "(I)V");
        jmethodID midSetAp2RiErrors = env->GetMethodID(clsStatus, "setAp2RiErrors", "(I)V");
        jmethodID midSetAp3RiErrors = env->GetMethodID(clsStatus, "setAp3RiErrors", "(I)V");
        jmethodID midSetBlp0Sync = env->GetMethodID(clsStatus, "setBlp0Sync", "(I)V");
        jmethodID midSetBlp1Sync = env->GetMethodID(clsStatus, "setBlp1Sync", "(I)V");
        jmethodID midSetBlp2Sync = env->GetMethodID(clsStatus, "setBlp2Sync", "(I)V");
        jmethodID midSetBlp3Sync = env->GetMethodID(clsStatus, "setBlp3Sync", "(I)V");
        jmethodID midSetBlp0Rcu = env->GetMethodID(clsStatus, "setBlp0Rcu", "(I)V");
        jmethodID midSetBlp1Rcu = env->GetMethodID(clsStatus, "setBlp1Rcu", "(I)V");
        jmethodID midSetBlp2Rcu = env->GetMethodID(clsStatus, "setBlp2Rcu", "(I)V");
        jmethodID midSetBlp3Rcu = env->GetMethodID(clsStatus, "setBlp3Rcu", "(I)V");
        jmethodID midSetCpStatus = env->GetMethodID(clsStatus, "setCpStatus", "(I)V");
        jmethodID midSetBlp0AdcOffset = env->GetMethodID(clsStatus, "setBlp0AdcOffset", "(I)V");
        jmethodID midSetBlp1AdcOffset = env->GetMethodID(clsStatus, "setBlp1AdcOffset", "(I)V");
        jmethodID midSetBlp2AdcOffset = env->GetMethodID(clsStatus, "setBlp2AdcOffset", "(I)V");
        jmethodID midSetBlp3AdcOffset = env->GetMethodID(clsStatus, "setBlp3AdcOffset", "(I)V");

	jint testData = 1;

	if(midSetVoltage1V2 != 0)
	{
		env->CallVoidMethod(status, midSetVoltage1V2, testData);
	}
	if(midSetVoltage2V5 != 0)
	{
		env->CallVoidMethod(status, midSetVoltage2V5, testData);
	}
	if(midSetVoltage3V3 != 0)
	{
		env->CallVoidMethod(status, midSetVoltage3V3, testData);
	}
	if(midSetPcbTemp != 0)
	{
		env->CallVoidMethod(status, midSetPcbTemp, testData);
	}
	if(midSetBpTemp != 0)
	{
		env->CallVoidMethod(status, midSetBpTemp, testData);
	}
	if(midSetAp0Temp != 0)
	{
		env->CallVoidMethod(status, midSetAp0Temp, testData);
	}
	if(midSetAp1Temp != 0)
	{
		env->CallVoidMethod(status, midSetAp1Temp, testData);
	}
	if(midSetAp2Temp != 0)
	{
		env->CallVoidMethod(status, midSetAp2Temp, testData);
	}
	if(midSetAp3Temp != 0)
	{
		env->CallVoidMethod(status, midSetAp3Temp, testData);
	}
	if(midSetBpClock != 0)
	{
		env->CallVoidMethod(status, midSetBpClock, testData);
	}
	if(midSetNofFrames != 0)
	{
		env->CallVoidMethod(status, midSetNofFrames, testData);
	}
	if(midSetNofErrors != 0)
	{
		env->CallVoidMethod(status, midSetNofErrors, testData);
	}
	if(midSetLastError != 0)
        {
                env->CallVoidMethod(status, midSetLastError, testData);
        }
        if(midSetSeqnr != 0)
        {
                env->CallVoidMethod(status, midSetSeqnr, testData);
        }
        if(midSetError != 0)
        {
                env->CallVoidMethod(status, midSetError, testData);
        }
        if(midSetInterface != 0)
        {
                env->CallVoidMethod(status, midSetInterface, testData);
        }
        if(midSetMode != 0)
        {
                env->CallVoidMethod(status, midSetMode, testData);
        }
        if(midSetRiErrors != 0)
        {
                env->CallVoidMethod(status, midSetRiErrors, testData);
        }
        if(midSetRcuxErrors != 0)
        {
                env->CallVoidMethod(status, midSetRcuxErrors, testData);
        }
        if(midSetLcuErrors != 0)
        {
                env->CallVoidMethod(status, midSetLcuErrors, testData);
        }
        if(midSetCepErrors != 0)
        {
                env->CallVoidMethod(status, midSetCepErrors, testData);
        }
        if(midSetSerdesErrors != 0)
        {
                env->CallVoidMethod(status, midSetSerdesErrors, testData);
        }
        if(midSetAp0RiErrors != 0)
        {
                env->CallVoidMethod(status, midSetAp0RiErrors, testData);
        }
        if(midSetAp1RiErrors != 0)
        {
                env->CallVoidMethod(status, midSetAp1RiErrors, testData);
        }
        if(midSetAp2RiErrors != 0)
        {
                env->CallVoidMethod(status, midSetAp2RiErrors, testData);
        }
        if(midSetAp3RiErrors != 0)
        {
                env->CallVoidMethod(status, midSetAp3RiErrors, testData);
        }
        if(midSetBlp0Sync != 0)
        {
                env->CallVoidMethod(status, midSetBlp0Sync, testData);
        }
        if(midSetBlp1Sync != 0)
        {
                env->CallVoidMethod(status, midSetBlp1Sync, testData);
        }
        if(midSetBlp2Sync != 0)
        {
                env->CallVoidMethod(status, midSetBlp2Sync, testData);
        }
        if(midSetBlp3Sync != 0)
        {
                env->CallVoidMethod(status, midSetBlp3Sync, testData);
        }
        if(midSetBlp0Rcu != 0)
        {
                env->CallVoidMethod(status, midSetBlp0Rcu, testData);
        }
        if(midSetBlp1Rcu != 0)
        {
                env->CallVoidMethod(status, midSetBlp1Rcu, testData);
        }
        if(midSetBlp2Rcu != 0)
        {
                env->CallVoidMethod(status, midSetBlp2Rcu, testData);
        }
        if(midSetBlp3Rcu != 0)
        {
                env->CallVoidMethod(status, midSetBlp3Rcu, testData);
        }
        if(midSetCpStatus != 0)
        {
                env->CallVoidMethod(status, midSetCpStatus, testData);
        }
        if(midSetBlp0AdcOffset != 0)
        {
                env->CallVoidMethod(status, midSetBlp0AdcOffset, testData);
        }
        if(midSetBlp1AdcOffset != 0)
        {
                env->CallVoidMethod(status, midSetBlp1AdcOffset, testData);
        }
        if(midSetBlp2AdcOffset != 0)
        {
                env->CallVoidMethod(status, midSetBlp2AdcOffset, testData);
        }
        if(midSetBlp3AdcOffset != 0)
        {
                env->CallVoidMethod(status, midSetBlp3AdcOffset, testData);
        }
}
