package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.io.Serializable;
import java.util.ArrayList;
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
	private int mom2Id = -1;

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
	private String timeStamp;
	/*
	 * only for input
	 */
	private String stationSet;
	/**
	 * Stations that belongs to the array configuration (mom2 -> otdb)
	 * e.g. [CS001,CS002]
	 */
	private String stations;

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
	private String startTime;
	/**
	 * end time of observation (mom2 <- otdb)
	 * e.g. 2005-Dec-22 14:53:00
	 * 
	 */
	private String endTime;
	
	
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
	public String getStations() {
		return stations;
	}

	public void setStations(String stations) {
		this.stations = stations;
	}

	public String getClockMode() {
		return clockMode;
	}
	public void setClockMode(String clockMode) {
		this.clockMode = clockMode;
	}
	public String getEndTime() {
		return endTime;
	}
	public void setEndTime(String endTime) {
		this.endTime = endTime;
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
