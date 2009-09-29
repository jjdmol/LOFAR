package nl.astron.lofar.odtb.mom2otdbadapter;

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.util.TimeZone;

import nl.astron.lofar.odtb.mom2otdbadapter.config.AdapterConfiguration;
import nl.astron.lofar.odtb.mom2otdbadapter.config.Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.config.Mom2Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.config.OTDBConfiguration;
import nl.astron.lofar.odtb.mom2otdbadapter.config.StubConfiguration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.Repository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.StubRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.mom2otdb.HttpServer;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.OTDBListener;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.Queue;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.TaskExecutor;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class MomOtdbAdapter {

	private Log log = LogFactory.getLog(MomOtdbAdapter.class);

	/**
	 * Starts all services
	 * @throws IOException 
	 * @throws CertificateException 
	 * @throws NoSuchAlgorithmException 
	 * @throws KeyStoreException 
	 * @throws KeyManagementException 
	 * @throws UnrecoverableKeyException 
	 */
	protected void startServices(Configuration config) throws IOException, UnrecoverableKeyException, KeyManagementException, KeyStoreException, NoSuchAlgorithmException, CertificateException {
		TimeZone.setDefault(TimeZone.getTimeZone("UTC")); 
		Queue queue = new Queue();
		Repository repository = null;
		if (config.getRepository() instanceof StubConfiguration){
			repository = new StubRepository();
		}else if (config.getRepository() instanceof OTDBConfiguration){
			repository = new OTDBRepository((OTDBConfiguration) config.getRepository());
		}
		TaskExecutor otdbQueueProcessor = new TaskExecutor(queue,
				config.getMom2());
		otdbQueueProcessor.start();

		OTDBListener otdbListener = new OTDBListener(queue, config, repository);
		otdbListener.start();

		HttpServer server = new HttpServer(config, repository);
		server.start();
		//Mom2Listener server = new Mom2Listener(repository);
		//server.start();
	}

//	/**
//	 * Parse arguments
//	 * @param args
//	 * @throws Exception
//	 */
//	protected void parseArguments(String[] args) throws Exception {
//
//		if (args.length > 0) {
//			for (int i = 0; i < args.length-1; i= i+2) {
//				String argument = args[i];
//				String value = args[i+1];
//				parseArgument(argument, value);
//
//			}
//		}
//	}

//	/**
//	 * Parse one argument
//	 * @param argument
//	 * @param value
//	 * @throws Exception
//	 */
//	protected void parseArgument(String argument, String value)
//			throws Exception {
//		if (argument.equals("-u")) {
//			username = value;
//		}
//	}

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
		MomOtdbAdapter adapter = new MomOtdbAdapter();
		
		try {
//			adapter.parseArguments(args);
//            PropertyConfigurator.configure(logConfig);
//            jInitCPPLogger aCPPLogger=new jInitCPPLogger(logConfig);
//            logger.info("jOTDBServer started. LogPropFile: "+ logConfig);
			Configuration configuration = new Configuration();
			StubConfiguration stubConfiguration = new StubConfiguration();
			stubConfiguration.setInterval(5);
			configuration.setRepository(stubConfiguration);
			Mom2Configuration momConfiguration = new Mom2Configuration();
			momConfiguration.setUsername("bastiaan");
			momConfiguration.setPassword("bastiaan");
			momConfiguration.setAuthUrl("http://localhost:8080/useradministration");
			momConfiguration.setMom2SchemasUrl("http://localhost:8080/mom2lofar/schemas/");
			momConfiguration.setMom2ImportUrl("http://localhost:8080/mom2lofar/interface/importXML2.do");
			configuration.setMom2(momConfiguration);
			AdapterConfiguration adapterConfiguration = new AdapterConfiguration();
			adapterConfiguration.setHttpPort(8081);
			adapterConfiguration.setTrustedKeystoreLocation("c:/mom-otdb-adapter-trusted-keystore.jks");
			adapterConfiguration.setTrustedKeystorePassword("adapter-trusted");
			adapterConfiguration.setKeystoreLocation("c:/mom-otdb-adapter-keystore.jks");
			adapterConfiguration.setKeystorePassword("adapter");
			configuration.setAdapter(adapterConfiguration);
			adapter.startServices(configuration);
		} catch (Exception e) {
			e.printStackTrace();
			adapter.showSyntax();
			System.exit(0);
		}
		

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
