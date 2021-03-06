

import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBconnection;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;


class tConnection
{
 
    static
     {
	System.loadLibrary("jotdb3");
     }
   
   
   public static void main(String[] args)
     {
	tConnection tconn = new tConnection();
	tconn.test();
     }
   
    public void test() {
        try {
            // do the test
            System.out.println("Starting... ");
	
            // create a jOTDBconnection
            jOTDBconnection conn = new jOTDBconnection("paulus","boskabouter","coolen","10.87.2.185","11501_coolen");
	
            System.out.println("Trying to connect to the database");
            boolean connect=conn.connect();
            boolean isConnected=conn.isConnected();
            assert  connect : "Connection failed";
            assert isConnected : "Connnection flag failed";

            System.out.println("Connection succesful!");

            System.out.println("getTreeList(0,0)");
            Vector treeList;
            treeList = conn.getTreeList((short)0, (short)0);
            if (treeList.size() == 0) {
                System.out.println("Error:" + conn.errorMsg());
            } else {
                System.out.println("Collected tree list");
                //showTreeList(treeList);
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



