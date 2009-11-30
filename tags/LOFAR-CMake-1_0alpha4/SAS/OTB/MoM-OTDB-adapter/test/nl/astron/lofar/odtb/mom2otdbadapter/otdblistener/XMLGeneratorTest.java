package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;

import javax.xml.parsers.ParserConfigurationException;

import junit.framework.Assert;

import nl.astron.lofar.odtb.mom2otdbadapter.config.Mom2Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.config.OTDBConfiguration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.RepositoryException;
import nl.astron.util.AstronConverter;

import org.junit.Test;

public class XMLGeneratorTest {

	@Test
	public void testGetObservationXml() throws IOException, ParserConfigurationException, RepositoryException {
		Locale.setDefault(Locale.US);
		OTDBConfiguration config = new OTDBConfiguration();
		config.setRmiHost("lofar17");
		config.setRmiPort(10399);
		config.setTemplateId(5001);
		OTDBRepository repository = new OTDBRepository(config);
		Date startDate = new Date();
		startDate.setMonth(4);
		Date endDate = new Date();
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat();
        simpleDateFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
		List<LofarObservation> lofarObservations = repository.getLatestChanges(startDate, endDate);
		for (int i = 0; i < lofarObservations.size(); i++){
			LofarObservation lofarObservation = lofarObservations.get(i);
			Mom2Configuration mom2Config = new Mom2Configuration();
			mom2Config.setMom2SchemasUrl("http://localhost:8080/mom2lofar/schemas/");
			String xml = XMLGenerator.getObservationXml(lofarObservation, mom2Config);
			File file = new File("examples/obs-" + lofarObservation.getMom2Id() + "-" + lofarObservation.getStatus() + ".xml");
			PrintWriter writer = new PrintWriter(file);
			writer.print(xml);
			writer.flush();
			writer.close();
		}
		//2009-Oct-02 08:51:23 "yyyy-MMM-dd HH:mm:ss"
	}

}
