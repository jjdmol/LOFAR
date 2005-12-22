package nl.astron.lofar.odtb.mom2otdbadapter.otdblistener;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import nl.astron.wsrt.util.WsrtConverter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class Queue {
	private Log log = LogFactory.getLog(this.getClass());

	private List tasks = new ArrayList();

	private String taskDir = "./tasks";

	private static final String FILE_DATE_TIME_FORMAT = "yyyy_MM_dd'T'HH_mm_ss";
	private static final String DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss";
	private Date startTime = null;

	private Date endTime = null;
	
	private boolean isTaskLocked = false;
	
	private boolean isTimeLocked = false;

	public Queue() throws IOException {
		File dir = new File(taskDir);
		if (!dir.exists()) {
			dir.mkdir();
		} else {
			String[] files = dir.list();
			for (int i = 0; i < files.length; i++) {
				Task task = new Task();
				String xml = getFile(dir.getAbsolutePath() + File.separator
						+ files[i]);
				task.setXml(xml);
				tasks.add(task);
			}
		}
	}

	public synchronized Task get() {
		while (tasks.size() == 0 && isTaskLocked) {
			log.info("Waiting for tasks....");
			try {
				wait();
			} catch (InterruptedException e) {
			}
		}
		isTaskLocked = true;
		Task task = (Task) tasks.get(0);
		log.info("Processing task...., one task removed. Number of tasks:"
				+ tasks.size());
		notifyAll();
		return task;
	}

	public synchronized void remove(Task task) {
		File file = new File(task.getFileName());
		if (!file.delete()) {
			log.error("Can not delete file:" + task.getFileName());
		}
		log.info("Remove task(" + task.getMom2Id() + ") Number of tasks:"
				+ tasks.size());
		tasks.remove(0);
		isTaskLocked = false;
		notifyAll();
	}

	public synchronized TimePeriod getTimePeriod() throws IOException {
		while (isTimeLocked) {
			try {
				wait();
			} catch (InterruptedException e) {
			}
		}
		isTimeLocked = true;
		String fileName = taskDir + File.separator + "last_time_period.txt";
		String content = getFile(fileName);
		if (content != null) {
			String[] time = content.split(",");
			//long newStartTime = new Long().longValue();
			startTime = WsrtConverter.toDate(time[1],DATE_TIME_FORMAT);
			//startTime.setTime(newStartTime);
			
		} else {
			startTime = new Date();
			startTime.setTime(0);

		}
		endTime = new Date();
		TimePeriod time = new TimePeriod();
		time.setStartTime(startTime);
		time.setEndTime(endTime);
		notifyAll();
		return time;
	}

	public synchronized void saveTimePeriod() throws IOException {
		String fileName = taskDir + File.separator + "last_time_period.txt";
		String content = WsrtConverter.toDateString(startTime,DATE_TIME_FORMAT) + "," + WsrtConverter.toDateString(endTime,DATE_TIME_FORMAT);
		storeString(fileName, content);
		isTimeLocked = false;
		notifyAll();
	}

	public synchronized void add(Task task) throws IOException {
		tasks.add(task);
		log.info("Task added. Number of tasks:" + tasks.size());
		storeTask(task);
		notifyAll();
	}

	protected String getFile(String fileName) throws IOException {
		File file = new File(fileName);
		if (file.exists()) {
			FileInputStream fileInputStream = new FileInputStream(file);
			BufferedReader reader = new BufferedReader(new InputStreamReader(
					fileInputStream));
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

	protected void storeTask(Task task) throws FileNotFoundException,
			IOException {
		Date date = new Date();
		String fileName = taskDir + File.separator + "mom2id_"
				+ task.getMom2Id() + "_time_"
				+ WsrtConverter.toDateString(date, FILE_DATE_TIME_FORMAT) + ".xml";
		task.setFileName(fileName);
		storeString(fileName, task.getXml());
	}

	protected void storeString(String fileName, String content)
			throws FileNotFoundException, IOException {
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
}
