package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Date;
import java.util.List;

import javax.xml.parsers.ParserConfigurationException;

import nl.astron.lofar.odtb.mom2otdbadapter.config.Mom2Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.config.OTDBConfiguration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.RepositoryException;

import org.junit.Test;

public class XMLGeneratorTest {

	@Test
	public void testGetObservationXml() throws IOException, ParserConfigurationException, RepositoryException {
		OTDBConfiguration config = new OTDBConfiguration();
		config.setRmiHost("lofar17");
		config.setRmiPort(10500);
		config.setTemplateId(5001);
		OTDBRepository repository = new OTDBRepository(config);
		Date startDate = new Date();
		startDate.setMonth(4);
		Date endDate = new Date();
		List<LofarObservation> lofarObservations = repository.getLatestChanges(startDate, endDate);
		if (lofarObservations.size() > 0) {
			LofarObservation lofarObservation = lofarObservations.get(0);
			Mom2Configuration mom2Config = new Mom2Configuration();
			mom2Config.setMom2SchemasUrl("C:/java/workspace/MoM-OTDB-adapter/schemas/");
			String xml = XMLGenerator.getObservationXml(lofarObservation, mom2Config);
			File file = new File("examples/obs-" + lofarObservation.getMom2Id() + "-" + lofarObservation.getStatus() + ".xml");
			PrintWriter writer = new PrintWriter(file);
			writer.print(xml);
			writer.flush();
			writer.close();
		}
	}

}
