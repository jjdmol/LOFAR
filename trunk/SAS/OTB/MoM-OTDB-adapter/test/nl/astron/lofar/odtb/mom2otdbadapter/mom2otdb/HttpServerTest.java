package nl.astron.lofar.odtb.mom2otdbadapter.mom2otdb;


public class HttpServerTest {


	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
		Mom2HttpRequestHandler handler = new Mom2HttpRequestHandler();
		HttpServer server = new HttpServer(443, handler);
		server.start();

	}
}
