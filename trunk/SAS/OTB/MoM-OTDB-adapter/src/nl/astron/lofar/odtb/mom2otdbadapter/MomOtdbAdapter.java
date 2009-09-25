package nl.astron.lofar.odtb.mom2otdbadapter;

import java.io.IOException;
import java.rmi.NotBoundException;
import java.util.TimeZone;

import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.OTDBListener;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.Queue;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.TaskExecutor;

public class MomOtdbAdapter {
	protected String username = null;

	protected String password = null;

	protected String momUrl = null;

	protected String authUrl = null;

	protected String rmiHost = null;

	protected Integer rmiPort = null;
	
	protected Integer seconds = null;

	public MomOtdbAdapter() {

	}

	/**
	 * Starts all services
	 * @throws IOException
	 * @throws NotBoundException
	 */
	protected void startServices() throws IOException, NotBoundException {
		TimeZone.setDefault(TimeZone.getTimeZone("UTC")); 
		Queue queue = new Queue();
		OTDBRepository repository = new OTDBRepository(rmiHost, rmiPort
				.intValue());
		TaskExecutor otdbQueueProcessor = new TaskExecutor(queue,
				username, password, authUrl, momUrl);
		otdbQueueProcessor.start();

//		OTDBListener otdbListener = new OTDBListener(queue, seconds.intValue()*1000, repository);
//		otdbListener.start();

		//Mom2Listener server = new Mom2Listener(repository);
		//server.start();
	}

	/**
	 * Parse arguments
	 * @param args
	 * @throws Exception
	 */
	protected void parseArguments(String[] args) throws Exception {

		if (args.length > 0) {
			for (int i = 0; i < args.length-1; i= i+2) {
				String argument = args[i];
				String value = args[i+1];
				parseArgument(argument, value);

			}
		}
		if (username == null || password == null || authUrl == null
				|| momUrl == null || rmiHost == null || rmiPort == null || seconds == null) {
			throw new Exception();

		}
	}

	/**
	 * Parse one argument
	 * @param argument
	 * @param value
	 * @throws Exception
	 */
	protected void parseArgument(String argument, String value)
			throws Exception {
		if (argument.equals("-u")) {
			username = value;
		} else if (argument.equals("-p")) {
			password = value;
		} else if (argument.equals("-authurl")) {
			authUrl = value;
		} else if (argument.equals("-mom2url")) {
			momUrl = value;
		} else if (argument.equals("-rmihost")) {
			rmiHost = value;
		} else if (argument.equals("-rmiport")) {
			rmiPort = new Integer(value);
		}else if (argument.equals("-rmiseconds")) {
			seconds = new Integer(value);
		}
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
		MomOtdbAdapter adapter = new MomOtdbAdapter();
		try {
			adapter.parseArguments(args);
		} catch (Exception e) {
			adapter.showSyntax();
			System.exit(0);
		}
		adapter.startServices();

	}

	/**
	 * Shows the syntax of this program
	 */
	public void showSyntax() {

		System.out.println("\n--- Syntax ---");
		System.out.println("java -jar mom-otdb-adapter.jar " + " -argument <value> ...");
		System.out.println("\n--- Arguments ---");
		System.out.println("-u <webapplication username>");
		System.out.println("-p <webapplication password>");
		System.out.println("-authurl <authorization module url>");
		System.out.println("-mom2url <mom2 url>");
		System.out.println("-rmihost <jOTDB RMI host>");
		System.out.println("-rmiport <jOTDB RMI port>");
		System.out.println("-rmiseconds <checks jOTDB with a interval of given amount of seconds>");
		System.out.println("\n---Example ---");
		System.out
				.println("java -jar mom-otdb-adapter.jar -u bastiaan -p bastiaan -rmihost lofar17.astron.nl -rmiport 10099 -rmiseconds 5 -mom2url http://localhost:8080/mom2 -authurl http://localhost:8080/wsrtauth ");
	};
}
