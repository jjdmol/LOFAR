import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb3.jConverterInterface;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBaccessInterface;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBinterface;

class tRemoteConverter
{
    private static jOTDBinterface             remoteOTDB;
    private static jOTDBaccessInterface       remoteOTDBaccess;


    private static jConverterInterface tc;
    private static HashMap<Short,String> aM;
    private static Iterator it;
    public static String RMIServerName      = "lofar17.astron.nl";
    public static String RMIServerPort      = "11500";
    public static String RMIRegistryName    =  jOTDBinterface.SERVICENAME;
    public static String RMIConverterName   =  jConverterInterface.SERVICENAME;
    private static String RMIaccessName     =  jOTDBaccessInterface.SERVICENAME;
    private static String itsServiceName    = "";
    
    public static void main (String[] args)
    {
    	try {

            System.out.println("Starting... ");

            // create a remote access object
            System.out.println("openRemoteAccess for "+RMIaccessName);

            // create a remote access object
            Registry registry = LocateRegistry.getRegistry(RMIServerName,Integer.parseInt(RMIServerPort));
            remoteOTDBaccess = (jOTDBaccessInterface) registry.lookup(RMIaccessName);
            System.out.println(remoteOTDBaccess);

            System.out.println("Connection to RemoteAccess succesful!");

        } catch (Exception e) {
            System.out.println("Open Remote Connection via RMI and JNI failed: " + e);
            System.exit(0);
        }


        try {
            itsServiceName = remoteOTDBaccess.login("coolen","whatever","coolen");

            // Now we have the name of the connection service, let's see if we can connect to it and get
            // access to it's functions
            System.out.println("openRemoteConnection for jOTDB_"+itsServiceName);
            String aRegName = "jOTDB_"+itsServiceName;

            // create a remote OTDB object
            Registry registry = LocateRegistry.getRegistry(RMIServerName,Integer.parseInt(RMIServerPort));

            remoteOTDB = (jOTDBinterface) registry.lookup(aRegName);
            System.out.println(remoteOTDB);

            // do the test
            System.out.println("Trying to connect to the database");
            boolean c = remoteOTDB.connect();
            assert c : "Connection failed";
            c = remoteOTDB.isConnected();
            assert c : "Connnection flag failed";
            System.out.println("Connection succesful!");


     	    System.out.println("Connection succesful!");
	     	     	     
            System.out.println ("Searching for a Template tree");
            Vector treeList;
            treeList = remoteOTDB.getTreeList ((short)20, (short)0);  // 20 = template, 0 = all
            if (treeList.size () == 0) {
                System.out.println ("No template tree found!");
                System.exit (0);
            }

            tc = (jConverterInterface) registry.lookup("jConverter_"+itsServiceName);
	     
            // Test converter

            // Classif
            System.out.println("****** Classif testen ******");
            System.out.println("Converting 2: "+ tc.getClassif((short)2));
            System.out.println("Converting test: "+ tc.getClassif("test"));
            aM=tc.getClassif();
            it = aM.keySet().iterator();
            while (it.hasNext()) {
                Short key = (Short)it.next();
                System.out.println(key.toString() + "  <->  " + aM.get(key));
            }

            // ParamType
            System.out.println("****** ParamType testen ******");
            System.out.println("Converting 101: "+ tc.getParamType((short)101));
            System.out.println("Converting text: "+ tc.getParamType("text"));
            aM=tc.getParamType();

            it = aM.keySet().iterator();
            while (it.hasNext()) {
                Short key = (Short)it.next();
                System.out.println(key.toString() + "  <->  " + aM.get(key));
            }
             
            // TreeState
            System.out.println("****** TreeState testen ******");
            System.out.println("Converting 1100: "+ tc.getTreeState((short)1100));
            System.out.println("Converting active: "+ tc.getTreeState("active"));
            aM=tc.getTreeState();
            it = aM.keySet().iterator();
            while (it.hasNext()) {
                Short key = (Short)it.next();
                System.out.println(key.toString() + "  <->  " + aM.get(key));
            }
             
            // TreeType
            System.out.println("****** TreeType testen ******");
            System.out.println("Converting 30: "+ tc.getTreeType((short)30));
            System.out.println("Converting hardware: "+ tc.getTreeType("hardware"));
            aM=tc.getTreeType();
            it = aM.keySet().iterator();
            while (it.hasNext()) {
                Short key = (Short)it.next();
                System.out.println(key.toString() + "  <->  " + aM.get(key));
            }
             
            // Unit
            System.out.println("****** Unit testen ******");
            System.out.println("Converting 7: "+ tc.getUnit((short)7));
            System.out.println("Converting RAM: "+ tc.getUnit("RAM"));
            aM=tc.getUnit();
            it = aM.keySet().iterator();
            while (it.hasNext()) {
                Short key = (Short)it.next();
                System.out.println(key.toString() + "  <->  " + aM.get(key));
            }
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }
    }
}
