package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.StringReader;

import nl.astron.util.AstronValidator;
import nl.astron.util.http.client.AstronHttpClient;
import nl.astron.util.http.client.HttpClientConfig;
import nl.astron.util.http.client.handler.StringResponseHandler;
import nl.astron.util.http.exception.AstronHttpException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.http.params.BasicHttpParams;
import org.apache.xerces.parsers.DOMParser;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.xml.sax.InputSource;

/**
 * Execute the tasks that are in the queue
 * 
 * @author Bastiaan Verhoef
 *
 */
public class TaskExecutor extends Thread {
	private static final int SLEEP_MILLISECONDS = 30000;

	private Log log = LogFactory.getLog(this.getClass());

	private AstronHttpClient httpClient = null;

	private Queue queue = null;

	private String username = null;

	private String password = null;

	private String importXMLUrl = null;

	/*
	 * seconds to wait
	 */
	/**
	 * Constructor 
	 * @param queue Queue to retrieve tasks
	 * @param username username of mom2
	 * @param password password of mom2
	 * @param authUrl authorization module url
	 * @param momUrl mom2 module url
	 */
	public TaskExecutor(Queue queue, String username, String password,
			String authUrl, String momUrl) {
		this.username = username;
		this.password = password;
		this.importXMLUrl = momUrl + "/interface/importMom2XML.do";
		this.queue = queue;
		HttpClientConfig config = new HttpClientConfig(authUrl);
		httpClient = new AstronHttpClient(config);
	}
	/**
	 * start the taskExecutor thread
	 */
	public void run() {
		while (true) {
			/*
			 * get new task, if no tasks available, wait until new tasks added
			 */
			Task task = queue.get();
			log.debug("Process task: " + task.getXml());
			/*
			 * process task
			 * if task process successfull, remove tasks
			 * else move task to the end of the task list
			 */
			if (processTask(task)) {
				queue.remove(task);
			} else {
				queue.moveToEndOfTaskList(task);
			}

		}

	}

	/**
	 * Process task. It logs in to MoM2, it post the task xml to MoM2 and logouts.
	 * @param task tass to process
	 * @return If MoM2 result is ok, it returns true
	 */
	protected boolean processTask(Task task) {
		boolean succeed = false;
		try {
			httpClient.login(username, password);
			BasicHttpParams params = new BasicHttpParams();
			params.setParameter("command", "xmlcontent");
			params.setParameter("importxml2",task.getXml());
			String result = httpClient.doPost(importXMLUrl, params,null, new StringResponseHandler());
			log.info(result);
			httpClient.logout();
			return isSucceed(result);
		} catch (AstronHttpException ahe) {
			log.error("AstronHttpException:" + ahe.getMessage(), ahe);
			sleep();
		} 
		return succeed;
	}

	/**
	 * If some error occured, the task executor going to sleep for a while
	 */
	protected void sleep() {
		try {
			log.debug("Wait some seconds");
			Thread.sleep(SLEEP_MILLISECONDS);
		} catch (InterruptedException e) {
		}
	}

	/**
	 * Analyzed the mom2 output and return if the task is done
	 * @param string
	 * @return true, if task is successfull executed
	 */
	protected boolean isSucceed(String string) {
		try {
			Document document = convertStringToDocument(string);
			Node element = document.getDocumentElement();
			if (equal(element, "mom:result")) {
				for (int i = 0; i < element.getChildNodes().getLength(); i++) {
					Node nodeChild = element.getChildNodes().item(i);
					if (equal(nodeChild, "errors")) {
						String value = getValue(nodeChild);
						if (!AstronValidator.isBlankOrNull(value)) {
							log.error("Mom2 returns errors: "
									+ value);
							return false;
						} else {
							return true;
						}
					}
				}
			}

		} catch (Exception e) {
			log.fatal("Exception: " + e.getMessage(), e);
		}
		return false;
	}

	/**
	 * Converts a xml to a document
	 * @param myXML
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
	 * The equal method compares if an node has the given name
	 * 
	 * @param node
	 * @param nodeName
	 * @return true, if equal
	 */
	protected boolean equal(Node node, String nodeName) {
		return node.getNodeName().equals(nodeName);
	}

	/**
	 * The getValue method returns the value of an node
	 * 
	 * @param node
	 * @return value of the node
	 */
	protected String getValue(Node node) {
		String value = null;
		if (node.getFirstChild() != null) {
			value = node.getFirstChild().getNodeValue();
			if (log.isDebugEnabled()) {
				log.debug("Node: " + node.getNodeName() + " value: " + value);
			}
		}
		return value;
	}
}
