package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.io.Serializable;


public class LofarObservation implements Serializable{
	/*
	 * for both input and output
	 */
	protected int mom2Id = -1;
	protected String status = null;
	protected String measurementMom2Ids = null;
	protected String angleTimes = null;
	protected String timeStamp = null;
	/*
	 * only for input
	 */
	protected String subbands = null;
	protected String backend = null;
	protected String arrayConfiguration = null;
	protected String stations = null;
	protected String srgConfiguration = null;
	protected Integer samplingFrequency = null;
	protected String bandSelection = null;
	protected String angle1 = null;
	protected String angle2 = null;

	protected String directionType = null;
	protected int requestedDuration = 0;
	
	/*
	 * only for output
	 */
	protected String startTime = null;
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
