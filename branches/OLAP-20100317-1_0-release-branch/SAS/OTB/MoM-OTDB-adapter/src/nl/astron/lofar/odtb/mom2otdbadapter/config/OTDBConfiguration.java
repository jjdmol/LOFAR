package nl.astron.lofar.odtb.mom2otdbadapter.config;

public class OTDBConfiguration extends RepositoryConfiguration {

	private String rmiHost;
	private Integer rmiPort;
	private Integer templateId;
	
	public String getRmiHost() {
		return rmiHost;
	}

	public void setRmiHost(String rmiHost) {
		this.rmiHost = rmiHost;
	}

	public Integer getRmiPort() {
		return rmiPort;
	}

	public void setRmiPort(Integer rmiPort) {
		this.rmiPort = rmiPort;
	}

	public void setTemplateId(Integer templateId) {
		this.templateId = templateId;
	}

	public Integer getTemplateId() {
		return templateId;
	}

}
