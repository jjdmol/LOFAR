
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBaccessInterface;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBinterface;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;




class tRemoteConnection {
    private static jOTDBinterface             remoteOTDB1;
    private static jOTDBinterface             remoteOTDB2;
    private static jOTDBaccessInterface       remoteOTDBaccess;
    public static String RMIserverName      = "lofar17.astron.nl";
    public static String RMIserverPort      = "11500";
    private static String RMIaccessName     = jOTDBaccessInterface.SERVICENAME;
    private static String itsServiceName1    = "";
    private static String itsServiceName2    = "";

    public static void main (String[] args) {
        try {
            System.out.println("Starting... ");

	    // create a remote access object
            System.out.println("openRemoteAccess for "+RMIaccessName);

            // create a remote access object
            Registry registry = LocateRegistry.getRegistry(RMIserverName,Integer.parseInt(RMIserverPort));
            remoteOTDBaccess = (jOTDBaccessInterface) registry.lookup(RMIaccessName);

            System.out.println(remoteOTDBaccess);

            System.out.println("Connection to RemoteAccess succesful!");

        } catch (Exception e) {
	        System.out.println("Open Remote Connection via RMI and JNI failed: " + e);
            System.exit(0);
	    }


        try {
            itsServiceName1 = remoteOTDBaccess.login("coolen","whatever","coolen");

            // Now we have the name of the connection service, let's see if we can connect to it and get
            // access to it's functions
            System.out.println("openRemoteConnection for jOTDB_"+itsServiceName1);
            String aRegName = "jOTDB_"+itsServiceName1;

            // create a remote OTDB object
            Registry registry = LocateRegistry.getRegistry(RMIserverName,Integer.parseInt(RMIserverPort));
            remoteOTDB1 = (jOTDBinterface) registry.lookup(aRegName);

            System.out.println(remoteOTDB1);

            // do the test
            System.out.println("Trying to connect to the database");
            boolean c = remoteOTDB1.connect();
            assert c : "Connection failed";
            c = remoteOTDB1.isConnected();
            assert c : "Connnection flag failed";
            System.out.println("Connection succesful!");


    	    System.out.println("Connection succesful!");
	     
            System.out.println("getTreeList(0,0)");
            Vector treeList;
            treeList = remoteOTDB1.getTreeList((short)0, (short)0);
            if (treeList.size() == 0) {
                System.out.println("Error:" + remoteOTDB1.errorMsg());
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


        // set up 2nd connection

            itsServiceName2 = remoteOTDBaccess.login("overeem","whatever","overeem");

            // Now we have the name of the connection service, let's see if we can connect to it and get
            // access to it's functions
            System.out.println("openRemoteConnection for jOTDB_"+itsServiceName2);
            String aRegName2 = "jOTDB_"+itsServiceName2;

            // create a remote OTDB object
            Registry registry2 = LocateRegistry.getRegistry(RMIserverName,Integer.parseInt(RMIserverPort));
            remoteOTDB2 = (jOTDBinterface) registry2.lookup(aRegName2);

            System.out.println(remoteOTDB2);

            // do the test
            System.out.println("Trying to connect to the database");
            boolean c2 = remoteOTDB2.connect();
            assert c2 : "Connection 2 failed";
            c2 = remoteOTDB2.isConnected();
            assert c2 : "Connnection 2 flag failed";
            System.out.println("Connection 2 succesful!");


    	    System.out.println("Connection succesful!");

            System.out.println("getTreeList(0,0)");
            Vector treeList2;
            treeList2 = remoteOTDB2.getTreeList((short)0, (short)0);
            if (treeList2.size() == 0) {
                System.out.println("Error:" + remoteOTDB2.errorMsg());
                System.exit (0);
            } else {
                System.out.println("Collected tree list");
            }
            System.out.println("getTreeInfo(treeList2.elementAt(1))");
            jOTDBtree tInfo2 = (jOTDBtree)treeList2.elementAt(1);
            if (tInfo2.treeID()==0) {
                System.out.println("NOT SUCH TREE FOUND!");
            } else {
                System.out.println(tInfo2.classification);
                System.out.println(tInfo2.creator);
                System.out.println(tInfo2.creationDate);
                System.out.println(tInfo2.type);
                System.out.println(tInfo2.state);
                System.out.println(tInfo2.originalTree);
                System.out.println(tInfo2.campaign);
                System.out.println(tInfo2.starttime);
                System.out.println(tInfo2.stoptime);
                System.out.println(tInfo2.treeID());
            }

            // check if 1st database is still same
            System.out.println("First  databasename: "+remoteOTDB1.getDBName());
            System.out.println("Second databasename: "+remoteOTDB2.getDBName());




        } catch (Exception ex) {

                System.out.println("Error: "+ ex);
                ex.printStackTrace();
            }
    }

}
