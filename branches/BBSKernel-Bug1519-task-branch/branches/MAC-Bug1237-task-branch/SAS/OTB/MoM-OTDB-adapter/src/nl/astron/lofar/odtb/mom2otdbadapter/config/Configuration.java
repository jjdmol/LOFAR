package nl.astron.lofar.odtb.mom2otdbadapter.config;

public class Configuration {
	private AdapterConfiguration adapter;
	private Mom2Configuration mom2;
	private RepositoryConfiguration repository;

	public AdapterConfiguration getAdapter() {
		return adapter;
	}

	public void setAdapter(AdapterConfiguration adapter) {
		this.adapter = adapter;
	}

	public Mom2Configuration getMom2() {
		return mom2;
	}

	public void setMom2(Mom2Configuration mom2) {
		this.mom2 = mom2;
	}

	public RepositoryConfiguration getRepository() {
		return repository;
	}

	public void setRepository(RepositoryConfiguration repository) {
		this.repository = repository;
	}

}
