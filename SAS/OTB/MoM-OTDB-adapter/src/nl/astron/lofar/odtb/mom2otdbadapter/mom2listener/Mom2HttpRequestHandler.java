package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.IOException;
import java.io.StringReader;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.Repository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.RepositoryException;
import nl.astron.util.XMLConverter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.apache.http.HttpEntityEnclosingRequest;
import org.apache.http.HttpRequest;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.ParseException;
import org.apache.http.protocol.HttpContext;
import org.apache.http.protocol.HttpRequestHandler;
import org.apache.http.util.EntityUtils;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

public class Mom2HttpRequestHandler implements HttpRequestHandler {
	private static Log log = LogFactory.getLog(Mom2HttpRequestHandler.class);
	private Repository repository;

	public Mom2HttpRequestHandler(Repository repository) {
		this.repository = repository;
	}

	@Override
	public void handle(HttpRequest request, HttpResponse response, HttpContext context) {
		boolean requestHandledSuccessfull = false; 
		log.info("handle Mom2 request...");
		if (log.isDebugEnabled()) {
			for (Header header : request.getAllHeaders()) {
				log.debug("Header: " + header.getName() + " value: " + header.getValue());
			}
		}
		if (request instanceof HttpEntityEnclosingRequest) {
			HttpEntity entity = ((HttpEntityEnclosingRequest) request).getEntity();
			String content = null;
			try {
				content = EntityUtils.toString(entity);
				if (log.isDebugEnabled()){
					log.debug(content);
				}
			} catch (ParseException e) {
				response.setStatusCode(HttpStatus.SC_BAD_REQUEST);
				log.error("Problem with parsing http request: " + e.getMessage(), e);
			} catch (IOException e) {
				response.setStatusCode(HttpStatus.SC_BAD_REQUEST);
				log.error("Problem with reading http request: " + e.getMessage(), e);
			}
			if (content != null){
				InputSource inputSource = new InputSource(new StringReader(content));
				Document document = null;
				try {
					document = XMLConverter.convertXMLToDocument(inputSource);
				} catch (Exception e) {
					response.setStatusCode(HttpStatus.SC_BAD_REQUEST);
					log.error("Problem with parsing xml content: " + e.getMessage(), e);
				}		
				LofarObservation lofarObservation = XMLParser.getLofarObservation(document);
				try {
					repository.store(lofarObservation);
					response.setStatusCode(HttpStatus.SC_OK);
					requestHandledSuccessfull = true;
				} catch (RepositoryException e) {
					response.setStatusCode(HttpStatus.SC_INTERNAL_SERVER_ERROR);
					log.fatal("Problem occurred with OTDB: " + e.getMessage(), e);
				} catch (Exception e) {
					response.setStatusCode(HttpStatus.SC_INTERNAL_SERVER_ERROR);
					log.fatal("Problem occurred : " + e.getMessage(), e);
				}
				response.setStatusCode(HttpStatus.SC_OK);
			}		

		} else {
			response.setStatusCode(HttpStatus.SC_BAD_REQUEST);
			log.error("Request with invalid content, Content-Type must be 'text/xml'");

		}
		if (requestHandledSuccessfull){
			log.info("handle Mom2 request...done");	
		}else {
			log.info("handle Mom2 request...failed");
		}
		

	}

}
