package nl.astron.lofar.odtb.mom2otdbadapter;

import java.io.File;
import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.util.Locale;
import java.util.TimeZone;

import nl.astron.lofar.odtb.mom2otdbadapter.config.ConfigXMLParser;
import nl.astron.lofar.odtb.mom2otdbadapter.config.Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.config.OTDBConfiguration;
import nl.astron.lofar.odtb.mom2otdbadapter.config.StubConfiguration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.Repository;
import nl.astron.lofar.odtb.mom2otdbadapter.data.StubRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.mom2listener.HttpServer;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.OTDBListener;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.Queue;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.TaskExecutor;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.log4j.PropertyConfigurator;

public class MomOtdbAdapter {

	private static Log log = LogFactory.getLog(MomOtdbAdapter.class);

	/**
	 * Starts all services
	 * 
	 * @throws IOException
	 * @throws CertificateException
	 * @throws NoSuchAlgorithmException
	 * @throws KeyStoreException
	 * @throws KeyManagementException
	 * @throws UnrecoverableKeyException
	 */
	protected void startServices(Configuration config) throws IOException, UnrecoverableKeyException,
			KeyManagementException, KeyStoreException, NoSuchAlgorithmException, CertificateException {
		TimeZone.setDefault(TimeZone.getTimeZone("UTC"));
		Locale.setDefault(Locale.US);
		Queue queue = new Queue();
		Repository repository = null;
		if (config.getRepository() instanceof StubConfiguration) {
			repository = new StubRepository();
		} else if (config.getRepository() instanceof OTDBConfiguration) {
			repository = new OTDBRepository((OTDBConfiguration) config.getRepository());
		}
		TaskExecutor otdbQueueProcessor = new TaskExecutor(queue, config.getMom2());
		otdbQueueProcessor.start();

		OTDBListener otdbListener = new OTDBListener(queue, config, repository);
		otdbListener.start();

		HttpServer server = new HttpServer(config, repository);
		server.start();
	}


	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
		File log4jLocation = null;
		File configLocation = null;
		boolean valid = true;
		if (args.length > 0) {
			for (int i = 0; i < args.length - 1; i = i + 2) {
				String argument = args[i];
				String value = args[i + 1];
				if (argument.equals("-l")) {
					log4jLocation = new File(value);
				} else if (argument.equals("-c")) {
					configLocation = new File(value);
				}
			}
		}
		if (configLocation == null) {
			valid = false;
			showSyntax();
		} else {
			if (!configLocation.exists()) {
				System.out.println(configLocation.getPath() + " does not exist");
				valid = false;
			}
		}
		if (valid) {
			if (log4jLocation != null && !log4jLocation.exists()) {
				System.out.println(log4jLocation.getPath() + " does not exist");
			}
		}
		if (valid) {
			MomOtdbAdapter adapter = new MomOtdbAdapter();

			try {
				if (log4jLocation != null) {
					PropertyConfigurator.configure(log4jLocation.getAbsolutePath());

					log.info("MomOtdbAdapter started. log4j: " + log4jLocation.getAbsolutePath());
				} else {
					log.info("MomOtdbAdapter started.");
				}
				adapter.startServices(ConfigXMLParser.parse(configLocation));
			} catch (Exception e) {
				log.error(e.getMessage(), e);
				System.exit(0);
			}
		} else {
			System.out.println("Unable to start MomOtdbAdapter");
			System.exit(0);
		}
	}

	/**
	 * Shows the syntax of this program
	 */
	public static void showSyntax() {

		System.out.println("\n--- Syntax ---");
		System.out.println("java -jar mom-otdb-adapter.jar " + " -argument <value> ...");
		System.out.println("\n--- Arguments ---");
		System.out.println("-c <config file>");
		System.out.println("-l <log4j.properties file>");
		System.out.println("\n---Example ---");
		System.out.println("java -jar mom-otdb-adapter.jar -c ./config.xml -l ./log4j.properties");
	};
}
