package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
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

import nl.astron.lofar.odtb.mom2otdbadapter.config.Configuration;
import nl.astron.lofar.odtb.mom2otdbadapter.data.Repository;

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

	private static final String JKS = "jks";

	private Configuration config;

	private HttpRequestHandler httpRequestHandler;

	private SSLIOSessionHandler sslSessionHandler;

	public HttpServer(Configuration config, HttpRequestHandler httpRequestHandler, SSLIOSessionHandler sslSessionHandler) {
		this.config = config;
		this.httpRequestHandler = httpRequestHandler;
		this.sslSessionHandler = sslSessionHandler;
	}

	public HttpServer(Configuration config, Repository repository) {
		this(config, new Mom2HttpRequestHandler(repository), new ClientAuthSSLIOSessionHandler());
	}
	
	public HttpServer(Configuration config, HttpRequestHandler httpRequestHandler) {
		this(config, httpRequestHandler, new ClientAuthSSLIOSessionHandler());
	}
	
	public void start() throws KeyStoreException, NoSuchAlgorithmException, CertificateException, IOException, UnrecoverableKeyException, KeyManagementException{
		FileInputStream keyStoreInputStream = new FileInputStream(new File(config.getAdapter().getKeystoreLocation()));
		KeyStore keystore = KeyStore.getInstance(JKS);
		keystore.load(keyStoreInputStream, config.getAdapter().getKeystorePassword().toCharArray());
		KeyManagerFactory kmfactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
		kmfactory.init(keystore, config.getAdapter().getKeystorePassword().toCharArray());
		KeyManager[] keymanagers = kmfactory.getKeyManagers();
		FileInputStream trustedKeyStoreInputStream = new FileInputStream(new File(config.getAdapter().getTrustedKeystoreLocation()));
		KeyStore trustedKeystore = KeyStore.getInstance(JKS);
		trustedKeystore.load(trustedKeyStoreInputStream, config.getAdapter().getTrustedKeystorePassword().toCharArray());		
	    TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm()); 
	    tmf.init(trustedKeystore); 		
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

		ioReactor.listen(new InetSocketAddress(config.getAdapter().getHttpPort()));
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
