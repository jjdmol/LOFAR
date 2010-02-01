package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class Beam implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = -2820149572789270759L;

	private LofarObservation parentObservation; 
	
	private Integer mom2Id ;
	
	/**
	 * Angle 1 (mom2 -> otdb)
	 * e.g. [6.123662, 5.233748, 1459568]
	 */
	private List<Double> raList = new ArrayList<Double>();
	/**
	 * Angle 2 (mom2 -> otdb)
	 * e.g. [1.026719, 0,711018, 0384089]
	 */
	private List<Double> decList  = new ArrayList<Double>();

	/**
	 * Direction type (mom2 -> otdb)
	 * e.g. J2000
	 */
	private String equinox;
	
	/**
	 * duration
	 */
	private List<Integer> durations = new ArrayList<Integer>();
	
	private List<Integer> angleTimes= new ArrayList<Integer>();
	
	/**
	 * subbands that must be used (mom2 -> otdb)
	 * e.g. [1,3,5,7]
	 */
	private List<Integer> subbands = new ArrayList<Integer>();

	private List<Integer> beamlets = new ArrayList<Integer>();
	
	public Beam(LofarObservation observation){
		this.parentObservation = observation;
	}
	
	public Integer getMom2Id() {
		return mom2Id;
	}

	public void setMom2Id(Integer mom2Id) {
		this.mom2Id = mom2Id;
	}


	public String getEquinox() {
		return equinox;
	}

	public void setEquinox(String equinox) {
		this.equinox = equinox;
	}


	public List<Double> getRaList() {
		return raList;
	}

	public void setRaList(List<Double> raList) {
		this.raList = raList;
	}

	public List<Double> getDecList() {
		return decList;
	}

	public void setDecList(List<Double> decList) {
		this.decList = decList;
	}

	public List<Integer> getDurations() {
		return durations;
	}

	public void setDurations(List<Integer> durations) {
		this.durations = durations;
	}

	public List<Integer> getAngleTimes() {
		return angleTimes;
	}

	public void setAngleTimes(List<Integer> angleTimes) {
		this.angleTimes = angleTimes;
	}

	public List<Integer> getSubbands() {
		return subbands;
	}

	public void setSubbands(List<Integer> subbands) {
		this.subbands = subbands;
	}

	public LofarObservation getParentObservation() {
		return parentObservation;
	}

	public List<Integer> getBeamlets() {
		return beamlets;
	}

	public void setBeamlets(List<Integer> beamlets) {
		this.beamlets = beamlets;
	}
	
	
}
