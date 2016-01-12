
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import nl.astron.lofar.lofarutils.remoteFileInterface;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBaccessInterface;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBinterface;


public class tRemoteFile {
    private static jOTDBinterface             remoteOTDB;
    private static jOTDBaccessInterface       remoteOTDBaccess;


    private static remoteFileInterface          fi;
    public static String RMIServerName       = "lofar17.astron.nl";
    public static String RMIServerPort       = "11500";
    public static String RMIRegistryName     =  jOTDBinterface.SERVICENAME;
    public static String RMIremoteFileName   =  remoteFileInterface.SERVICENAME;
    private static String RMIaccessName      =  jOTDBaccessInterface.SERVICENAME;
    private static String itsServiceName    = "";
    

    static String fileName       = "tRemoteFile.java";  // we need a file to be transported

    public static void main(String[] argv) {

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


            // do the test
            System.out.println("Starting...");
            fi = (remoteFileInterface) registry.lookup("remoteFile_"+itsServiceName);
            
            System.out.println("Starting upload");
            File aFile = new File(fileName);
            byte uldata[] = new byte[(int)aFile.length()]; 
            System.out.println("File opened: "+aFile.getName()+" bytesize: "+ uldata.length);
            BufferedInputStream input = new BufferedInputStream(new FileInputStream(aFile));
            input.read(uldata,0,uldata.length);
            input.close();
            if (fi.uploadFile(uldata,fileName+".uploaded")) {
                System.out.println("upload finished");
            } else {
                System.out.println("upload failed");
            }
            
            
            System.out.println("Starting download");
            byte[] dldata = fi.downloadFile(fileName+".uploaded");
            File file = new File(fileName+".downloaded");
            System.out.println("opening File: "+file.getName());
            BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(file.getName()));
            System.out.println("Received buffer length: "+dldata.length);
            output.write(dldata,0,dldata.length);
            output.flush();
            output.close();
            System.out.println("Download finished");
        } catch(Exception e) {
            System.err.println("FileServer exception: "+ e.getMessage());
            e.printStackTrace();
        }
    }
}



