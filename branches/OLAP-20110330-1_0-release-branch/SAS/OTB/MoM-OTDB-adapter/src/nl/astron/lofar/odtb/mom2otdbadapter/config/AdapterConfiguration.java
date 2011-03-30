package nl.astron.lofar.odtb.mom2otdbadapter.config;

public class AdapterConfiguration {

	private Integer httpPort;
	private String keystoreLocation;
	private String keystorePassword;
	private String trustedKeystoreLocation;
	private String trustedKeystorePassword;
	
	public Integer getHttpPort() {
		return httpPort;
	}
	public void setHttpPort(Integer httpPort) {
		this.httpPort = httpPort;
	}
	public String getKeystoreLocation() {
		return keystoreLocation;
	}
	public void setKeystoreLocation(String keystoreLocation) {
		this.keystoreLocation = keystoreLocation;
	}
	public String getKeystorePassword() {
		return keystorePassword;
	}
	public void setKeystorePassword(String keystorePassword) {
		this.keystorePassword = keystorePassword;
	}
	public String getTrustedKeystoreLocation() {
		return trustedKeystoreLocation;
	}
	public void setTrustedKeystoreLocation(String trustedKeystoreLocation) {
		this.trustedKeystoreLocation = trustedKeystoreLocation;
	}
	public String getTrustedKeystorePassword() {
		return trustedKeystorePassword;
	}
	public void setTrustedKeystorePassword(String trustedKeystorePassword) {
		this.trustedKeystorePassword = trustedKeystorePassword;
	}
	
}
