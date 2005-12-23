package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.IOException;
import java.rmi.UnmarshalException;
import java.util.List;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class OTDBListener extends Thread {
	private Log log = LogFactory.getLog(this.getClass());

	private int seconds = 100;

	private Queue queue = null;

	private OTDBRepository repository = null;

	/*
	 * seconds to wait
	 */
	public OTDBListener(Queue queue, int seconds, OTDBRepository repository) {
		this.seconds = seconds;
		this.queue = queue;
		this.repository = repository;
	}

	public void run() {
		while (true) {
			try {

				log.debug("Add new tasks");
				TimePeriod timePeriod = queue.getTimePeriod();
				List lofarObservations = repository.getLatestChanges(timePeriod.getStartTime(), timePeriod.getEndTime());
				for (int i = 0; i < lofarObservations.size(); i++) {
					LofarObservation observation = (LofarObservation) lofarObservations
							.get(i);
					Task task = convertToTask(observation);
					if (task != null) {
						queue.add(task);
					}
				}
				queue.saveTimePeriod();
				log.debug("Going to sleep");
				Thread.sleep(seconds);
			} catch (InterruptedException e) {
			} catch (UnmarshalException e) {
				log.error("UnmarshalException: " + e.getMessage(), e);
			}catch (IOException e) {
				log.error("IOException: " + e.getMessage(), e);
			}
		}
	}

	protected Task convertToTask(LofarObservation lofarObservation) {
		try {
			Task task = new Task();
			XMLGenerator xmlGenerator = new XMLGenerator();
			String xml = xmlGenerator.getObservationXml(lofarObservation);
			task.setXml(xml);
			task.setMom2Id(lofarObservation.getMom2Id() + "");
			return task;
		} catch (Exception e) {
			log.error(e.getMessage(),e);

		}
		return null;
	}
}
