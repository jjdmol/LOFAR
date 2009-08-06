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
import nl.astron.util.AstronValidator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.xerces.dom.DocumentImpl;
import org.apache.xerces.parsers.DOMParser;
import org.apache.xml.serialize.OutputFormat;
import org.apache.xml.serialize.XMLSerializer;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;

/**
 * Process a mom2 connection
 * @author Bastiaan Verhoef
 *
 */
public class ProcessMom2Connection extends Thread {
	private Log log = LogFactory.getLog(this.getClass());

	private BufferedReader in = null;

	private PrintWriter out = null;

	private Socket client = null;

	private OTDBRepository repository;

	
	/**
	 * Constructor
	 * @param repository OTDBRepository with connection to jotdb
	 * @param client Socket with the connection to mom
	 */
	public ProcessMom2Connection(OTDBRepository repository, Socket client) {
		this.client = client;
		this.repository = repository;
	}


	/**
	 * Starts the thread ProcessMom2Connection.
	 * It processes mom2 input and writes output to mom2.
	 * 
	 */
	public void run() {
		log.info("Process connection with mom2");
		try {
			String line = null;
			StringBuffer stringBuffer = new StringBuffer(); 
			in = new BufferedReader(new InputStreamReader(client.getInputStream()));
			/*
			 * it reads the input stream, until the end token is given
			 */
			while ((line =in.readLine()) != null && !line.equals("[input end]")) {
				stringBuffer.append(line);
			}
			/*
			 * process the input stream, it returns the result xml
			 */
			String outputString = processInput(stringBuffer.toString());
			/*
			 * write the result xml to the output stream
			 */
			out = new PrintWriter(client.getOutputStream(), true);
			out.println(outputString);
			out.println("[input end]");
		} catch (IOException e) {
			log.error("IOException: " + e.getMessage(), e);
		} 
	}

	/**
	 * It process the input and stores it to repository
	 * @param input input xml
	 * @return result xml
	 */
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

	/**
	 * Convert a xml-string to a Document
	 * @param myXML xml string
	 * @return Document
	 * @throws Exception
	 */
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

	/**
	 * Creates a result xml by given erros
	 * @param errors Errors 
	 * @return result xml
	 */
	public String getResultXml(String errors) {
		try {
			Document document = new DocumentImpl();
			Element result = document.createElement("result");
			document.appendChild(result);
			Element errorsElement = document.createElement("errors");

			if (!AstronValidator.isBlankOrNull(errors)) {
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
	
	/* (non-Javadoc)
	 * @see java.lang.Object#finalize()
	 */
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
