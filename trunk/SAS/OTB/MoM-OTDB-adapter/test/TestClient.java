

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

public class TestClient {
	public void listenSocket(){
//		Create socket connection
		   try{
		     Socket socket = new Socket("localhost", 4444);
			File file = new File("r:\\observation.xml");
			FileInputStream input = new FileInputStream(file);
			int byteInt = -1;
			while ((byteInt = input.read()) != -1){
				socket.getOutputStream().write(byteInt);

			}
			 socket.getOutputStream().flush();
			 socket.getOutputStream().close();
		     socket.close();
		   } catch (UnknownHostException e) {
		     System.out.println("Unknown host: localhost");
		     System.exit(1);
		   } catch  (IOException e) {
		     System.out.println("No I/O");
		     System.exit(1);
		   }
		}
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		TestClient testClient = new TestClient();
		testClient.listenSocket();
		// TODO Auto-generated method stub

	}

}
