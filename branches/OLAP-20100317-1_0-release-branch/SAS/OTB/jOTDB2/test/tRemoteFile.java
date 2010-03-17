
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.rmi.Naming;
import nl.astron.lofar.lofarutils.remoteFileInterface;


public class tRemoteFile {
 
    
   public static String RMIRegistryName    = remoteFileInterface.SERVICENAME;
   
   public static void main(String[] argv) {
        if(argv.length != 3) {
            System.out.println("Usage: java tRemoteFile fileName machineName machinePort");
            System.out.println("Where file is a file on the client.");
            System.exit(0);
        }
        String aRF="rmi://"+argv[1]+":"+argv[2]+"/"+RMIRegistryName;
        
        try {
            // do the test
            System.out.println("Starting...");
            remoteFileInterface fi = (remoteFileInterface) Naming.lookup(aRF);
            
            System.out.println("Starting upload");
            File aFile = new File(argv[0]);
            byte uldata[] = new byte[(int)aFile.length()]; 
            System.out.println("File opened: "+aFile.getName()+" bytesize: "+ uldata.length);
            BufferedInputStream input = new BufferedInputStream(new FileInputStream(aFile));
            input.read(uldata,0,uldata.length);
            input.close();
            if (fi.uploadFile(uldata,argv[0]+".uploaded")) {
                System.out.println("upload finished");
            } else {
                System.out.println("upload failed");
            }
            
            
            System.out.println("Starting download");
            byte[] dldata = fi.downloadFile(argv[0]+".uploaded");
            File file = new File(argv[0]+".downloaded");
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



