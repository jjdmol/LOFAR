

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.StringReader;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.mom2listener.XMLParser;

import org.apache.xerces.parsers.DOMParser;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

public class TestOTDB {

	/**
	 * @param args
	 * @throws NotBoundException 
	 * @throws IOException 
	 * @throws SAXException 
	 */
	public static void main(String[] args) throws NotBoundException, SAXException, IOException {
		
		DOMParser itsParser = new DOMParser();
		FileInputStream st = new FileInputStream(new File("c:/observation.xml"));
		
		InputSource source = new InputSource(st);
		itsParser.parse(source);
		// get document
		Document document = itsParser.getDocument();		
		XMLParser xmlParser = new XMLParser();
		LofarObservation lofarObservation = xmlParser
				.getLofarObservation(document);
		OTDBRepository repository = new OTDBRepository("lofar17.astron.nl", 10500);
		try {
			repository.store(lofarObservation);
		}catch (Exception e){
			e.printStackTrace();
		}
		System.out.println("test");

	}
	


}
