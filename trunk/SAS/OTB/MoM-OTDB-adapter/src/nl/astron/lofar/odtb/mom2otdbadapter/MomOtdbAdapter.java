package nl.astron.lofar.odtb.mom2otdbadapter;

import nl.astron.lofar.odtb.mom2otdbadapter.data.OTDBRepository;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.OTDBListener;
import nl.astron.lofar.odtb.mom2otdbadapter.otdblistener.Queue;


public class MomOtdbAdapter {

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception{
		Queue queue = new Queue();
		//OTDBQueueProcessor otdbQueueProcessor = new OTDBQueueProcessor(queue);
		//otdbQueueProcessor.start();
		OTDBRepository repository = new OTDBRepository("lofar17.astron.nl", 10099);
		OTDBListener otdbListener = new OTDBListener(queue,5000,repository);
		otdbListener.start();
		
		//Server server = new Server(repository);
		//server.start();
	}

}
