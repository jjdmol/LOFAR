package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

public class Task {
	protected String xml = null;
	protected String mom2Id = null;
	protected String time = null;
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
