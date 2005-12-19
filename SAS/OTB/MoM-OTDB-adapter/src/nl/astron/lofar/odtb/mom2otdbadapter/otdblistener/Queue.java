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
	private static final String DATE_TIME_FORMAT = "yyyy_MM_dd'T'HH_mm_ss";
	public Queue() throws IOException{
		File dir = new File(taskDir);
		if (!dir.exists()){
			dir.mkdir();
		}else {
			String[] files = dir.list();
			for (int i = 0; i < files.length;i++){
				Task task = new Task();
				String xml = getFile(dir.getAbsolutePath() + File.separator + files[i]);
				task.setXml(xml);
				tasks.add(task);
			}
		}
	}

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

	public synchronized void add(Task task) throws IOException{
		tasks.add(task);
		log.info("Task added. Number of tasks:" + tasks.size());
		storeTask(task);
		
		notifyAll();
	}
	protected String getFile(String fileName) throws IOException{
		File file = new File(fileName);
		FileInputStream fileInputStream = new FileInputStream(file);
		BufferedReader reader = new BufferedReader(new InputStreamReader(fileInputStream));
		String line = null;
		StringBuffer stringBuffer = new StringBuffer();
		while ((line = reader.readLine()) != null) {
			stringBuffer.append(line);
		}
		fileInputStream.close();
		return stringBuffer.toString();
	}
	protected void storeTask(Task task) throws FileNotFoundException, IOException{
		Date date  = new Date();
		String fileName = taskDir + File.separator + "mom2id_" + task.getMom2Id() + "_time_" + WsrtConverter.toDateString(date,DATE_TIME_FORMAT)+ ".xml";
		File file = new File(fileName);
		file.createNewFile();
		FileOutputStream fileOutputStream = new FileOutputStream(file);
		PrintWriter out = new PrintWriter(fileOutputStream, true);
		out.write(task.getXml());
		out.close();
		fileOutputStream.close();
	}
}
