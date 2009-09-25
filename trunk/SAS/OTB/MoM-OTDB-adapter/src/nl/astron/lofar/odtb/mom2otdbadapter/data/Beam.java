package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.io.Serializable;

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
	private Double ra = null;
	/**
	 * Angle 2 (mom2 -> otdb)
	 * e.g. [1.026719, 0,711018, 0384089]
	 */
	private Double dec = null;

	/**
	 * Direction type (mom2 -> otdb)
	 * e.g. J2000
	 */
	private String equinox;
	
	/**
	 * duration
	 */
	private Integer duration;
	
	/**
	 * subbands that must be used (mom2 -> otdb)
	 * e.g. [1,3,5,7]
	 */
	private String subbands;

	public Beam(LofarObservation observation){
		this.parentObservation = observation;
	}
	
	public Integer getMom2Id() {
		return mom2Id;
	}

	public void setMom2Id(Integer mom2Id) {
		this.mom2Id = mom2Id;
	}

	public Double getRa() {
		return ra;
	}

	public void setRa(Double ra) {
		this.ra = ra;
	}

	public Double getDec() {
		return dec;
	}

	public void setDec(Double dec) {
		this.dec = dec;
	}

	public String getEquinox() {
		return equinox;
	}

	public void setEquinox(String equinox) {
		this.equinox = equinox;
	}

	public Integer getDuration() {
		return duration;
	}

	public void setDuration(Integer duration) {
		this.duration = duration;
	}

	public String getSubbands() {
		return subbands;
	}

	public void setSubbands(String subbands) {
		this.subbands = subbands;
	}

	public LofarObservation getParentObservation() {
		return parentObservation;
	}
	
	
}
