package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.StringReader;
import java.net.Socket;


import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.xerces.parsers.DOMParser;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;



public class ProcessConnection extends Thread {
	private Log log = LogFactory.getLog(this.getClass());
	private Socket client;
	private OTDBRepository repository;
	


	// Constructor
	public ProcessConnection(OTDBRepository repository, Socket client) {
		this.client = client;
		this.repository =repository;
	}

	public void run() {
		log.info("Process client connection");
		String line = null;
		BufferedReader in = null;
		PrintWriter out = null;
		try {
/*			in = new BufferedReader(new InputStreamReader(client
					.getInputStream()));*/
			out = new PrintWriter(client.getOutputStream(), true);
/*			StringBuffer stringBuffer = new StringBuffer();
			while ((line = in.readLine()) != null) {

				//log.info(line);
				stringBuffer.append(line);
				
				
			}*/
			out.write(processInput(client.getInputStream()));
			out.close();
			client.close();
		} catch (IOException e) {
			log.error("Read failed");
		}
	}
	protected String processInput(InputStream input){
		try {
			log.info(input);
			Document document = convertFileToDocument(input);

			XMLParser xmlParser = new XMLParser();
			LofarObservation lofarObservation = xmlParser.getLofarObservation(document);
			repository.store(lofarObservation);
			return "succeed";
		}catch (Exception e){
			log.error("Exception throwed: " + e.getMessage(), e);
		}
		return "failed";
	}
	
	protected Document convertStringToDocument(String myXML) throws Exception {
		// read an xml string into a domtree
		Document document;
		DOMParser itsParser = new DOMParser();
		
		StringReader reader = new StringReader(myXML);
		InputSource source = new InputSource(reader);
		itsParser.parse(source);
				
		// get document
		document = itsParser.getDocument();
		return document;
	}
	public Document convertFileToDocument(InputStream inputStream) throws Exception {
		// read an xml string into a domtree
		Document document;
		DOMParser itsParser = new DOMParser();
		InputSource inputSource = new InputSource();
		inputSource.setByteStream(inputStream);
		
		itsParser.parse(inputSource);
				
		// get document
		document = itsParser.getDocument();
		return document;
	}
}
