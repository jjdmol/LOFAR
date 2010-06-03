package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.util.Date;
import java.util.List;

public interface Repository {
	
	public void store(LofarObservation lofarObservation) throws RepositoryException;
	
	public List<LofarObservation> getLatestChanges(Date startDate, Date endDate) throws RepositoryException;
}
