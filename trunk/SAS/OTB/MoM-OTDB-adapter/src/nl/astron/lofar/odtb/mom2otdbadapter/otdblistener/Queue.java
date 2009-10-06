package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import nl.astron.util.AstronConverter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Queue of tasks
 * 
 * @author Bastiaan Verhoef
 * 
 */
public class Queue {
	private Log log = LogFactory.getLog(this.getClass());

	private List<Task> tasks = new ArrayList<Task>();

	private String taskDir = "./tasks";

	private static final String FILE_DATE_TIME_FORMAT = "yyyy_MM_dd'T'HH_mm_ss";

	private static final String DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss";

	// private static final String OTDB_TIME_FORMAT = "yyyy-MMM-dd HH:mm:ss";

	private Date startTime = null;

	// private Date endTime = null;

	// private boolean isTaskLocked = false;

	/**
	 * Constructor that looks for tasks (*.xml) in the ./tasks directory and
	 * load them.
	 * 
	 * @throws IOException
	 */
	public Queue() throws IOException {
		File dir = new File(taskDir);
		/*
		 * it the tasks dir not exists, create it
		 */
		if (!dir.exists()) {
			dir.mkdir();
		}
		/*
		 * if it exists, look for tasks
		 */
		else {
			String[] files = dir.list(new TasksFilter());
			for (int i = 0; i < files.length; i++) {
				Task task = new Task();
				String fileName = taskDir + File.separator + files[i];
				String xml = getFile(fileName);
				task.setXml(xml);
				task.setFileName(fileName);
				tasks.add(task);
			}
		}
	}

	/**
	 * Retrieve a task, if there are tasks, if there are no tasks, wait until a
	 * task is in the queue
	 * 
	 * @return Task
	 */
	public synchronized Task get() {
		while (tasks.size() == 0) {
			log.info("No task available...");
			try {
				wait();
			} catch (InterruptedException e) {
			}
		}
		Task task = null;
		synchronized (tasks) {
			task = tasks.get(0);
		}
		if (log.isDebugEnabled()) {
			log.debug("Get task (mom2Id: " + task.getMom2Id() + ") ....Number of tasks:" + tasks.size());
		}
		return task;
	}

	/**
	 * Remove a task
	 * 
	 * @param task
	 *            task to be removed
	 */
	public synchronized void remove(Task task) {
		synchronized (tasks) {
			File file = new File(task.getFileName());
			if (!file.delete()) {
				log.error("Can not delete file:" + task.getFileName());
			}
			log.info("Remove task( mom2Id: " + task.getMom2Id() + ") Number of tasks:" + tasks.size());
			tasks.remove(0);
		}
	}

	/**
	 * If a task can not be executed, move it to the end of the tasks list, so
	 * other tasks can be executed.
	 * 
	 * @param task
	 *            task to be moved to the end of the tasks list.
	 */
	public synchronized void moveToEndOfTaskList(Task task) {
		log.info("Can not execute task(" + task.getMom2Id() + ") Number of tasks:" + tasks.size());
		synchronized (tasks) {
			tasks.remove(0);
			tasks.add(task);
		}
	}

	/**
	 * Retrieves the new time period, from the last time period to now
	 * 
	 * @return Time period
	 * @throws IOException
	 */
	public synchronized TimePeriod getTimePeriod() throws IOException {

		String fileName = taskDir + File.separator + "last_time_period.txt";
		String content = getFile(fileName);
		if (content != null) {
			String[] time = content.split(",");
			// long newStartTime = new Long().longValue();
			startTime = AstronConverter.toDate(time[1], DATE_TIME_FORMAT);
			startTime.setSeconds(startTime.getSeconds() + 1);
			// startTime.setTime(newStartTime);

		} else {
			startTime = new Date();
			startTime.setTime(0);

		}
		Date endTime = new Date();
		// endTime.setHours(endTime.getHours()+1);
		TimePeriod time = new TimePeriod();
		time.setStartTime(startTime);
		time.setEndTime(endTime);
		// notifyAll();
		return time;
	}

	/**
	 * Save the time period
	 * 
	 * @throws IOException
	 */
	public synchronized void saveTimePeriod(Date endDate) throws IOException {
		String fileName = taskDir + File.separator + "last_time_period.txt";
		String content = AstronConverter.toDateString(startTime, DATE_TIME_FORMAT) + ","
				+ AstronConverter.toDateString(endDate, DATE_TIME_FORMAT);
		File file = new File(fileName);
		if (!file.exists()) {
			file.createNewFile();
		}
		FileOutputStream fileOutputStream = new FileOutputStream(file);
		PrintWriter out = new PrintWriter(fileOutputStream, true);
		out.write(content);
		out.close();
		fileOutputStream.close();
	}

	/**
	 * Add the task to the task list and store it as a xml file
	 * 
	 * @param task
	 *            Task to be stored
	 * @throws IOException
	 */
	public synchronized void add(Task task) throws IOException {
		synchronized (tasks) {
			tasks.add(task);
			log.info("Add task (mom2Id: " + task.getMom2Id() + ") Number of tasks:" + tasks.size());
			storeTask(task);
		}
		notifyAll();
	}

	/**
	 * Retrieve file by file name
	 * 
	 * @param fileName
	 *            name of the file to be read
	 * @return file as string
	 * @throws IOException
	 */
	protected String getFile(String fileName) throws IOException {
		File file = new File(fileName);
		if (file.exists()) {
			FileInputStream fileInputStream = new FileInputStream(file);
			BufferedReader reader = new BufferedReader(new InputStreamReader(fileInputStream));
			String line = null;
			StringBuffer stringBuffer = new StringBuffer();
			while ((line = reader.readLine()) != null) {
				stringBuffer.append(line);
			}
			fileInputStream.close();
			return stringBuffer.toString();
		} else {
			return null;
		}

	}

	/**
	 * Store a task
	 * 
	 * @param task
	 *            task to be stored
	 * @throws FileNotFoundException
	 * @throws IOException
	 */
	protected void storeTask(Task task) throws FileNotFoundException, IOException {
		String fileName = taskDir + File.separator
				+ AstronConverter.toDateString(task.getTime(), FILE_DATE_TIME_FORMAT) + "mom2id_" + task.getMom2Id()
				+ ".xml";
		task.setFileName(fileName);
		File file = new File(fileName);
		if (!file.exists()) {
			file.createNewFile();
		}
		FileOutputStream fileOutputStream = new FileOutputStream(file);
		PrintWriter out = new PrintWriter(fileOutputStream, true);
		out.write(task.getXml());
		out.close();
		fileOutputStream.close();
	}

	/**
	 * Filters task files
	 * 
	 * @author Bastiaan Verhoef
	 * 
	 */
	class TasksFilter implements FilenameFilter {
		/**
		 * filters task files, tasks files ends on .xml
		 */
		public boolean accept(File dir, String name) {
			return (name.endsWith(".xml"));
		}
	}
}
