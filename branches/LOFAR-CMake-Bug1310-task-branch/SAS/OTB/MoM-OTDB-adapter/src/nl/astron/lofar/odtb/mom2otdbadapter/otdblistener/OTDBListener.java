package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.IOException;
import java.rmi.UnmarshalException;
import java.util.List;

import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Polls if there are changes in the OTDB database.
 * @author Bastiaan Verhoef
 *
 */
public class OTDBListener extends Thread {
	private Log log = LogFactory.getLog(this.getClass());

	private int milliseconds = 1000;

	private Queue queue = null;

	private OTDBRepository repository = null;


	/**
	 * Constructor
	 * @param queue Queue where this listener add the tasks
	 * @param milliseconds interval (in milliseconds) between the retrieval of the changes
	 * @param repository OTDBRepository where this listener retrieves the changes
	 */
	public OTDBListener(Queue queue, int milliseconds, OTDBRepository repository) {
		this.milliseconds = milliseconds;
		this.queue = queue;
		this.repository = repository;
	}

	/**
	 *  Starts the OTDBListener and retrieves changes with an interval 
	 */
	public void run() {
		
		while (true) {
			try {

				log.debug("Add new tasks");
				/*
				 * get time period of this retrieval
				 */
				TimePeriod timePeriod = queue.getTimePeriod();
				/*
				 * look for new changes in this time period
				 */
				List lofarObservations = repository.getLatestChanges(timePeriod.getStartTime(), timePeriod.getEndTime());
				/*
				 * convert retrieved observations to tasks and add the tasks to the queue
				 */
				for (int i = 0; i < lofarObservations.size(); i++) {
					LofarObservation observation = (LofarObservation) lofarObservations
							.get(i);
					Task task = convertToTask(observation);
					if (task != null) {
						queue.add(task);
					}
				}
				/*
				 * save the time period of this retrieval
				 */
				queue.saveTimePeriod();
				log.debug("Going to sleep");
				/*
				 * sleep a given milliseconds
				 */
				Thread.sleep(milliseconds);
			} catch (InterruptedException e) {
			} catch (UnmarshalException e) {
				log.error("UnmarshalException: " + e.getMessage(), e);
			}catch (IOException e) {
				log.error("IOException: " + e.getMessage(), e);
			}
		}
	}

	/**
	 * Converts a observation to a task using the xml generation
	 * @param lofarObservation
	 * @return Task
	 */
	protected Task convertToTask(LofarObservation lofarObservation) {
		try {
			Task task = new Task();
			XMLGenerator xmlGenerator = new XMLGenerator();
			String xml = xmlGenerator.getObservationXml(lofarObservation);
			task.setXml(xml);
			task.setMom2Id(lofarObservation.getMom2Id() + "");
			task.setTime(lofarObservation.getTimeStamp());
			return task;
		} catch (Exception e) {
			log.error(e.getMessage(),e);

		}
		return null;
	}
}
