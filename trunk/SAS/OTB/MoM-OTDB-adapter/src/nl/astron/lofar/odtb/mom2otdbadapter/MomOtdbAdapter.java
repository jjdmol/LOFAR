package nl.astron.lofar.odtb.mom2otdbadapter;

import java.io.IOException;
import java.rmi.NotBoundException;
import java.util.TimeZone;

import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.mom2listener.Server;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.OTDBListener;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.OTDBQueueProcessor;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.Queue;

public class MomOtdbAdapter {
	protected String username = null;

	protected String password = null;

	protected String momUrl = null;

	protected String authUrl = null;

	protected String rmiHost = null;

	protected Integer rmiPort = null;

	public MomOtdbAdapter() {

	}

	protected void startServices() throws IOException, NotBoundException {
		TimeZone.setDefault(TimeZone.getTimeZone("UTC")); 
		Queue queue = new Queue();
		OTDBRepository repository = new OTDBRepository(rmiHost, rmiPort
				.intValue());
/*		OTDBQueueProcessor otdbQueueProcessor = new OTDBQueueProcessor(queue,
				username, password, authUrl, momUrl);
		otdbQueueProcessor.start();
*/
		OTDBListener otdbListener = new OTDBListener(queue, 5000, repository);
		otdbListener.start();

		Server server = new Server(repository);
		server.start();
	}

	protected void parseArguments(String[] args) throws Exception {

		if (args.length > 0) {
			for (int i = 0; i < args.length-1; i= i+2) {
				String argument = args[i];
				String value = args[i+1];
				parseArgument(argument, value);

			}
		}
		if (username == null || password == null || authUrl == null
				|| momUrl == null || rmiHost == null || rmiPort == null) {
			throw new Exception();

		}
	}

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

	public void showSyntax() {

		System.out.println("\n--- Syntax ---");
		System.out.println("MomOtdbAdapter " + " -argument <value> ...");
		System.out.println("\n--- Arguments ---");
		System.out.println("-u <webapplication username>");
		System.out.println("-p <webapplication password>");
		System.out.println("-authurl <authorization module url>");
		System.out.println("-mom2url <mom2 url>");
		System.out.println("-rmihost <jOTDB RMI host>");
		System.out.println("-rmiport <jOTDB RMI port>");
		System.out.println("\n---Example ---");
		System.out
				.println("MomOtdbAdapter -u bastiaan -p bastiaan -rmihost lofar17.astron.nl -rmiport 10099 -mom2url http://localhost:8080/mom2 -authurl http://localhost:8080/wsrtauth");
	};
}
