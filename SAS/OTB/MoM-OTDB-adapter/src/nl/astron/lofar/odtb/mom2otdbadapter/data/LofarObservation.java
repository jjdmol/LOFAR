package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;


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
	 * 
	 */
	private static final long serialVersionUID = 6822871605012685225L;

	/**
	 * mom2Id of the lofar observation (mom2 <-> otdb)
	 * e.g. 200
	 */
	private Integer mom2Id;
	
	private Integer observationId;

	/**
	 * status of a lofar observation, (mom2 <-> otdb)
	 * e.g. being specified. 
	 */
	private String status;
	
	
	/**
	 * timestamp of status change in UTC (mom2 <-> otdb)
	 * e.g. 2005-Dec-22 15:00:00
	 * 
	 */
	private Date timeStamp;
	/*
	 * only for input
	 */
	private String stationSet;
	/**
	 * Stations that belongs to the array configuration (mom2 -> otdb)
	 * e.g. [CS001,CS002]
	 */
	private List<String> stations = new ArrayList<String>();

	private String antennaArray;	
	
	private String antennaSet;	
	
	/**
	 * Band selection (mom2 -> otdb)
	 * e.g. LB_10_90
	 */
	private String bandFilter;
	
	private String clockMode;

	private List<Beam> beams = new ArrayList<Beam>();
	/*
	 * only for output
	 */
	
	/**
	 * start time of observation (mom2 <- otdb)
	 * e.g. 2005-Dec-22 14:50:00
	 * 
	 */
	private Date startTime;
	/**
	 * end time of observation (mom2 <- otdb)
	 * e.g. 2005-Dec-22 14:53:00
	 * 
	 */
	private Date endTime;
	private Double clockFrequency;
	private Double channelWidth;
	private Integer channelsPerSubband;
	private Double subbandWidth;
	private String fileNameMask;
	private Integer samplesPerSecond;
	private Double integrationInterval;

	public Integer getMom2Id() {
		return mom2Id;
	}
	public void setMom2Id(Integer mom2Id) {
		this.mom2Id = mom2Id;
	}
	public String getStatus() {
		return status;
	}
	public void setStatus(String status) {
		this.status = status;
	}



	public String getAntennaArray() {
		return antennaArray;
	}
	public void setAntennaArray(String antennaArray) {
		this.antennaArray = antennaArray;
	}
	public String getAntennaSet() {
		return antennaSet;
	}
	public void setAntennaSet(String antennaSet) {
		this.antennaSet = antennaSet;
	}
	public String getBandFilter() {
		return bandFilter;
	}
	public void setBandFilter(String bandFilter) {
		this.bandFilter = bandFilter;
	}
	
	
	public List<Beam> getBeams() {
		return beams;
	}
	public void setBeams(List<Beam> beams) {
		this.beams = beams;
	}
	
	
	public String getStationSet() {
		return stationSet;
	}
	public void setStationSet(String stationSet) {
		this.stationSet = stationSet;
	}
	public List<String> getStations() {
		return stations;
	}

	public void setStations(List<String> stations) {
		this.stations = stations;
	}

	public String getClockMode() {
		return clockMode;
	}
	public void setClockMode(String clockMode) {
		this.clockMode = clockMode;
	}
	public Integer getObservationId() {
		return observationId;
	}
	public void setObservationId(Integer observationId) {
		this.observationId = observationId;
	}
	public Date getTimeStamp() {
		return timeStamp;
	}
	public void setTimeStamp(Date timeStamp) {
		this.timeStamp = timeStamp;
	}
	public Date getStartTime() {
		return startTime;
	}
	public void setStartTime(Date startTime) {
		this.startTime = startTime;
	}
	public Date getEndTime() {
		return endTime;
	}
	public void setEndTime(Date endTime) {
		this.endTime = endTime;
	}
	public Double getChannelWidth() {
		return channelWidth;
	}
	public void setChannelWidth(Double channelWidth) {
		this.channelWidth = channelWidth;
	}
	public Integer getChannelsPerSubband() {
		return channelsPerSubband;
	}
	public void setChannelsPerSubband(Integer channelsPerSubband) {
		this.channelsPerSubband = channelsPerSubband;
	}
	public Double getSubbandWidth() {
		return subbandWidth;
	}
	public void setSubbandWidth(Double subbandWidth) {
		this.subbandWidth = subbandWidth;
	}
	public String getFileNameMask() {
		return fileNameMask;
	}
	public void setFileNameMask(String fileNameMask) {
		this.fileNameMask = fileNameMask;
	}
	public Double getClockFrequency() {
		return clockFrequency;
	}
	public void setClockFrequency(Double clockFrequency) {
		this.clockFrequency = clockFrequency;
	}
	public Integer getSamplesPerSecond() {
		return samplesPerSecond;
	}
	public void setSamplesPerSecond(Integer samplesPerSecond) {
		this.samplesPerSecond = samplesPerSecond;
	}
	public Double getIntegrationInterval() {
		return integrationInterval;
	}
	public void setIntegrationInterval(Double integrationInterval) {
		this.integrationInterval = integrationInterval;
	}

	

}
