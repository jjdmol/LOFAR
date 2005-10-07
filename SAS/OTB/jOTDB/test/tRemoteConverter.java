import java.rmi.registry.*;
import jOTDB.jOTDBinterface;
import jOTDB.jConverterInterface;
import java.util.Vector;

class tRemoteConverter
{
    private static jOTDBinterface conn;
    private static jConverterInterface tc;

    public static void main (String[] args)
    {
	try
	  {
	     if (args.length != 1)
	       {
		  System.out.println ("Usage: java tRemoteTreeMaintenance <rmi-registry_hostname>");
		  System.exit(0);
	       }

	     System.out.println("Starting... ");

	     // create a remote object
	     Registry remoteRegistry = LocateRegistry.getRegistry (args[0]);
	     conn = (jOTDBinterface) remoteRegistry.lookup (jOTDBinterface.SERVICENAME);

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
	     
	     // Test converter
	     tc = (jConverterInterface) remoteRegistry.lookup (jConverterInterface.SERVICENAME);	     	     
	     System.out.println (tc.getClassif ((short)1));
	     System.out.println (tc.getClassif ("development"));
	     System.out.println (tc.getClassif ((short)2));
	     System.out.println (tc.getClassif ("test"));
	     System.out.println (tc.getClassif ((short)3));
	     System.out.println (tc.getClassif ("operational"));

	     System.out.println (tc.getParamType ((short)0));
	     System.out.println (tc.getParamType ("node"));
	     System.out.println (tc.getParamType ((short)101));
	     System.out.println (tc.getParamType ((short)102));
	     System.out.println (tc.getParamType ((short)6));
	     System.out.println (tc.getParamType ((short)20));

	     System.out.println (tc.getUnit ((short)1));
	     System.out.println (tc.getUnit ((short)2));
	     System.out.println (tc.getUnit ((short)3));
	     System.out.println (tc.getUnit ((short)4));
	     System.out.println (tc.getUnit ((short)5));
	     System.out.println (tc.getUnit ((short)6));

	  }
	catch (Exception e)
	    {
		System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	    }
	
     }
}
