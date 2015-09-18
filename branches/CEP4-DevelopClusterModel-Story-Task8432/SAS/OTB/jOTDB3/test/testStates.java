
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBinterface;
import nl.astron.lofar.sas.otb.jotdb3.jTreeState;


/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author coolen
 */
public class testStates {
   public static void main(String[] args)
     {
	testStates tconn = new testStates();
	tconn.test();
     }

    public void test() {
        try {
            // do the test
            System.out.println("Starting... ");
            String sd="2009-05-01 11:42:52.000";
            String ed="2009-10-01 11:42:52.000";
            Registry remoteRegistry = LocateRegistry.getRegistry("lofar17", 10500);
            jOTDBinterface jotdbInterface = (jOTDBinterface) remoteRegistry.lookup(jOTDBinterface.SERVICENAME);
            jotdbInterface.connect();
            Vector<jTreeState> stateList = jotdbInterface.getStateList(0,false, sd, ed);
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }
     }
}
