package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.StringReader;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.Repository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.RepositoryException;
import nl.astron.util.XMLConverter;

import org.apache.http.HttpEntity;
import org.apache.http.HttpEntityEnclosingRequest;
import org.apache.http.HttpRequest;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.protocol.HttpContext;
import org.apache.http.protocol.HttpRequestHandler;
import org.apache.http.util.EntityUtils;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

public class Mom2HttpRequestHandler implements HttpRequestHandler {

	private Repository repository;
	public Mom2HttpRequestHandler(Repository repository){
		this.repository = repository;
	}
	
	@Override
	public void handle(HttpRequest request, HttpResponse response, HttpContext context) {
        if (request instanceof HttpEntityEnclosingRequest) {
            HttpEntity entity = ((HttpEntityEnclosingRequest) request).getEntity();

			try {
				String content = EntityUtils.toString(entity);
				InputSource inputSource = new InputSource(new StringReader(content));
				Document document = XMLConverter.convertXMLToDocument(inputSource);
				LofarObservation lofarObservation = XMLParser
						.getLofarObservation(document);
				repository.store(lofarObservation);
				response.setStatusCode(HttpStatus.SC_OK);				
			}catch (RepositoryException e) {
				response.setStatusCode(HttpStatus.SC_INTERNAL_SERVER_ERROR);
			}catch (Exception e) {
				response.setStatusCode(HttpStatus.SC_BAD_REQUEST);
			}

			response.setStatusCode(HttpStatus.SC_OK);	
            
        }else {
        	response.setStatusCode(HttpStatus.SC_BAD_REQUEST);
        }
		

	}


}
