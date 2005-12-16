package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class OTDBListener extends Thread {
	private Log log = LogFactory.getLog(this.getClass());

	private int seconds = 100;

	private Queue queue = null;

	/*
	 * seconds to wait
	 */
	public OTDBListener(Queue queue, int seconds) {
		this.seconds = seconds;
		this.queue = queue;
	}

	public void run() {
		while (true) {
			try {
				log.debug("Add new task");
				Task task = new Task();
				task.setXml("Test xml");
				queue.add(task);
				log.debug("Going to sleep");
				Thread.sleep(seconds);
			} catch (InterruptedException e) {
			}
		}
	}
}
