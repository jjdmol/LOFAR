package nl.astron.lofar.odtb.mom2otdbadapter.data;

import java.util.Date;

import nl.astron.lofar.odtb.mom2otdbadapter.config.OTDBConfiguration;

import org.junit.Before;
import org.junit.Test;

public class OTDBRepositoryTest {

	private OTDBRepository repository;
	@Before
	public void setUp() throws Exception {
		OTDBConfiguration config =new OTDBConfiguration();
		config.setRmiHost("lofar17");
		config.setRmiPort(10399);
		repository = new OTDBRepository(config);
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
		Beam beam = new Beam(lofarObservation);
		beam.setRa(85.650575);
		beam.setDec(49.852009);
		beam.setEquinox("J2000");
		beam.setDuration(423);
		beam.setSubbands("[123,124]");
		lofarObservation.getBeams().add(beam);
		beam = new Beam(lofarObservation);
		beam.setRa(85.650575);
		beam.setDec(49.852009);
		beam.setEquinox("J2000");
		beam.setDuration(42);
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
