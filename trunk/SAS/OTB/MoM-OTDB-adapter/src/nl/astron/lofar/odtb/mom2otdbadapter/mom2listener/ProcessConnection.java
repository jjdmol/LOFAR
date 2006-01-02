package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.StringReader;
import java.io.StringWriter;
import java.net.Socket;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.wsrt.util.WsrtValidator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.xerces.dom.DocumentImpl;
import org.apache.xerces.parsers.DOMParser;
import org.apache.xml.serialize.OutputFormat;
import org.apache.xml.serialize.XMLSerializer;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;

public class ProcessConnection extends Thread {
	private Log log = LogFactory.getLog(this.getClass());


	private BufferedReader in = null;

	private PrintWriter out = null;

	private Socket client = null;

	private OTDBRepository repository;

	// Constructor
	public ProcessConnection(OTDBRepository repository, Socket client) {
		this.client = client;
		this.repository = repository;
	}

	public void run() {
		log.info("Process connection with mom2");
		try {
			String line = null;
			StringBuffer stringBuffer = new StringBuffer(); 
			//log.info("Read inputstream...");
			in = new BufferedReader(new InputStreamReader(client.getInputStream()));
			while ((line =in.readLine()) != null && !line.equals("[input end]")) {
				stringBuffer.append(line);
			}
			//log.info("Process mom2 input...");
			String outputString = processInput(stringBuffer.toString());
			//log.debug("Write output: " + outputString);
			out = new PrintWriter(client.getOutputStream(), true);
			out.println(outputString);
			out.println("[input end]");
		} catch (IOException e) {
			log.error("IOException: " + e.getMessage(), e);
		} 
	}

	protected String processInput(String input) {
		try {
			Document document = convertStringToDocument(input);

			XMLParser xmlParser = new XMLParser();
			LofarObservation lofarObservation = xmlParser
					.getLofarObservation(document);
			repository.store(lofarObservation);
			return getResultXml(null);
		} catch (Exception e) {
			log.error("Exception throwed: " + e.getMessage(), e);
			return getResultXml("Exception throwed: " + e.getMessage());
		}

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

	public String getResultXml(String errors) {
		try {
			Document document = new DocumentImpl();
			Element result = document.createElement("result");
			document.appendChild(result);
			Element errorsElement = document.createElement("errors");

			if (!WsrtValidator.isBlankOrNull(errors)) {
				errorsElement.appendChild(document.createTextNode(errors));
			}
			result.appendChild(errorsElement);

			OutputFormat format = new OutputFormat(document); // Serialize DOM
			format.setIndenting(true);

			StringWriter stringOut = new StringWriter(); // Writer will be a
			// String
			XMLSerializer serial = new XMLSerializer(stringOut, format);
			serial.asDOMSerializer(); // As a DOM Serializer

			serial.serialize(document.getDocumentElement());

			return stringOut.toString(); // Spit out DOM as a String
		} catch (IOException io) {
			log.error("IOException throwed: " + io.getMessage(), io);
		}
		return "";
	}
	
	protected void finalize() {
		// Clean up
		try {
			in.close();
			out.close();
			client.close();
		} catch (IOException e) {
			log.error("Could not close.");
		}
	}
}
