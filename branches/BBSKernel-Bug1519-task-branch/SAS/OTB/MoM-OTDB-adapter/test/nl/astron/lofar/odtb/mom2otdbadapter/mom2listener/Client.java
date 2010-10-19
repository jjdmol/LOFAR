package nl.astron.lofar.odtb.mom2otdbadapter.mom2listener;

import java.io.BufferedReader;
import java.io.FileReader;
import java.net.URL;
import java.security.KeyStore;

import nl.astron.util.http.client.AstronHttpClient;
import nl.astron.util.http.client.HttpClientConfig;
import nl.astron.util.http.client.handler.StringResponseHandler;
import nl.astron.util.http.exception.AstronHttpException;

import org.apache.http.conn.ssl.SSLSocketFactory;
import org.apache.http.entity.StringEntity;
import org.apache.http.params.BasicHttpParams;

public class Client {

	/**
	 * @param args
	 * @throws AstronHttpException 
	 */
	public static void main(String[] args) throws Exception {
		ClassLoader cl = Client.class.getClassLoader();
		URL url = cl.getResource("keystore-client.jks");
		KeyStore keystore = KeyStore.getInstance("jks");
		keystore.load(url.openStream(), "client".toCharArray());
       
		SSLSocketFactory socketFactory = new SSLSocketFactory(keystore, "client", keystore);

		HttpClientConfig config = new HttpClientConfig("https://localhost/adsf", socketFactory);
		AstronHttpClient client = new AstronHttpClient(config);
		BufferedReader in  = new BufferedReader(new FileReader("c:/observation.xml"));
		StringBuilder builder = new StringBuilder();
		String line = null;
		while ((line = in.readLine()) != null){
			builder.append(line)	;	
		}
		StringEntity entity = new StringEntity(builder.toString());
		entity.setContentType("text/xml");
////        reqEntity.setChunked(true);
//        // It may be more appropriate to use FileEntity class in this particular 
//        // instance but we are using a more generic InputStreamEntity to demonstrate
//        // the capability to stream out data from any arbitrary source
//        // 
//        // FileEntity entity = new FileEntity(file, "binary/octet-stream"); 
//        
//        httppost.setEntity(entity);

		BasicHttpParams params = new BasicHttpParams();
//		System.out.println("POST: " + builder.toString());
//		params.setParameter("observation", builder.toString());
		System.out.println(client.doPost("https://localhost/CRAFT/user/setUpIssueList.do", entity, new StringResponseHandler()));


	}

}
