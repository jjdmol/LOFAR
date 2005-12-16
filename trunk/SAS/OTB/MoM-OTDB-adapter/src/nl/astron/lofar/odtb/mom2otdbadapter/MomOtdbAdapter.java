package nl.astron.lofar.odtb.mom2otdbadapter;

import nl.astron.lofar.odtb.mom2otdbadapter.mom2listener.Server;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.OTDBListener;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.OTDBQueueProcessor;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.Queue;


public class MomOtdbAdapter {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
/*		Queue queue = new Queue();
		OTDBQueueProcessor otdbQueueProcessor = new OTDBQueueProcessor(queue);
		otdbQueueProcessor.start();
		OTDBListener otdbListener = new OTDBListener(queue,5000);
		otdbListener.start();*/
		
		Server server = new Server();
		server.start();
	}

}
