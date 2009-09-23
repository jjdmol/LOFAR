package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.File;
import java.io.FileInputStream;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.util.XMLConverter;

import org.junit.Test;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

public class XMLParserTest {

	@Test
	public void testGetLofarObservation() throws Exception {
		FileInputStream st = new FileInputStream(new File("examples/observation.xml"));
		InputSource inputSource = new InputSource(st);
		Document document = XMLConverter.convertXMLToDocument(inputSource, null);
		XMLParser xmlParser = new XMLParser();
		LofarObservation lofarObservation = xmlParser.getLofarObservation(document);
		
	}

}
