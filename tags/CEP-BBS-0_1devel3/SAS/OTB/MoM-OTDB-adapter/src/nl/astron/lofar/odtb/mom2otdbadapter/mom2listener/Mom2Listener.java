package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.IOException;
import java.net.ServerSocket;

import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Listen at port 4444 for mom request
 * @author Bastiaan Verhoef
 *
 */
public class Mom2Listener {
	private Log log = LogFactory.getLog(this.getClass());
	private OTDBRepository repository;
	
	/**
	 * Constructor
	 * @param repository OTDBRepository to use
	 */
	public Mom2Listener(OTDBRepository repository){
		this.repository =repository;
	}

	/**
	 * Starts a ServerSocket on port 4444
	 * If a clients connect, it starts a new class ProcessMom2Connection
	 */
	public void start() {
		ServerSocket server = null;
		try {
			log.info("Starting server on port 4444");
			server = new ServerSocket(4444);
		} catch (IOException e) {
			log.fatal("Could not listen on port 4444", e);
		}
		while (true) {
			ProcessMom2Connection w;
			try {
				// server.accept returns a client connection
				w = new ProcessMom2Connection(repository, server.accept());
				w.start();
			} catch (IOException e) {
				log.error("Accept failed: 4444", e);
			}
		}
	}

}
