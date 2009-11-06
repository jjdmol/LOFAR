package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

/**
 * Task
 * @author Bastiaan Verhoef
 *
 */
public class Task {
	/**
	 * MoM2 xml representation of a lofarobservation
	 */
	protected String xml = null;
	
	/**
	 * Mom2 id 
	 */
	protected String mom2Id = null;
	/**
	 * Timestamp of the status change of the lofarobservation
	 */
	protected String time = null;
	
	/**
	 * Name of the file that belongs to this task
	 */
	protected String fileName = null;
	public String getXml() {
		return xml;
	}

	public void setXml(String xml) {
		this.xml = xml;
	}

	public String getMom2Id() {
		return mom2Id;
	}

	public void setMom2Id(String mom2Id) {
		this.mom2Id = mom2Id;
	}

	public String getTime() {
		return time;
	}

	public void setTime(String time) {
		this.time = time;
	}

	public String getFileName() {
		return fileName;
	}

	public void setFileName(String fileName) {
		this.fileName = fileName;
	}
}
