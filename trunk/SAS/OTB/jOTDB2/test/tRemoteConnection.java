import java.rmi.Naming;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
//import jOTDB.jOTDBconnection;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBinterface;


class tRemoteConnection {
    private static jOTDBinterface remoteOTDB;
    public static String RMIServerName      = "lofar17.astron.nl";
    public static String RMIServerPort      = "1091";
    public static String RMIRegistryName    = jOTDBinterface.SERVICENAME;
    
    public static void main (String[] args) {
        try {
//            if (args.length != 1) {
//                System.out.println ("Usage: java tRemoteConnection <rmi-registry_hostname>");
//		System.exit(0);
//	    }

            System.out.println("Starting... ");

	    // create a remote object
//	    Registry remoteRegistry = LocateRegistry.getRegistry (args[0]);
//	    remoteOTDB = (jOTDBinterface) remoteRegistry.lookup (jOTDBinterface.SERVICENAME);
            String RMIRegHostName="rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIRegistryName;
            remoteOTDB = (jOTDBinterface) Naming.lookup (RMIRegHostName);     	     
	    System.out.println (remoteOTDB);
					    
	    // do the test	
	    System.out.println("Trying to connect to the database");
	    assert remoteOTDB.connect() : "Connection failed";	
	    assert remoteOTDB.isConnected() : "Connnection flag failed";
	     
	    System.out.println("Connection succesful!");
	     
	    System.out.println("getTreeList(0,0)");
	    Vector treeList;
	    treeList = remoteOTDB.getTreeList((short)0, (short)0);
	    if (treeList.size() == 0) {
                System.out.println("Error:" + remoteOTDB.errorMsg());
		System.exit (0);
	    } else {
		System.out.println("Collected tree list");
            }
            System.out.println("getTreeInfo(treeList.elementAt(1))");
            jOTDBtree tInfo = (jOTDBtree)treeList.elementAt(1);
            if (tInfo.treeID()==0) {
                System.out.println("NOT SUCH TREE FOUND!");
            } else {
                System.out.println(tInfo.classification);
                System.out.println(tInfo.creator);
                System.out.println(tInfo.creationDate);	
                System.out.println(tInfo.type);
                System.out.println(tInfo.state);
                System.out.println(tInfo.originalTree);
                System.out.println(tInfo.campaign);	
                System.out.println(tInfo.starttime);
                System.out.println(tInfo.stoptime);
                System.out.println(tInfo.treeID());	   
            }
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }
     }
}
