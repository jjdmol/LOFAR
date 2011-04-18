package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.IOException;
import java.util.List;

import javax.xml.parsers.ParserConfigurationException;

import nl.astron.lofar.odtb.mom2otdbadapter.config.Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.LofarObservation;
import nl.astron.lofar.odtb.mom2otdbadapter.data.Repository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.RepositoryException;
import nl.astron.lofar.odtb.mom2otdbadapter.util.AbstractThread;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Polls if there are changes in the OTDB database.
 * 
 * @author Bastiaan Verhoef
 * 
 */
public class OTDBListener extends AbstractThread {

	private Log log = LogFactory.getLog(this.getClass());

	private Configuration configuration;

	private Queue queue = null;

	private Repository repository = null;

	/**
	 * Constructor
	 * 
	 * @param queue
	 *            Queue where this listener add the tasks
	 * @param milliseconds
	 *            interval (in milliseconds) between the retrieval of the
	 *            changes
	 * @param repository
	 *            OTDBRepository where this listener retrieves the changes
	 */
	public OTDBListener(Queue queue, Configuration configuration, Repository repository) {
		super(configuration.getRepository().getInterval());
		this.configuration = configuration;
		this.queue = queue;
		this.repository = repository;
		log.info("OTDBListener started with interval of " + configuration.getRepository().getInterval() + " s.");
	}

	/**
	 * Starts the OTDBListener and retrieves changes with an interval
	 * @throws RepositoryException 
	 * @throws IOException 
	 * @throws ParserConfigurationException 
	 */
	public void doRun() throws RepositoryException, IOException, ParserConfigurationException {
		if (log.isDebugEnabled()) {
			log.debug("Looking for new tasks");
		}
		/*
		 * get time period of this retrieval
		 */
		TimePeriod timePeriod = queue.getTimePeriod();
		/*
		 * look for new changes in this time period
		 */
		List<LofarObservation> lofarObservations = repository.getLatestChanges(timePeriod.getStartTime(), timePeriod
				.getEndTime());
		/*
		 * convert retrieved observations to tasks and add the tasks to the
		 * queue
		 */
		for (LofarObservation observation : lofarObservations) {
			Task task = convertToTask(observation);
			if (task != null) {
				queue.add(task);
			}
		}
		if (lofarObservations.size() > 0) {
			/*
			 * save the time period of this retrieval
			 */
			queue.saveTimePeriod(lofarObservations.get(lofarObservations.size() - 1).getTimeStamp());
		}

	}

	/**
	 * Converts a observation to a task using the xml generation
	 * 
	 * @param lofarObservation
	 * @return Task
	 * @throws ParserConfigurationException
	 * @throws IOException
	 */
	protected Task convertToTask(LofarObservation lofarObservation) throws IOException, ParserConfigurationException {
		Task task = new Task();
		String xml = XMLGenerator.getObservationXml(lofarObservation, configuration.getMom2());
		task.setXml(xml);
		task.setMom2Id(lofarObservation.getMom2Id() + "");
		task.setTime(lofarObservation.getTimeStamp());
		return task;
	}

}
