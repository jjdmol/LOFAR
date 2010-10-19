/*
 * navigatorRMIImpl.java
 *
 * Created on 7 juni 2005, 15:35
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */


package otbtest;

import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.ArrayList;

/**
 *
 * @author Arthur Coolen
 *
 * What to do to get RMI going.
 *
 * look at the server ip and port in navigatorRMIImpl.java and otbgui.java and change if you take another server ip
 *
 * make a subdir otb and place navigatorRMI.jave navigatorRMIServer.java and navigatorRMIImpl.java in it
 * go one dir below  the otb dir and say javac otb/navigatorRMI.java
 *                                       javac otb/navigatorRMIImpl.java
 *                                       javac otb/navigatorRMIServer.java
 *
 * from the same place make the stub by typing rmic otb.navigatorRMIImpl
 * this will create a navigatorRMIImpl_Stub.class
 *
 * ftp navigatorRMI.class  navigatorRMIImpl.class  navigatorRMIServer.class to the server to an otb subdir
 * login to the server and go to the directory and type rmiregistry &  this will start the rmi registry
 * then start the server with java otb/navigatorRMIServer 10099 &
 * 10099 represents the chosen port.
 *
 * ATTENTION:
 *
 * Make VERY sure that when you run the server side classes, that you use the same or a newer java version!
 * Make also VERY sure that the CLASSPATH is set to the place where your classes reside, ESPECIALLY BEFORE you
 * start rmiserver !!!!
 *
 * */

public class navigatorRMIImpl extends UnicastRemoteObject implements navigatorRMI {

    // JNI test with stringarraylist
    public native String [] getDBList();

    static {
       System.loadLibrary("OTDB");
   }


    private ArrayList<String []> people = new ArrayList<String []>();

    
    /**
     * Construct a remote object
     * @param msg the message of the remote object, such as "Hello, world!".
     * @exception RemoteException if the object handle cannot be constructed.
     */
    public navigatorRMIImpl() throws RemoteException {
   }

    /**
     * Implementation of the remotely invocable method.
     * @return the found data
     * @exception RemoteException if the remote invocation fails.
     */
    public ArrayList<String []> getList() throws RemoteException {
        return people;
  }

  public void setList(String [] aStrList) {
     for (int j=0; j < aStrList.length;j++) {                  
        // find the last aaa="bbb" combination
        String [] aS1 = aStrList[j].split("[=]");
        String [] aS2 = aS1[0].split("[.]");
     
        String [] saveString= new String[2];
        for (int i=0; i< aS2.length-1;i++) {
           if (i>0) {
              saveString[0]+="."+aS2[i];
           } else {
              saveString[0]=aS2[i];
           }
        }
        saveString[1]=aS2[aS2.length-1]+"="+aS1[1];
        saveString[0]=saveString[0].replace('_',' ');
        saveString[1]=saveString[1].replace('_',' ');
        people.add(saveString);
      }
   }
}
    
    
