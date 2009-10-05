package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import nl.astron.lofar.odtb.mom2otdbadapter.config.Mom2Configuration;
import nl.astron.util.XMLConverter;
import nl.astron.util.http.client.AstronHttpClient;
import nl.astron.util.http.client.HttpClientConfig;
import nl.astron.util.http.client.handler.StringResponseHandler;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.http.message.BasicNameValuePair;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
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

	private Mom2Configuration config;

	/*
	 * seconds to wait
	 */
	/**
	 * Constructor
	 * 
	 * @param queue
	 *            Queue to retrieve tasks
	 * @param username
	 *            username of mom2
	 * @param password
	 *            password of mom2
	 * @param authUrl
	 *            authorization module url
	 * @param momUrl
	 *            mom2 module url
	 */
	public TaskExecutor(Queue queue, Mom2Configuration config) {
		this.config = config;
		this.queue = queue;
		HttpClientConfig httpClientConfig = new HttpClientConfig(config.getAuthUrl());
		httpClient = new AstronHttpClient(httpClientConfig);
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
			//log.debug("Process task: " + task.getXml());
			/*
			 * process task if task process successfull, remove tasks else move
			 * task to the end of the task list
			 */
			if (processTask(task)) {
				queue.remove(task);
			} else {
				queue.moveToEndOfTaskList(task);
			}

		}

	}

	/**
	 * Process task. It logs in to MoM2, it post the task xml to MoM2 and
	 * logouts.
	 * 
	 * @param task
	 *            tass to process
	 * @return If MoM2 result is ok, it returns true
	 */
	protected boolean processTask(Task task) {
		boolean succeed = false;
		try {
			httpClient.login(config.getUsername(), config.getPassword());
			log.info("Login " + config.getAuthUrl());
			List<BasicNameValuePair> parameters = new ArrayList<BasicNameValuePair>();
			parameters.add(new BasicNameValuePair("command", "IMPORTXML2"));
			parameters.add(new BasicNameValuePair("xmlcontent", task.getXml()));
			log.info("Post xml (mom2Id: " + task.getMom2Id() + ") to mom2 " + config.getMom2ImportUrl() );
			String result = httpClient.doPost(config.getMom2ImportUrl(), parameters, new StringResponseHandler());
			if (log.isDebugEnabled()){
				log.debug(result);
			}
			httpClient.logout();
			log.info("Logout " + config.getAuthUrl());
			return isSucceed(result);
		} catch (IOException e) {
			log.error("IOException:" + e.getMessage(), e);
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
	 * 
	 * @param string
	 * @return true, if task is successfull executed
	 */
	protected boolean isSucceed(String string) {
		boolean result = false;
		try {
			StringReader reader = new StringReader(string);
			Document document = XMLConverter.convertXMLToDocument(new InputSource(reader));
			NodeList errors = document.getElementsByTagName("error");
			if (errors.getLength() > 0) {
				String errorString = "Mom2 returns errors: ";
				for (int i = 0; i < errors.getLength(); i++) {
					if (i >= 1) {
						errorString += "\n";
					}
					NodeList description = ((Element) errors.item(i)).getElementsByTagName("description");
					errorString += getValue(description.item(0));
				}
			}
			result = true;
		} catch (Exception e) {
			log.fatal("Exception: " + e.getMessage(), e);
		}
		return result;
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
