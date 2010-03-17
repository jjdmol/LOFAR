package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.File;
import java.io.FileInputStream;
import java.io.StringWriter;
import java.util.Date;
import java.util.List;

import nl.astron.lofar.odtb.mom2otdbadapter.config.Mom2Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.config.OTDBConfiguration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.StubRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.XMLGenerator;
import nl.astron.util.XMLConverter;

import org.junit.Test;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

public class XMLParserTest {

	@Test
	public void testGetLofarObservationStub() throws Exception {
		FileInputStream st = new FileInputStream(new File("examples/output.xml"));
		InputSource inputSource = new InputSource(st);
		Document document = XMLConverter.convertXMLToDocument(inputSource);
		LofarObservation lofarObservation = XMLParser.getLofarObservation(document);

		StubRepository repository = new StubRepository();
		repository.store(lofarObservation);
		Date startDate = new Date();
		startDate.setMonth(4);
		Date endDate = new Date();
		Mom2Configuration config = new Mom2Configuration();
		config.setMom2SchemasUrl("http://localhost:8080/mom2lofar/schemas/");
		List<LofarObservation> observations = repository.getLatestChanges(startDate, endDate);
		for (LofarObservation observation: observations){
			System.out.println(XMLGenerator.getObservationXml(observation, config));
		}
		System.out.println("YES");
	}
	
	@Test
	public void testGetLofarObservation() throws Exception {
		FileInputStream st = new FileInputStream(new File("examples/observation-64-specified.xml"));
		InputSource inputSource = new InputSource(st);
		Document document = XMLConverter.convertXMLToDocument(inputSource);
		LofarObservation lofarObservation = XMLParser.getLofarObservation(document);
		OTDBConfiguration config =new OTDBConfiguration();
		config.setRmiHost("lofar17");
		config.setRmiPort(10399);
		config.setTemplateId(5001);
		OTDBRepository repository = new OTDBRepository(config);
		repository.store(lofarObservation);
		System.out.println("YES");
	}

}
