package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.rmi.RemoteException;
import java.util.Date;

import org.junit.Before;
import org.junit.Test;

public class OTDBRepositoryTest {

	private OTDBRepository repository;
	@Before
	public void setUp() throws Exception {
		repository = new OTDBRepository("lofar17", 10399);
	}

	
	@Test
	public void testStore() throws RepositoryException{
		LofarObservation lofarObservation = new LofarObservation();
		lofarObservation.setMom2Id(3);
		lofarObservation.setStatus("being specified");
		lofarObservation.setAntennaArray("HBA");
		lofarObservation.setAntennaSet("HBA_TWO");
		lofarObservation.setBandFilter("HBA_100_190");
		lofarObservation.setClockMode("<<Clock200");
		lofarObservation.setStationSet("Custom");
		lofarObservation.setStations("[CS302, CS010]");
		Beam beam = new Beam();
		beam.setRa("85.650575");
		beam.setDec("49.852009");
		beam.setEquinox("J2000");
		beam.setRequestedDuration("423");
		beam.setSubbands("[123,124]");
		lofarObservation.getBeams().add(beam);
		beam = new Beam();
		beam.setRa("85.650575");
		beam.setDec("49.852009");
		beam.setEquinox("J2000");
		beam.setRequestedDuration("42");
		beam.setSubbands("[1,2,3,4]");
		lofarObservation.getBeams().add(beam);
		repository.store(lofarObservation);

	}

	@Test
	public void testGetLatestChanges() throws RepositoryException  {
		Date startDate = new Date();
		startDate.setMonth(4);
		Date endDate = new Date();
		repository.getLatestChanges(startDate, endDate);

	}

}
