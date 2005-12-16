package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class Queue {
	private Log log = LogFactory.getLog(this.getClass());
	private List tasks = new ArrayList();

	public synchronized Task get() {
		while (tasks.size()==0) {
			log.info("Waiting for tasks....");
			try {
				wait();
			} catch (InterruptedException e) {
			}
		}

		Task task = (Task) tasks.get(0);
		tasks.remove(0);
		log.info("Processing task...., one task removed. Number of tasks:" + tasks.size());
		notifyAll();
		return task;
	}

	public synchronized void add(Task task) {
		tasks.add(task);
		log.info("Task added. Number of tasks:" + tasks.size());
		notifyAll();
	}
}
