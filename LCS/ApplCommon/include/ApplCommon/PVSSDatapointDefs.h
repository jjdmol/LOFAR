// This file was generated by create_db_files v2.0 on Mon Sep 16 12:18:59 UTC 2013

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

// MACScheduler
#define PSN_MAC_SCHEDULER	"LOFAR_PermSW_MACScheduler"
#define PST_MAC_SCHEDULER	"MACScheduler"
#define PN_MS_PLANNED_OBSERVATIONS	"plannedObservations"
#define PN_MS_ACTIVE_OBSERVATIONS	"activeObservations"
#define PN_MS_FINISHED_OBSERVATIONS	"finishedObservations"
#define PN_MS_OTDB_CONNECTED	"OTDB.connected"
#define PN_MS_OTDB_LAST_POLL	"OTDB.lastPoll"
#define PN_MS_OTDB_POLLINTERVAL	"OTDB.pollinterval"

// CRTriggerControl
#define PSN_CR_TRIGGER_CONTROL	"LOFAR_PermSW_CRTriggerControl"
#define PST_CR_TRIGGER_CONTROL	"CRTriggerControl"

// Observation
#define PSN_OBSERVATION	"LOFAR_ObsSW_@observation@"
#define PST_OBSERVATION	"Observation"
#define	PN_OBS_CLAIM_CLAIM_DATE	"claim.claimDate"
#define	PN_OBS_CLAIM_NAME	"claim.name"
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
#define PN_OBS_PROCESS_TYPE	"processType"
#define PN_OBS_PROCESS_SUBTYPE	"processSubtype"
#define PN_OBS_STRATEGY	"strategy"
#define PN_OBS_STATION_LIST	"stationList"
#define PN_OBS_INPUT_NODE_LIST	"inputNodeList"
#define PN_OBS_BGL_NODE_LIST	"BGLNodeList"
#define PN_OBS_STORAGE_NODE_LIST	"storageNodeList"
#define PN_OBS_BEAMS_ANGLE1	"Beams.angle1"
#define PN_OBS_BEAMS_ANGLE2	"Beams.angle2"
#define PN_OBS_BEAMS_DIRECTION_TYPE	"Beams.directionType"
#define PN_OBS_BEAMS_SUBBAND_LIST	"Beams.subbandList"
#define PN_OBS_TIED_ARRAY_BEAMS_BEAM_INDEX	"TiedArrayBeams.beamIndex"
#define PN_OBS_TIED_ARRAY_BEAMS_ANGLE1	"TiedArrayBeams.angle1"
#define PN_OBS_TIED_ARRAY_BEAMS_ANGLE2	"TiedArrayBeams.angle2"
#define PN_OBS_TIED_ARRAY_BEAMS_DIRECTION_TYPE	"TiedArrayBeams.directionType"
#define PN_OBS_TIED_ARRAY_BEAMS_DISPERSION	"TiedArrayBeams.dispersion"
#define PN_OBS_TIED_ARRAY_BEAMS_COHERENT	"TiedArrayBeams.coherent"

// ObservationControl
#define PSN_OBSERVATION_CONTROL	"LOFAR_ObsSW_@observation@_ObservationControl"
#define PST_OBSERVATION_CONTROL	"ObservationControl"
#define PN_OBSCTRL_COMMAND	"command"

// InputBuffer
#define PSN_INPUT_BUFFER	"LOFAR_PermSW_@psionode@_InputBuffer"
#define PST_INPUT_BUFFER	"InputBuffer"
#define PN_IPB_STATION_NAME	"stationName"
#define PN_IPB_OBSERVATION_NAME	"observationName"
#define PN_IPB_STREAM0_BLOCKS_IN	"stream0.blocksIn"
#define PN_IPB_STREAM0_PERC_BAD	"stream0.percBad"
#define PN_IPB_STREAM0_REJECTED	"stream0.rejected"
#define PN_IPB_STREAM1_BLOCKS_IN	"stream1.blocksIn"
#define PN_IPB_STREAM1_PERC_BAD	"stream1.percBad"
#define PN_IPB_STREAM1_REJECTED	"stream1.rejected"
#define PN_IPB_STREAM2_BLOCKS_IN	"stream2.blocksIn"
#define PN_IPB_STREAM2_PERC_BAD	"stream2.percBad"
#define PN_IPB_STREAM2_REJECTED	"stream2.rejected"
#define PN_IPB_STREAM3_BLOCKS_IN	"stream3.blocksIn"
#define PN_IPB_STREAM3_PERC_BAD	"stream3.percBad"
#define PN_IPB_STREAM3_REJECTED	"stream3.rejected"
#define PN_IPB_LATE	"late"
#define PN_IPB_IO_TIME	"IOTime"

// Adder
#define PSN_ADDER	"LOFAR_ObsSW_@osionode@_@adder@"
#define PST_ADDER	"Adder"
#define PN_ADD_DROPPING	"dropping"
#define PN_ADD_DROPPED	"dropped"
#define PN_ADD_DATA_PRODUCT_TYPE	"dataProductType"
#define PN_ADD_DATA_PRODUCT	"dataProduct"
#define PN_ADD_FILE_NAME	"fileName"
#define PN_ADD_LOCUS_NODE	"locusNode"
#define PN_ADD_WRITER	"writer"
#define PN_ADD_DIRECTORY	"directory"
#define PN_ADD_OBSERVATION_NAME	"observationName"

// Writer
#define PSN_WRITER	"LOFAR_ObsSW_@oslocusnode@_@writer@"
#define PST_WRITER	"Writer"
#define PN_WTR_WRITTEN	"written"
#define PN_WTR_DROPPED	"dropped"
#define PN_WTR_FILE_NAME	"fileName"
#define PN_WTR_DATA_RATE	"dataRate"
#define PN_WTR_DATA_PRODUCT_TYPE	"dataProductType"
#define PN_WTR_OBSERVATION_NAME	"observationName"

// IONode
#define PSN_IO_NODE	"LOFAR_PIC_BGP_@midplane@_@ionode@"
#define PST_IO_NODE	"IONode"
#define PN_ION_STATION0	"station0"
#define PN_ION_IP0	"IP0"
#define PN_ION_MAC0	"MAC0"
#define PN_ION_STATION1	"station1"
#define PN_ION_IP1	"IP1"
#define PN_ION_MAC1	"MAC1"
#define PN_ION_MAC_FOREIGN	"MACForeign"
#define PN_ION_USE2ND_STATION	"use2ndStation"
#define PN_ION_USED_STATION	"usedStation"
#define PN_ION_USEDIP	"usedIP"
#define PN_ION_USEDMAC	"usedMAC"

// LocusNode
#define PSN_LOCUS_NODE	"LOFAR_PIC_@osrack@_@locusnode@"
#define PST_LOCUS_NODE	"LocusNode"
#define PN_LCN_FREE	"free"
#define PN_LCN_TOTAL	"total"
#define PN_LCN_CLAIMED	"claimed"

// CEPHardwareMonitor
#define PSN_CEP_HARDWARE_MONITOR	"LOFAR_PermSW_CEPHardwareMonitor"
#define PST_CEP_HARDWARE_MONITOR	"CEPHardwareMonitor"
#define PN_CHM_BGP_CONNECTED	"BGP.connected"
#define PN_CHM_CLUSTER_CONNECTED	"Cluster.connected"

// OnlineControl
#define PSN_ONLINE_CONTROL	"LOFAR_ObsSW_@observation@_OnlineControl"
#define PST_ONLINE_CONTROL	"OnlineControl"

// BGPAppl
#define PSN_BGP_APPL	"LOFAR_ObsSW_@observation@_OnlineControl_BGPAppl"
#define PST_BGP_APPL	"BGPAppl"
#define PN_BGPA_IO_NODE_LIST	"ioNodeList"
#define PN_BGPA_ADDER_LIST	"adderList"
#define PN_BGPA_LOCUS_NODE_LIST	"locusNodeList"
#define PN_BGPA_WRITER_LIST	"writerList"
#define PN_BGPA_DATA_PRODUCT_LIST	"dataProductList"
#define PN_BGPA_DATA_PRODUCT_TYPE_LIST	"dataProductTypeList"

// CobaltStationInput
#define PSN_COBALT_STATION_INPUT	"LOFAR_PermSW_@pscobaltnode@_CobaltStationInput"
#define PST_COBALT_STATION_INPUT	"CobaltStationInput"
#define PN_CSI_STATION_NAME	"stationName"
#define PN_CSI_OBSERVATION_NAME	"observationName"
#define PN_CSI_STREAM0_BLOCKS_IN	"stream0.blocksIn"
#define PN_CSI_STREAM0_REJECTED	"stream0.rejected"
#define PN_CSI_STREAM1_BLOCKS_IN	"stream1.blocksIn"
#define PN_CSI_STREAM1_REJECTED	"stream1.rejected"
#define PN_CSI_STREAM2_BLOCKS_IN	"stream2.blocksIn"
#define PN_CSI_STREAM2_REJECTED	"stream2.rejected"
#define PN_CSI_STREAM3_BLOCKS_IN	"stream3.blocksIn"
#define PN_CSI_STREAM3_REJECTED	"stream3.rejected"

// CobaltGPUProc
#define PSN_COBALTGPU_PROC	"LOFAR_ObsSW_@observation@_@oscobaltnode@_@cobaltgpuproc@"
#define PST_COBALTGPU_PROC	"CobaltGPUProc"
#define PN_CGP_OBSERVATION_NAME	"observationName"
#define PN_CGP_DATA_PRODUCT_TYPE	"dataProductType"
#define PN_CGP_SUBBAND	"subband"
#define PN_CGP_DROPPING	"dropping"
#define PN_CGP_WRITTEN	"written"
#define PN_CGP_DROPPED	"dropped"

// CobaltOutputProc
#define PSN_COBALT_OUTPUT_PROC	"LOFAR_ObsSW_@observation@_@cobaltoutputproc@"
#define PST_COBALT_OUTPUT_PROC	"CobaltOutputProc"
#define PN_COP_LOCUS_NODE	"locusNode"
#define PN_COP_DATA_PRODUCT_TYPE	"dataProductType"
#define PN_COP_FILE_NAME	"fileName"
#define PN_COP_DIRECTORY	"directory"
#define PN_COP_DROPPING	"dropping"
#define PN_COP_WRITTEN	"written"
#define PN_COP_DROPPED	"dropped"

// Cabinet
#define PSN_CABINET	"LOFAR_PIC_@cabinet@"
#define PST_CABINET	"Cabinet"
#define PN_CAB_FRONT_DOOR_OPEN	"frontDoorOpen"
#define PN_CAB_FRONT_FAN_INNER	"frontFanInner"
#define PN_CAB_FRONT_FAN_OUTER	"frontFanOuter"
#define PN_CAB_FRONT_AIRFLOW	"frontAirflow"
#define PN_CAB_BACK_DOOR_OPEN	"backDoorOpen"
#define PN_CAB_BACK_FAN_INNER	"backFanInner"
#define PN_CAB_BACK_FAN_OUTER	"backFanOuter"
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
#define PN_RSP_BITMODE	"bitmode"
#define PN_RSP_BITMODE_CAPABILITY	"bitmodeCapability"
#define PN_RSP_ALERT	"alert"
#define PN_RSP_SPLITTER_ON	"splitterOn"
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
#define PN_RCU_ANTENNAS	"antennas"
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
#define PN_RCU_TRIGGER_TRIGGER_MODE	"Trigger.triggerMode"
#define PN_RCU_TRIGGER_FILTER0_COEFF0	"Trigger.filter0.coeff0"
#define PN_RCU_TRIGGER_FILTER0_COEFF1	"Trigger.filter0.coeff1"
#define PN_RCU_TRIGGER_FILTER0_COEFF2	"Trigger.filter0.coeff2"
#define PN_RCU_TRIGGER_FILTER0_COEFF3	"Trigger.filter0.coeff3"
#define PN_RCU_TRIGGER_FILTER1_COEFF0	"Trigger.filter1.coeff0"
#define PN_RCU_TRIGGER_FILTER1_COEFF1	"Trigger.filter1.coeff1"
#define PN_RCU_TRIGGER_FILTER1_COEFF2	"Trigger.filter1.coeff2"
#define PN_RCU_TRIGGER_FILTER1_COEFF3	"Trigger.filter1.coeff3"

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
#define PN_TBB_RECORDING	"recording"
#define PN_TBB_IMAGE_INFO_VERSION	"imageInfo.version"
#define PN_TBB_IMAGE_INFO_WRITE_DATE	"imageInfo.writeDate"
#define PN_TBB_IMAGE_INFO_TP_FILE	"imageInfo.TPFile"
#define PN_TBB_IMAGE_INFO_MP_FILE	"imageInfo.MPFile"

// LBAAntenna
#define PSN_LBA_ANTENNA	"LOFAR_PIC_@lbaantenna@"
#define PST_LBA_ANTENNA	"LBAAntenna"

// HBAAntenna
#define PSN_HBA_ANTENNA	"LOFAR_PIC_@hbaantenna@"
#define PST_HBA_ANTENNA	"HBAAntenna"

// StationInfo
#define PSN_STATION_INFO	"LOFAR_PIC_StationInfo"
#define PST_STATION_INFO	"StationInfo"
#define PN_STI_STATIONID	"stationID"
#define PN_STI_N_RSP_BOARDS	"N_RSPBoards"
#define PN_STI_N_TB_BOARDS	"N_TBBoards"
#define PN_STI_N_LBAS	"N_LBAS"
#define PN_STI_N_HBAS	"N_HBAS"
#define PN_STI_HBA__SPLIT	"HBA_Split"
#define PN_STI_WIDE_LBAS	"wide_LBAS"
#define PN_STI_POWER48_ON	"power48On"
#define PN_STI_POWER220_ON	"power220On"
#define PN_STI_DATASTREAM0	"datastream0"
#define PN_STI_DATASTREAM1	"datastream1"
#define PN_STI_CABINET_X	"Cabinet.X"
#define PN_STI_CABINET_Y	"Cabinet.Y"
#define PN_STI_CABINET_Z	"Cabinet.Z"
#define PN_STI_HBA_NR_BROKEN	"HBA.nrBroken"
#define PN_STI_HBA_NR_BROKEN_ARCHIVE	"HBA.nrBrokenArchive"
#define PN_STI_HBA_CENTERX	"HBA.centerX"
#define PN_STI_HBA_CENTERY	"HBA.centerY"
#define PN_STI_HBA_CENTERZ	"HBA.centerZ"
#define PN_STI_HBA_HBA0_CENTERX	"HBA.HBA0.centerX"
#define PN_STI_HBA_HBA0_CENTERY	"HBA.HBA0.centerY"
#define PN_STI_HBA_HBA0_CENTERZ	"HBA.HBA0.centerZ"
#define PN_STI_HBA_HBA0__NORMAL_VECTOR_X	"HBA.HBA0.NormalVector.X"
#define PN_STI_HBA_HBA0__NORMAL_VECTOR_Y	"HBA.HBA0.NormalVector.Y"
#define PN_STI_HBA_HBA0__NORMAL_VECTOR_Z	"HBA.HBA0.NormalVector.Z"
#define PN_STI_HBA_HBA0__ROTATION_MATRIX_X	"HBA.HBA0.RotationMatrix.X"
#define PN_STI_HBA_HBA0__ROTATION_MATRIX_Y	"HBA.HBA0.RotationMatrix.Y"
#define PN_STI_HBA_HBA0__ROTATION_MATRIX_Z	"HBA.HBA0.RotationMatrix.Z"
#define PN_STI_HBA_HBA0_ROTATION	"HBA.HBA0.rotation"
#define PN_STI_HBA_HBA1_CENTERX	"HBA.HBA1.centerX"
#define PN_STI_HBA_HBA1_CENTERY	"HBA.HBA1.centerY"
#define PN_STI_HBA_HBA1_CENTERZ	"HBA.HBA1.centerZ"
#define PN_STI_HBA_HBA1__NORMAL_VECTOR_X	"HBA.HBA1.NormalVector.X"
#define PN_STI_HBA_HBA1__NORMAL_VECTOR_Y	"HBA.HBA1.NormalVector.Y"
#define PN_STI_HBA_HBA1__NORMAL_VECTOR_Z	"HBA.HBA1.NormalVector.Z"
#define PN_STI_HBA_HBA1__ROTATION_MATRIX_X	"HBA.HBA1.RotationMatrix.X"
#define PN_STI_HBA_HBA1__ROTATION_MATRIX_Y	"HBA.HBA1.RotationMatrix.Y"
#define PN_STI_HBA_HBA1__ROTATION_MATRIX_Z	"HBA.HBA1.RotationMatrix.Z"
#define PN_STI_HBA_HBA1_ROTATION	"HBA.HBA1.rotation"
#define PN_STI_LBA_NR_BROKEN	"LBA.nrBroken"
#define PN_STI_LBA_NR_BROKEN_ARCHIVE	"LBA.nrBrokenArchive"
#define PN_STI_LBA_CENTERX	"LBA.centerX"
#define PN_STI_LBA_CENTERY	"LBA.centerY"
#define PN_STI_LBA_CENTERZ	"LBA.centerZ"
#define PN_STI_LBA__NORMAL_VECTOR_X	"LBA.NormalVector.X"
#define PN_STI_LBA__NORMAL_VECTOR_Y	"LBA.NormalVector.Y"
#define PN_STI_LBA__NORMAL_VECTOR_Z	"LBA.NormalVector.Z"
#define PN_STI_LBA__ROTATION_MATRIX_X	"LBA.RotationMatrix.X"
#define PN_STI_LBA__ROTATION_MATRIX_Y	"LBA.RotationMatrix.Y"
#define PN_STI_LBA__ROTATION_MATRIX_Z	"LBA.RotationMatrix.Z"

// LogProcessor
#define PSN_LOG_PROCESSOR	"LOFAR_PermSW_Daemons_LogProcessor"
#define PST_LOG_PROCESSOR	"LogProcessor"

// SASGateway
#define PSN_SAS_GATEWAY	"LOFAR_PermSW_Daemons_SASGateway"
#define PST_SAS_GATEWAY	"SASGateway"

// SoftwareMonitor
#define PSN_SOFTWARE_MONITOR	"LOFAR_PermSW_Daemons_SoftwareMonitor"
#define PST_SOFTWARE_MONITOR	"SoftwareMonitor"
#define PN_SWM_SW_LEVEL	"SWLevel"

// HardwareMonitor
#define PSN_HARDWARE_MONITOR	"LOFAR_PermSW_HardwareMonitor"
#define PST_HARDWARE_MONITOR	"HardwareMonitor"
#define PN_HWM_RSP_CONNECTED	"RSP.connected"
#define PN_HWM_TBB_CONNECTED	"TBB.connected"
#define PN_HWM_EC_CONNECTED	"EC.connected"

// SHMInfoServer
#define PSN_SHM_INFO_SERVER	"LOFAR_PermSW_SHMInfoServer"
#define PST_SHM_INFO_SERVER	"SHMInfoServer"

// StationControl
#define PSN_STATION_CONTROL	"LOFAR_PermSW_StationControl"
#define PST_STATION_CONTROL	"StationControl"
#define PN_SC_ACTIVE_OBSERVATIONS	"activeObservations"

// ClockControl
#define PSN_CLOCK_CONTROL	"LOFAR_PermSW_ClockControl"
#define PST_CLOCK_CONTROL	"ClockControl"
#define PN_CLC_CONNECTED	"connected"
#define PN_CLC_REQUESTED_CLOCK	"requestedClock"
#define PN_CLC_ACTUAL_CLOCK	"actualClock"
#define PN_CLC_REQUESTED_BITMODE	"requestedBitmode"
#define PN_CLC_ACTUAL_BITMODE	"actualBitmode"

// StnObservation
#define PSN_STN_OBSERVATION	"LOFAR_ObsSW_@observation@"
#define PST_STN_OBSERVATION	"StnObservation"
#define PN_OBS_NAME	"name"
#define	PN_OBS_CLAIM_CLAIM_DATE	"claim.claimDate"
#define	PN_OBS_CLAIM_NAME	"claim.name"
#define PN_OBS_RECEIVER_BITMAP	"receiverBitmap"
#define PN_OBS_HBA_BITMAP	"HBABitmap"
#define PN_OBS_LBA_BITMAP	"LBABitmap"

// BeamControl
#define PSN_BEAM_CONTROL	"LOFAR_ObsSW_@observation@_BeamControl"
#define PST_BEAM_CONTROL	"BeamControl"
#define PN_BC_CONNECTED	"connected"
#define PN_BC_SUB_ARRAY	"subArray"
#define PN_BC_SUBBAND_LIST	"subbandList"
#define PN_BC_BEAMLET_LIST	"beamletList"
#define PN_BC_ANGLE1	"angle1"
#define PN_BC_ANGLE2	"angle2"
#define PN_BC_DIRECTION_TYPE	"directionType"
#define PN_BC_BEAM_NAME	"beamName"

// CalibrationControl
#define PSN_CALIBRATION_CONTROL	"LOFAR_ObsSW_@observation@_CalibrationControl"
#define PST_CALIBRATION_CONTROL	"CalibrationControl"
#define PN_CC_CONNECTED	"connected"
#define PN_CC_BEAM_NAMES	"beamNames"
#define PN_CC_ANTENNA_ARRAY	"antennaArray"
#define PN_CC_FILTER	"filter"
#define PN_CC_NYQUISTZONE	"nyquistzone"
#define PN_CC_RCUS	"rcus"

// TBBControl
#define PSN_TBB_CONTROL	"LOFAR_ObsSW_@observation@_TBBControl"
#define PST_TBB_CONTROL	"TBBControl"
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
#define PN_TBC_TRIGGER_MISSED	"trigger.missed"

#endif
