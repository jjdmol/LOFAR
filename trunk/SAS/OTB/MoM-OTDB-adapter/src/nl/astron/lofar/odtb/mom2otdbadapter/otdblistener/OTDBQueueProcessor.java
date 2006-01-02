package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.IOException;

import nl.astron.util.http.AstronHttpClient;
import nl.astron.util.http.exception.AstronHttpException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class OTDBQueueProcessor extends Thread {
	private static final int SLEEP_MILLISECONDS = 30000;

	private Log log = LogFactory.getLog(this.getClass());

	private AstronHttpClient httpClient = null;

	private Queue queue = null;

	private String username = null;

	private String password = null;

	private String momUrl = null;
	


	/*
	 * seconds to wait
	 */
	public OTDBQueueProcessor(Queue queue, String username, String password,
			String authUrl, String momUrl) {
		this.username = username;
		this.password = password;
		this.momUrl = momUrl;
		this.queue = queue;
		httpClient = new AstronHttpClient(authUrl);
	}

	public void run() {
		while (true) {
			Task task = queue.get();
			log.debug("Process task: " + task.getXml());
			processTask(task);
			queue.remove(task);

		}

	}

	protected void processTask(Task task) {
		boolean succeed = false;
		while (!succeed) {
			try {
				httpClient.login(username, password);
				String[] paramNames = new String[]{"command", "xmlcontent" } ;
				String[] paramValues = new String[] {"importxml2", task.getXml()};

				String result = httpClient
						.getResponseAsString(momUrl
								+ "/interface/importMom2XML.do", paramNames,paramValues);
				log.info(result);
				httpClient.logout();
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
