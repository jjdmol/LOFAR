package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.IOException;

import nl.astron.util.http.AstronHttpClient;
import nl.astron.util.http.exception.AstronHttpException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class OTDBQueueProcessor extends Thread {
	private static final int SLEEP_MILLISECONDS = 5000;

	private Log log = LogFactory.getLog(this.getClass());

	private AstronHttpClient httpClient = null;

	private Queue queue = null;

	/*
	 * seconds to wait
	 */
	public OTDBQueueProcessor(Queue queue) {
		this.queue = queue;
	}

	public void run() {
		try {
			httpClient = new AstronHttpClient("http", "localhost", 8080,
					"/wsrtauth");
			httpClient.login("bastiaan", "bastiaan");
			while (true) {
				Task task = queue.get();
				log.debug("Process task: " + task.getXml());
				processTask(task);

			}
		} catch (AstronHttpException ahe) {
			log.error("AstronHttpException:" + ahe.getMessage(), ahe);

		}

	}

	protected void processTask(Task task) {
		boolean succeed = false;
		while (!succeed) {
			try {

				String result = httpClient.getResponseAsString("http", 8080,
						"/mom2",
						"/interface/importXML.do?command=IMPORTXML&xmlcontent="
								+ "test");
				//ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(result);
				//String string = new String(result);
				log.info(result);
				
				succeed = true;
			} catch (AstronHttpException ahe) {
				log.error("AstronHttpException:" + ahe.getMessage(), ahe);
				sleep();
			} catch (IOException ahe) {
				log.error("IOException:" + ahe.getMessage(), ahe);
				sleep();
			}
		}
	}

	protected void sleep() {
		try {
			log.debug("Wait some seconds");
			Thread.sleep(SLEEP_MILLISECONDS);
		} catch (InterruptedException e) {
		}
	}
}
