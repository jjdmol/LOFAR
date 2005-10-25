/*
 * navigatorRMIServer.java
 *
 * Created on June 10, 2005, 2:11 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package otbtest;

import java.rmi.Naming;

/**
 *
 * @author Arthur Coolen
 *
 * How to build the JNI part (RMI is explained in navigatorRMIImpl.java)
 * 
 * On the server you need to following programs in subdir otb:
 * navigatorRMIImpl.java
 * navigatorRMIServer.java
 * OTDBImpl.cc
 * 
 * from withing otb do the following:
 * cd ..
 * javac otb/navigatorRMIImpl.java 
 * javac otb/navigatorRMIServer.java
 * javah -jni otb.navigatorRMIImpl this will build: otb_navigatorRMIImpl.h
 * mv otb_navigatorRMIImpl.h otb
 * cd otb
 * g++ -shared -I/opt/jdk1.5.0_03/include -I/opt/jdk1.5.0_03/include/linux \
 *      OTDBImpl.cc -o libOTDB.so
 *
 * Then Start the server with :
 * cd ..
 * java otb.navigatorRMIServer
 *
 * Requirement is that the rmiregistry 10099 &  as explained 
 * in navigatorRMIImpl.java should be running also
 *
 */

public class navigatorRMIServer {
    
  /**
   * Server program for the navigator example program
   * @param argv The command line arguments which are ignored.
   */

    public static void main (String[] argv) {
      try {
	navigatorRMIImpl aNRI = new navigatorRMIImpl(); 

        Naming.rebind("rmi://10.0.0.154:10099/navigatorRMItje", aNRI);
//        Naming.rebind("rmi://dop32.astron.nl:10099/navigatorRMItje", aNRI);
        System.out.println ("navigator Server is ready.");

        //test the JNI binding
	aNRI.setList(aNRI.getDBList());
        System.out.println("People should be set now");

    } catch (Exception e) {
      System.out.println ("navigator Server failed: " + e);
    }

  }
}
