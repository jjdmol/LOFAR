import java.rmi.Naming;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jConverterInterface;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBinterface;
import nl.astron.lofar.sas.otb.jotdb2.jTreeMaintenanceInterface;


class tRemoteConverter
{
    private static jOTDBinterface conn;
    private static jConverterInterface tc;
    private static HashMap<Short,String> aM;
    private static Iterator it;
    public static String RMIServerName      = "lofar17.astron.nl";
    public static String RMIServerPort      = "1091";
    public static String RMIRegistryName    =  jOTDBinterface.SERVICENAME;
    public static String RMIConverterName   =  jConverterInterface.SERVICENAME;
    
    public static void main (String[] args)
    {
	try {

	     System.out.println("Starting... ");

	     // create a remote object
             conn = (jOTDBinterface) Naming.lookup ("rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIRegistryName);     	    
	     // do the test
	     System.out.println ("Starting... ");
	     
	     System.out.println ("Trying to connect to the database");
	     assert conn.connect () : "Connection failed";	
	     assert conn.isConnected () : "Connnection flag failed";
	     
	     System.out.println ("Connection succesful!");	     
	     	     	     
	     System.out.println ("Searching for a Template tree");
	     Vector treeList;
	     treeList = conn.getTreeList ((short)20, (short)0);  // 20 = template, 0 = all
	     if (treeList.size () == 0) {
		 System.out.println ("No template tree found!");
		 System.exit (0);	   
	     }
	     
             tc = (jConverterInterface) Naming.lookup ("rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIConverterName);     	    
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
