package nl.astron.lofar.odtb.mom2otdbadapter.mom2otdb;

import java.io.IOException;
import java.io.StringReader;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.mom2listener.XMLParser;

import org.apache.http.HttpEntity;
import org.apache.http.HttpEntityEnclosingRequest;
import org.apache.http.HttpException;
import org.apache.http.HttpRequest;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.entity.StringEntity;
import org.apache.http.protocol.HttpContext;
import org.apache.http.protocol.HttpRequestHandler;
import org.apache.http.util.EntityUtils;
import org.apache.xerces.parsers.DOMParser;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

public class Mom2HttpRequestHandler implements HttpRequestHandler {

	
	
	@Override
	public void handle(HttpRequest request, HttpResponse response, HttpContext context) throws HttpException, IOException {
        if (request instanceof HttpEntityEnclosingRequest) {
            HttpEntity entity = ((HttpEntityEnclosingRequest) request).getEntity();
            String content = EntityUtils.toString(entity);
			Document document;
//			try {
//				document = convertStringToDocument(content);
//				XMLParser xmlParser = new XMLParser();
//				LofarObservation lofarObservation = xmlParser
//						.getLofarObservation(document);
//				response.setStatusCode(HttpStatus.SC_OK);				
//			} catch (Exception e) {
//				response.setStatusCode(HttpStatus.SC_BAD_REQUEST);
//			}

			response.setStatusCode(HttpStatus.SC_OK);	
            
        }else {
        	response.setStatusCode(HttpStatus.SC_BAD_REQUEST);
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

}
