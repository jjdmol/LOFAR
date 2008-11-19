// This file was generated by create_db_files v1.0 on Tue Nov 18 15:00:28 UTC 2008

#ifndef LOFAR_DEPLOYMENT_PVSSDATAPOINTS_H
#define LOFAR_DEPLOYMENT_PVSSDATAPOINTS_H
// process
#define	PN_FSM_PROCESSID	"process.processID"
#define	PN_FSM_START_TIME	"process.startTime"
#define	PN_FSM_STOP_TIME	"process.stopTime"
#define	PN_FSM_LOG_MSG	"process.logMsg"
#define	PN_FSM_ERROR	"process.error"
#define	PN_FSM_CURRENT_ACTION	"process.currentAction"
// object
#define	PN_OBJ_STATE	"object.state"
#define	PN_OBJ_CHILD_STATE	"object.childState"
#define	PN_OBJ_MESSAGE	"object.message"
#define	PN_OBJ_LEAF	"object.leaf"

// Station
#define PSN_STATION	"LOFAR_PIC_@ring@_@station@"
#define PST_STATION	"Station"
#define PN_STS_POWER48_ON	"power48On"
#define PN_STS_POWER220_ON	"power220On"

// Station
#define PSN_STATION	"LOFAR_PermSW_@ring@_@station@"
#define PST_STATION	"Station"
#define PN_STS_POWER48_ON	"power48On"
#define PN_STS_POWER220_ON	"power220On"

// MACScheduler
#define PSN_MAC_SCHEDULER	"LOFAR_PermSW_MACScheduler"
#define PST_MAC_SCHEDULER	"MACScheduler"
#define PN_MS_PLANNED_OBSERVATIONS	"plannedObservations"
#define PN_MS_ACTIVE_OBSERVATIONS	"activeObservations"
#define PN_MS_FINISHED_OBSERVATIONS	"finishedObservations"
#define PN_MS_OTDB_CONNECTED	"OTDB.connected"
#define PN_MS_OTDB_LAST_POLL	"OTDB.lastPoll"
#define PN_MS_OTDB_POLLINTERVAL	"OTDB.pollinterval"

// Observation
#define PSN_OBSERVATION	"LOFAR_ObsSW_@observation@"
#define PST_OBSERVATION	"Observation"
#define	PN_OBS_CLAIM_CLAIM_DATE	"claim.claimDate"
#define	PN_OBS_CLAIM_NAME	"claim.name"
#define PN_OBS_RECEIVER_BITMAP	"receiverBitmap"
#define PN_OBS_CLAIM_PERIOD	"claimPeriod"
#define PN_OBS_PREPARE_PERIOD	"preparePeriod"
#define PN_OBS_START_TIME	"startTime"
#define PN_OBS_STOP_TIME	"stopTime"
#define PN_OBS_BAND_FILTER	"bandFilter"
#define PN_OBS_NYQUISTZONE	"nyquistzone"
#define PN_OBS_ANTENNA_ARRAY	"antennaArray"
#define PN_OBS_RECEIVER_LIST	"receiverList"
#define PN_OBS_SAMPLE_CLOCK	"sampleClock"
#define PN_OBS_RUN_STATE	"runState"
#define PN_OBS_MEASUREMENT_SET	"measurementSet"
#define PN_OBS_STATION_LIST	"stationList"
#define PN_OBS_INPUT_NODE_LIST	"inputNodeList"
#define PN_OBS_BGL_NODE_LIST	"BGLNodeList"
#define PN_OBS_STORAGE_NODE_LIST	"storageNodeList"
#define PN_OBS_BEAMS_ANGLE1	"Beams.angle1"
#define PN_OBS_BEAMS_ANGLE2	"Beams.angle2"
#define PN_OBS_BEAMS_DIRECTION_TYPE	"Beams.directionType"
#define PN_OBS_BEAMS_BEAMLET_LIST	"Beams.beamletList"
#define PN_OBS_BEAMS_SUBBAND_LIST	"Beams.subbandList"

// ObsCtrl
#define PSN_OBS_CTRL	"LOFAR_ObsSW_@observation@_ObsCtrl"
#define PST_OBS_CTRL	"ObsCtrl"

// OnlineCtrl
#define PSN_ONLINE_CTRL	"LOFAR_ObsSW_@observation@_OnlineCtrl"
#define PST_ONLINE_CTRL	"OnlineCtrl"

// Correlator
#define PSN_CORRELATOR	"LOFAR_ObsSW_@observation@_OnlineCtrl_Correlator"
#define PST_CORRELATOR	"Correlator"

// StorageAppl
#define PSN_STORAGE_APPL	"LOFAR_ObsSW_@observation@_OnlineCtrl_StorageAppl"
#define PST_STORAGE_APPL	"StorageAppl"

// Cabinet
#define PSN_CABINET	"LOFAR_PIC_@cabinet@"
#define PST_CABINET	"Cabinet"
#define PN_CAB_FRONT_DOOR_OPEN	"frontDoorOpen"
#define PN_CAB_FRONT_FAN_INNER	"frontFanInner"
#define PN_CAB_FRONT_FAN_OUTER	"frontFanOuter"
#define PN_CAB_FRONT_AIRFLOW	"frontAirflow"
#define PN_CAB_BACK_DOOR_OPEN	"backDoorOpen"
#define PN_CAB_BACKT_FAN_INNER	"backtFanInner"
#define PN_CAB_BACKT_FAN_OUTER	"backtFanOuter"
#define PN_CAB_BACK_AIRFLOW	"backAirflow"
#define PN_CAB_TEMP_ALARM	"tempAlarm"
#define PN_CAB_HUMIDITY_ALARM	"humidityAlarm"
#define PN_CAB_TEMPERATURE	"temperature"
#define PN_CAB_TEMP_MIN	"tempMin"
#define PN_CAB_TEMP_MAX	"tempMax"
#define PN_CAB_TEMP_MAX_MAX	"tempMaxMax"
#define PN_CAB_HUMIDITY	"humidity"
#define PN_CAB_HUMIDITY_MAX	"humidityMax"
#define PN_CAB_HUMIDITY_MAX_MAX	"humidityMaxMax"
#define PN_CAB_CONTROL_MODE	"controlMode"
#define PN_CAB_TEMPERATURE_SENSOR	"temperatureSensor"
#define PN_CAB_HUMIDITY_CONTROL	"humidityControl"
#define PN_CAB_DOOR_CONTROL	"doorControl"

// SubRack
#define PSN_SUB_RACK	"LOFAR_PIC_@cabinet@_@subrack@"
#define PST_SUB_RACK	"SubRack"
#define	PN_SRCK_SPU_STATUS_STATE	"SPU.status.state"
#define	PN_SRCK_SPU_STATUS_CHILD_STATE	"SPU.status.childState"
#define	PN_SRCK_SPU_STATUS_MESSAGE	"SPU.status.message"
#define	PN_SRCK_SPU_STATUS_LEAF	"SPU.status.leaf"
#define PN_SRCK_SPU__VHBA	"SPU.Vhba"
#define PN_SRCK_SPU__VLBA	"SPU.Vlba"
#define PN_SRCK_SPU__VDIG	"SPU.Vdig"
#define PN_SRCK_SPU_TEMPERATURE	"SPU.temperature"
#define	PN_SRCK_CLOCK_BOARD_STATUS_STATE	"clockBoard.status.state"
#define	PN_SRCK_CLOCK_BOARD_STATUS_CHILD_STATE	"clockBoard.status.childState"
#define	PN_SRCK_CLOCK_BOARD_STATUS_MESSAGE	"clockBoard.status.message"
#define	PN_SRCK_CLOCK_BOARD_STATUS_LEAF	"clockBoard.status.leaf"
#define PN_SRCK_CLOCK_BOARD__VFSP	"clockBoard.Vfsp"
#define PN_SRCK_CLOCK_BOARD__VCLOCK	"clockBoard.Vclock"
#define PN_SRCK_CLOCK_BOARD_VERSION	"clockBoard.version"
#define PN_SRCK_CLOCK_BOARD_FREQ	"clockBoard.freq"
#define PN_SRCK_CLOCK_BOARD_LOCK160	"clockBoard.lock160"
#define PN_SRCK_CLOCK_BOARD_LOCK200	"clockBoard.lock200"
#define PN_SRCK_CLOCK_BOARD_TEMPERATURE	"clockBoard.temperature"

// RSPBoard
#define PSN_RSP_BOARD	"LOFAR_PIC_@cabinet@_@subrack@_@RSPBoard@"
#define PST_RSP_BOARD	"RSPBoard"
#define PN_RSP_VOLTAGE12	"voltage12"
#define PN_RSP_VOLTAGE25	"voltage25"
#define PN_RSP_VOLTAGE33	"voltage33"
#define PN_RSP_VERSION	"version"
#define PN_RSP_ALERT	"alert"
#define	PN_RSP__ETHERNET_STATUS_STATE	"Ethernet.status.state"
#define	PN_RSP__ETHERNET_STATUS_CHILD_STATE	"Ethernet.status.childState"
#define	PN_RSP__ETHERNET_STATUS_MESSAGE	"Ethernet.status.message"
#define	PN_RSP__ETHERNET_STATUS_LEAF	"Ethernet.status.leaf"
#define PN_RSP_ETHERNET_PACKETS_RECEIVED	"Ethernet.packetsReceived"
#define PN_RSP_ETHERNET_PACKETS_ERROR	"Ethernet.packetsError"
#define PN_RSP_ETHERNET_LAST_ERROR	"Ethernet.lastError"
#define PN_RSP_MEP_SEQNR	"MEP.seqnr"
#define PN_RSP_MEP_ERROR	"MEP.error"
#define	PN_RSP_BP_STATUS_STATE	"BP.status.state"
#define	PN_RSP_BP_STATUS_CHILD_STATE	"BP.status.childState"
#define	PN_RSP_BP_STATUS_MESSAGE	"BP.status.message"
#define	PN_RSP_BP_STATUS_LEAF	"BP.status.leaf"
#define PN_RSP_BP_TEMPERATURE	"BP.temperature"
#define PN_RSP_BP_VERSION	"BP.version"
#define	PN_RSP_AP0_STATUS_STATE	"AP0.status.state"
#define	PN_RSP_AP0_STATUS_CHILD_STATE	"AP0.status.childState"
#define	PN_RSP_AP0_STATUS_MESSAGE	"AP0.status.message"
#define	PN_RSP_AP0_STATUS_LEAF	"AP0.status.leaf"
#define PN_RSP_AP0_TEMPERATURE	"AP0.temperature"
#define PN_RSP_AP0_VERSION	"AP0.version"
#define PN_RSP_AP0_SYNC_SAMPLE_COUNT	"AP0.SYNC.sampleCount"
#define PN_RSP_AP0_SYNC_SYNC_COUNT	"AP0.SYNC.syncCount"
#define PN_RSP_AP0_SYNC_ERROR_COUNT	"AP0.SYNC.errorCount"
#define	PN_RSP_AP1_STATUS_STATE	"AP1.status.state"
#define	PN_RSP_AP1_STATUS_CHILD_STATE	"AP1.status.childState"
#define	PN_RSP_AP1_STATUS_MESSAGE	"AP1.status.message"
#define	PN_RSP_AP1_STATUS_LEAF	"AP1.status.leaf"
#define PN_RSP_AP1_TEMPERATURE	"AP1.temperature"
#define PN_RSP_AP1_VERSION	"AP1.version"
#define PN_RSP_AP1_SYNC_SAMPLE_COUNT	"AP1.SYNC.sampleCount"
#define PN_RSP_AP1_SYNC_SYNC_COUNT	"AP1.SYNC.syncCount"
#define PN_RSP_AP1_SYNC_ERROR_COUNT	"AP1.SYNC.errorCount"
#define	PN_RSP_AP2_STATUS_STATE	"AP2.status.state"
#define	PN_RSP_AP2_STATUS_CHILD_STATE	"AP2.status.childState"
#define	PN_RSP_AP2_STATUS_MESSAGE	"AP2.status.message"
#define	PN_RSP_AP2_STATUS_LEAF	"AP2.status.leaf"
#define PN_RSP_AP2_TEMPERATURE	"AP2.temperature"
#define PN_RSP_AP2_VERSION	"AP2.version"
#define PN_RSP_AP2_SYNC_SAMPLE_COUNT	"AP2.SYNC.sampleCount"
#define PN_RSP_AP2_SYNC_SYNC_COUNT	"AP2.SYNC.syncCount"
#define PN_RSP_AP2_SYNC_ERROR_COUNT	"AP2.SYNC.errorCount"
#define	PN_RSP_AP3_STATUS_STATE	"AP3.status.state"
#define	PN_RSP_AP3_STATUS_CHILD_STATE	"AP3.status.childState"
#define	PN_RSP_AP3_STATUS_MESSAGE	"AP3.status.message"
#define	PN_RSP_AP3_STATUS_LEAF	"AP3.status.leaf"
#define PN_RSP_AP3_TEMPERATURE	"AP3.temperature"
#define PN_RSP_AP3_VERSION	"AP3.version"
#define PN_RSP_AP3_SYNC_SAMPLE_COUNT	"AP3.SYNC.sampleCount"
#define PN_RSP_AP3_SYNC_SYNC_COUNT	"AP3.SYNC.syncCount"
#define PN_RSP_AP3_SYNC_ERROR_COUNT	"AP3.SYNC.errorCount"

// RCU
#define PSN_RCU	"LOFAR_PIC_@cabinet@_@subrack@_@RSPBoard@_@rcu@"
#define PST_RCU	"RCU"
#define PN_RCU_DELAY	"Delay"
#define PN_RCU_INPUT_ENABLE	"InputEnable"
#define PN_RCU_LBL_ENABLE	"LBLEnable"
#define PN_RCU_LBH_ENABLE	"LBHEnable"
#define PN_RCU_HBA_ENABLE	"HBAEnable"
#define PN_RCU_BAND_SEL_LBA_HBA	"bandSelLbaHba"
#define PN_RCU_HBA_FILTER_SEL	"HBAFilterSel"
#define PN_RCU_VL_ENABLE	"VlEnable"
#define PN_RCU_VH_ENABLE	"VhEnable"
#define PN_RCU_VDD_VCC_ENABLE	"VddVccEnable"
#define PN_RCU_BAND_SEL_LBL_LBH	"bandSelLblLbh"
#define PN_RCU_LBA_FILTER_SEL	"LBAFilterSel"
#define PN_RCU_ATTENUATION	"Attenuation"
#define PN_RCU_NOF_OVERFLOW	"nofOverflow"
#define PN_RCU_ADC_STATISTICS_OVERFLOW	"ADCStatistics.overflow"
#define PN_RCU_TBB_ERROR	"TBB.error"
#define PN_RCU_TBB_MODE	"TBB.mode"
#define PN_RCU_TBB_START_ADDR	"TBB.startAddr"
#define PN_RCU_TBB_BUF_SIZE	"TBB.bufSize"
#define PN_RCU_TRIGGER_STARTLEVEL	"Trigger.startlevel"
#define PN_RCU_TRIGGER_BASELEVEL	"Trigger.baselevel"
#define PN_RCU_TRIGGER_STOPLEVEL	"Trigger.stoplevel"
#define PN_RCU_TRIGGER_FILTER	"Trigger.filter"
#define PN_RCU_TRIGGER_WINDOW	"Trigger.window"
#define PN_RCU_TRIGGER_OPERATING_MODE	"Trigger.operatingMode"
#define PN_RCU_TRIGGER_COEFF0	"Trigger.coeff0"
#define PN_RCU_TRIGGER_COEFF1	"Trigger.coeff1"
#define PN_RCU_TRIGGER_COEFF2	"Trigger.coeff2"
#define PN_RCU_TRIGGER_COEFF3	"Trigger.coeff3"

// TBBoard
#define PSN_TB_BOARD	"LOFAR_PIC_@cabinet@_@subrack@_@TBBoard@"
#define PST_TB_BOARD	"TBBoard"
#define PN_TBB_BOARDID	"boardID"
#define PN_TBB_RAM_SIZE	"RAMSize"
#define PN_TBB_SW_VERSION	"SWVersion"
#define PN_TBB_BOARD_VERSION	"boardVersion"
#define PN_TBB_TP_VERSION	"TPVersion"
#define PN_TBB_MP0_VERSION	"MP0Version"
#define PN_TBB_MP1_VERSION	"MP1Version"
#define PN_TBB_MP2_VERSION	"MP2Version"
#define PN_TBB_MP3_VERSION	"MP3Version"
#define PN_TBB_VOLTAGE12	"voltage12"
#define PN_TBB_VOLTAGE25	"voltage25"
#define PN_TBB_VOLTAGE33	"voltage33"
#define PN_TBB_TEMPPCB	"tempPCB"
#define PN_TBB_TEMPTP	"tempTP"
#define PN_TBB_TEMPMP0	"tempMP0"
#define PN_TBB_TEMPMP1	"tempMP1"
#define PN_TBB_TEMPMP2	"tempMP2"
#define PN_TBB_TEMPMP3	"tempMP3"
#define PN_TBB_IMAGE_INFO_VERSION	"imageInfo.version"
#define PN_TBB_IMAGE_INFO_WRITE_DATE	"imageInfo.writeDate"
#define PN_TBB_IMAGE_INFO_TP_FILE	"imageInfo.TPFile"
#define PN_TBB_IMAGE_INFO_MP_FILE	"imageInfo.MPFile"

// StationClock
#define PSN_STATION_CLOCK	"LOFAR_PIC_StationClock"
#define PST_STATION_CLOCK	"StationClock"
#define PN_SCK_CLOCK	"clock"

// LogProcessor
#define PSN_LOG_PROCESSOR	"LOFAR_PermSW_Daemons_LogProcessor"
#define PST_LOG_PROCESSOR	"LogProcessor"

// SASGateway
#define PSN_SAS_GATEWAY	"LOFAR_PermSW_Daemons_SASGateway"
#define PST_SAS_GATEWAY	"SASGateway"

// HardwareMonitor
#define PSN_HARDWARE_MONITOR	"LOFAR_PermSW_HardwareMonitor"
#define PST_HARDWARE_MONITOR	"HardwareMonitor"
#define PN_HWM_RSP_CONNECTED	"RSP.connected"
#define PN_HWM_TBB_CONNECTED	"TBB.connected"

// SoftwareMonitor
#define PSN_SOFTWARE_MONITOR	"LOFAR_PermSW_SoftwareMonitor"
#define PST_SOFTWARE_MONITOR	"SoftwareMonitor"

// TemperatureMonitor
#define PSN_TEMPERATURE_MONITOR	"LOFAR_PermSW_TemperatureMonitor"
#define PST_TEMPERATURE_MONITOR	"TemperatureMonitor"

// MACInfoServer
#define PSN_MAC_INFO_SERVER	"LOFAR_PermSW_MACInfoServer"
#define PST_MAC_INFO_SERVER	"MACInfoServer"

// StationCtrl
#define PSN_STATION_CTRL	"LOFAR_PermSW_StationCtrl"
#define PST_STATION_CTRL	"StationCtrl"

// DigBoardCtrl
#define PSN_DIG_BOARD_CTRL	"LOFAR_PermSW_DigBoardCtrl"
#define PST_DIG_BOARD_CTRL	"DigBoardCtrl"
#define PN_DBC_CONNECTED	"connected"
#define PN_DBC_CLOCK	"clock"

// StnObservation
#define PSN_STN_OBSERVATION	"LOFAR_ObsSW_@observation@"
#define PST_STN_OBSERVATION	"StnObservation"
#define PN_OBS_NAME	"name"
#define	PN_OBS_CLAIM_CLAIM_DATE	"claim.claimDate"
#define	PN_OBS_CLAIM_NAME	"claim.name"

// BeamCtrl
#define PSN_BEAM_CTRL	"LOFAR_ObsSW_@observation@_BeamCtrl"
#define PST_BEAM_CTRL	"BeamCtrl"
#define PN_BC_CONNECTED	"connected"
#define PN_BC_SUB_ARRAY	"subArray"
#define PN_BC_SUBBAND_LIST	"subbandList"
#define PN_BC_BEAMLET_LIST	"beamletList"
#define PN_BC_ANGLE1	"angle1"
#define PN_BC_ANGLE2	"angle2"
#define PN_BC_DIRECTION_TYPE	"directionType"
#define PN_BC_BEAM_NAME	"beamName"

// CalCtrl
#define PSN_CAL_CTRL	"LOFAR_ObsSW_@observation@_CalCtrl"
#define PST_CAL_CTRL	"CalCtrl"
#define PN_CC_CONNECTED	"connected"
#define PN_CC_BEAM_NAMES	"beamNames"
#define PN_CC_ANTENNA_ARRAY	"antennaArray"
#define PN_CC_FILTER	"filter"
#define PN_CC_NYQUISTZONE	"nyquistzone"
#define PN_CC_RCUS	"rcus"

// TBBCtrl
#define PSN_TBB_CTRL	"LOFAR_ObsSW_@observation@_TBBCtrl"
#define PST_TBB_CTRL	"TBBCtrl"
#define PN_TBC_CONNECTED	"connected"
#define PN_TBC_TRIGGER_RCU_NR	"trigger.rcuNr"
#define PN_TBC_TRIGGER_SEQUENCE_NR	"trigger.sequenceNr"
#define PN_TBC_TRIGGER_TIME	"trigger.time"
#define PN_TBC_TRIGGER_SAMPLE_NR	"trigger.sampleNr"
#define PN_TBC_TRIGGER_SUM	"trigger.sum"
#define PN_TBC_TRIGGER_NR_SAMPLES	"trigger.nrSamples"
#define PN_TBC_TRIGGER_PEAK_VALUE	"trigger.peakValue"
#define PN_TBC_TRIGGER_FLAGS	"trigger.flags"
#define PN_TBC_TRIGGER_TABLE	"trigger.table"

#endif
