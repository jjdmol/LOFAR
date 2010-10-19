package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.io.Serializable;


/**
 * This data class is a mapping to the lofar observation data of OTDB
 * It contains data that will store in OTDB
 * 
 * @author Bastiaan Verhoef
 *
 */
public class LofarObservation implements Serializable{
	/*
	 * for both input and output
	 */

	/**
	 * mom2Id of the lofar observation (mom2 <-> otdb)
	 * e.g. 200
	 */
	protected int mom2Id = -1;

	/**
	 * status of a lofar observation, (mom2 <-> otdb)
	 * e.g. being specified. 
	 */
	protected String status = null;
	
	/**
	 * mom2Ids of the measurements that belongs to the lofarobservation (mom2 <-> otdb)
	 * e.g. [201,202,203]
	 */
	protected String measurementMom2Ids = null;
	
	/**
	 * angles times in seconds. (mom2 <-> otdb)
	 * e.g. [+0,+30,+60]
	 */
	protected String angleTimes = null;
	
	/**
	 * timestamp of status change in UTC (mom2 <-> otdb)
	 * e.g. 2005-Dec-22 15:00:00
	 * 
	 */
	protected String timeStamp = null;
	/*
	 * only for input
	 */
	
	/**
	 * subbands that must be used (mom2 -> otdb)
	 * e.g. [1,3,5,7]
	 */
	protected String subbands = null;
	/**
	 * backend that must be used (mom2 -> otdb)
	 * e.g. Transient
	 */
	protected String backend = null;
	/**
	 * Array configuration (mom2 -> otdb)
	 * e.g. Detailed
	 */
	protected String arrayConfiguration = null;
	/**
	 * Stations that belongs to the array configuration (mom2 -> otdb)
	 * e.g. [CS001,CS002]
	 */
	protected String stations = null;
	/**
	 * SRG configuration (mom2 -> otdb)
	 * e.g. SA1
	 */
	protected String srgConfiguration = null;
	/**
	 * Sampling frequency in Hz (mom2 -> otdb)
	 * e.g. 160000000
	 */
	protected Integer samplingFrequency = null;
	/**
	 * Band selection (mom2 -> otdb)
	 * e.g. LB_10_90
	 */
	protected String bandSelection = null;
	/**
	 * Angle 1 (mom2 -> otdb)
	 * e.g. [6.123662, 5.233748, 1459568]
	 */
	protected String angle1 = null;
	/**
	 * Angle 2 (mom2 -> otdb)
	 * e.g. [1.026719, 0,711018, 0384089]
	 */
	protected String angle2 = null;

	/**
	 * Direction type (mom2 -> otdb)
	 * e.g. J2000
	 */
	protected String directionType = null;
	/**
	 * requested duration is milliseconds (mom2 -> otdb)
	 * e.g. 840000
	 */
	protected int requestedDuration = 0;
	
	/*
	 * only for output
	 */
	
	/**
	 * start time of observation (mom2 <- otdb)
	 * e.g. 2005-Dec-22 14:50:00
	 * 
	 */
	protected String startTime = null;
	/**
	 * end time of observation (mom2 <- otdb)
	 * e.g. 2005-Dec-22 14:53:00
	 * 
	 */
	protected String endTime = null;
	
	
	public int getMom2Id() {
		return mom2Id;
	}
	public void setMom2Id(int mom2Id) {
		this.mom2Id = mom2Id;
	}
	public String getStatus() {
		return status;
	}
	public void setStatus(String status) {
		this.status = status;
	}

	public String getSubbands() {
		return subbands;
	}

	public void setSubbands(String subbands) {
		this.subbands = subbands;
	}

	public String getArrayConfiguration() {
		return arrayConfiguration;
	}

	public void setArrayConfiguration(String arrayConfgiuration) {
		this.arrayConfiguration = arrayConfgiuration;
	}

	public String getBackend() {
		return backend;
	}

	public void setBackend(String backend) {
		this.backend = backend;
	}

	public String getBandSelection() {
		return bandSelection;
	}

	public void setBandSelection(String bandSelection) {
		this.bandSelection = bandSelection;
	}






	public Integer getSamplingFrequency() {
		return samplingFrequency;
	}

	public void setSamplingFrequency(Integer samplingFrequency) {
		this.samplingFrequency = samplingFrequency;
	}

	public String getSrgConfiguration() {
		return srgConfiguration;
	}

	public void setSrgConfiguration(String srgConfiguration) {
		this.srgConfiguration = srgConfiguration;
	}

	public String getAngleTimes() {
		return angleTimes;
	}

	public void setAngleTimes(String angeTimes) {
		this.angleTimes = angeTimes;
	}

	public String getAngle1() {
		return angle1;
	}

	public void setAngle1(String angle1) {
		this.angle1 = angle1;
	}

	public String getAngle2() {
		return angle2;
	}

	public void setAngle2(String angle2) {
		this.angle2 = angle2;
	}

	public String getDirectionType() {
		return directionType;
	}

	public void setDirectionType(String directionType) {
		this.directionType = directionType;
	}

	public String getStations() {
		return stations;
	}

	public void setStations(String stations) {
		this.stations = stations;
	}
	public int getRequestedDuration() {
		return requestedDuration;
	}
	public void setRequestedDuration(int requestedDuration) {
		this.requestedDuration = requestedDuration;
	}
	public String getEndTime() {
		return endTime;
	}
	public void setEndTime(String endTime) {
		this.endTime = endTime;
	}
	public String getMeasurementMom2Ids() {
		return measurementMom2Ids;
	}
	public void setMeasurementMom2Ids(String measurementMom2Ids) {
		this.measurementMom2Ids = measurementMom2Ids;
	}
	public String getStartTime() {
		return startTime;
	}
	public void setStartTime(String startTime) {
		this.startTime = startTime;
	}
	public String getTimeStamp() {
		return timeStamp;
	}
	public void setTimeStamp(String timeStamp) {
		this.timeStamp = timeStamp;
	}

}
