package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.io.Serializable;

public class Beam implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = -2820149572789270759L;

	
	private int mom2Id = -1;
	
	/**
	 * Angle 1 (mom2 -> otdb)
	 * e.g. [6.123662, 5.233748, 1459568]
	 */
	private String ra = null;
	/**
	 * Angle 2 (mom2 -> otdb)
	 * e.g. [1.026719, 0,711018, 0384089]
	 */
	private String dec = null;

	/**
	 * Direction type (mom2 -> otdb)
	 * e.g. J2000
	 */
	private String equinox;
	
	/**
	 * Requested duration
	 */
	private String requestedDuration;
	
	/**
	 * subbands that must be used (mom2 -> otdb)
	 * e.g. [1,3,5,7]
	 */
	private String subbands;

	public int getMom2Id() {
		return mom2Id;
	}

	public void setMom2Id(int mom2Id) {
		this.mom2Id = mom2Id;
	}




	public String getRa() {
		return ra;
	}

	public void setRa(String ra) {
		this.ra = ra;
	}

	public String getDec() {
		return dec;
	}

	public void setDec(String dec) {
		this.dec = dec;
	}

	public String getEquinox() {
		return equinox;
	}

	public void setEquinox(String equinox) {
		this.equinox = equinox;
	}

	public String getRequestedDuration() {
		return requestedDuration;
	}

	public void setRequestedDuration(String requestedDuration) {
		this.requestedDuration = requestedDuration;
	}

	public String getSubbands() {
		return subbands;
	}

	public void setSubbands(String subbands) {
		this.subbands = subbands;
	}
	
	
}
