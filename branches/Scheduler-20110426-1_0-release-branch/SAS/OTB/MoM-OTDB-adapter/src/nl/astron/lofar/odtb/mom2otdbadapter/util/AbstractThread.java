package nl.astron.lofar.odtb.mom2otdbadapter.util;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public abstract class AbstractThread extends Thread {
	private Log log = LogFactory.getLog(this.getClass());
	private static final int MILLISECONDS = 1000;

	private int milliseconds = 5 * MILLISECONDS;

	public AbstractThread(int seconds) {
		this.milliseconds = seconds * MILLISECONDS;
	}

	@Override
	public final void run() {
		boolean exceptionThrowed = false;
		while (true) {
			try {
				doRun();
				exceptionThrowed = false;
			} catch (Exception e) {
				if (!exceptionThrowed) {
					exceptionThrowed = true;
					log.fatal("Fatal exception occurred: " + e.getMessage(), e);
				} else {
					log.error("Fatal exception occurred: " + e.getMessage());
				}

			} finally {
				sleep();
			}
		}
	}

	protected abstract void doRun() throws Exception;

	/**
	 * If some error occured, the task executor going to sleep for a while
	 */
	protected void sleep() {
		try {
			if (log.isDebugEnabled()) {
				log.debug("Going to sleep: " + (milliseconds / MILLISECONDS));
			}
			Thread.sleep(milliseconds);
		} catch (InterruptedException e) {
		}
	}
}
