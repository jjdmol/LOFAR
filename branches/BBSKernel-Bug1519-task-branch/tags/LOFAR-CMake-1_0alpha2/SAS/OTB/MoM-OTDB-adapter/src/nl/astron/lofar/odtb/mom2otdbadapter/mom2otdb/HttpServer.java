package nl.astron.lofar.odtb.mom2otdbadapter.mom2otdb;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.URL;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLEngine;
import javax.net.ssl.SSLException;
import javax.net.ssl.SSLSession;
import javax.net.ssl.TrustManagerFactory;

import org.apache.http.impl.DefaultConnectionReuseStrategy;
import org.apache.http.impl.DefaultHttpResponseFactory;
import org.apache.http.impl.nio.SSLServerIOEventDispatch;
import org.apache.http.impl.nio.reactor.DefaultListeningIOReactor;
import org.apache.http.impl.nio.reactor.SSLIOSessionHandler;
import org.apache.http.nio.protocol.BufferingHttpServiceHandler;
import org.apache.http.nio.reactor.IOEventDispatch;
import org.apache.http.nio.reactor.ListeningIOReactor;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.CoreConnectionPNames;
import org.apache.http.params.CoreProtocolPNames;
import org.apache.http.params.HttpParams;
import org.apache.http.protocol.BasicHttpProcessor;
import org.apache.http.protocol.HttpRequestHandler;
import org.apache.http.protocol.HttpRequestHandlerRegistry;
import org.apache.http.protocol.ResponseConnControl;
import org.apache.http.protocol.ResponseContent;
import org.apache.http.protocol.ResponseDate;
import org.apache.http.protocol.ResponseServer;

public class HttpServer {

	private int port;

	private HttpRequestHandler httpRequestHandler;

	private SSLIOSessionHandler sslSessionHandler;

	public HttpServer(int port, HttpRequestHandler httpRequestHandler, SSLIOSessionHandler sslSessionHandler) {
		this.port = port;
		this.httpRequestHandler = httpRequestHandler;
		this.sslSessionHandler = sslSessionHandler;
	}

	public HttpServer(int port, HttpRequestHandler httpRequestHandler) {
		this(port, httpRequestHandler, new ClientAuthSSLIOSessionHandler());
	}
	
	public void start() throws KeyStoreException, NoSuchAlgorithmException, CertificateException, IOException, UnrecoverableKeyException, KeyManagementException{
		ClassLoader cl = this.getClass().getClassLoader();
		URL url = cl.getResource("keystore-server.jks");
		KeyStore keystore = KeyStore.getInstance("jks");
		keystore.load(url.openStream(), "server".toCharArray());
		KeyManagerFactory kmfactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
		kmfactory.init(keystore, "server".toCharArray());
		KeyManager[] keymanagers = kmfactory.getKeyManagers();
	    TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm()); 
	    tmf.init(keystore); 		
		SSLContext sslcontext = SSLContext.getInstance("TLS");
		sslcontext.init(keymanagers, tmf.getTrustManagers(), null);

		HttpParams params = new BasicHttpParams();
		params.setIntParameter(CoreConnectionPNames.SO_TIMEOUT, 5000).setIntParameter(
				CoreConnectionPNames.SOCKET_BUFFER_SIZE, 8 * 1024).setBooleanParameter(
				CoreConnectionPNames.STALE_CONNECTION_CHECK, false).setBooleanParameter(
				CoreConnectionPNames.TCP_NODELAY, true).setParameter(CoreProtocolPNames.ORIGIN_SERVER,
				"Jakarta-HttpComponents-NIO/1.1");

		BasicHttpProcessor httpproc = new BasicHttpProcessor();
		httpproc.addInterceptor(new ResponseDate());
		httpproc.addInterceptor(new ResponseServer());
		httpproc.addInterceptor(new ResponseContent());
		httpproc.addInterceptor(new ResponseConnControl());

		BufferingHttpServiceHandler handler = new BufferingHttpServiceHandler(httpproc,
				new DefaultHttpResponseFactory(), new DefaultConnectionReuseStrategy(), params);

		// Set up request handlers
		HttpRequestHandlerRegistry reqistry = new HttpRequestHandlerRegistry();
		reqistry.register("*", httpRequestHandler);

		handler.setHandlerResolver(reqistry);

		// Provide an event logger
		// handler.setEventListener(new EventLogger());

		IOEventDispatch ioEventDispatch = new SSLServerIOEventDispatch(handler, sslcontext,  sslSessionHandler, params);

		ListeningIOReactor ioReactor = new DefaultListeningIOReactor(2, params);

		ioReactor.listen(new InetSocketAddress(port));
		ioReactor.execute(ioEventDispatch);
	}
	
	

	static class ClientAuthSSLIOSessionHandler implements SSLIOSessionHandler {
		public void initalize(SSLEngine sslengine, HttpParams params) throws SSLException {
			// // Ask clients to authenticate
			sslengine.setWantClientAuth(true);
		}

		public void verify(SocketAddress remoteAddress, SSLSession session) throws SSLException {
			// no extra authentication
		}
	}

}
