package nl.astron.lofar.odtb.mom2otdbadapter.config;

public class Mom2Configuration {
	
	private String username;
	private String password;
	private String authUrl;
	private String mom2ImportUrl;
	private String mom2SchemasUrl;
	
	public String getUsername() {
		return username;
	}
	public void setUsername(String username) {
		this.username = username;
	}
	public String getPassword() {
		return password;
	}
	public void setPassword(String password) {
		this.password = password;
	}
	public String getAuthUrl() {
		return authUrl;
	}
	public void setAuthUrl(String authUrl) {
		this.authUrl = authUrl;
	}
	public String getMom2ImportUrl() {
		return mom2ImportUrl;
	}
	public void setMom2ImportUrl(String mom2ImportUrl) {
		this.mom2ImportUrl = mom2ImportUrl;
	}
	public String getMom2SchemasUrl() {
		return mom2SchemasUrl;
	}
	public void setMom2SchemasUrl(String mom2SchemasUrl) {
		this.mom2SchemasUrl = mom2SchemasUrl;
	}

}
