package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.File;
import java.io.FileInputStream;

import nl.astron.lofar.odtb.mom2otdbadapter.config.OTDBConfiguration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.util.XMLConverter;

import org.junit.Test;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

public class XMLParserTest {

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
