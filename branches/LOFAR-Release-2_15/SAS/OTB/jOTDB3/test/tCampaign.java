import java.util.Vector;
import nl.astron.lofar.sas.otb.jotdb3.jCampaign;
import nl.astron.lofar.sas.otb.jotdb3.jCampaignInfo;
import nl.astron.lofar.sas.otb.jotdb3.jInitCPPLogger;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBconnection;


class tCampaign
{

    private String ext="11501_coolen";

    static
     {
	System.loadLibrary ("jotdb3");
     }
   
   
   public static void main (String[] args)
     {
	String logConfig="tCampaign.log_prop";
        try {
            jInitCPPLogger aCPPLogger= new jInitCPPLogger(logConfig);
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }
	tCampaign tCampaign = new tCampaign ();
	tCampaign.test ();
     }

    public void showCampaignList (Vector<jCampaignInfo>  camps,jOTDBconnection conn)
    {

    boolean firstLine=true;
    jCampaignInfo aCampaignInfo;
	for (int i = 0; i < camps.size (); ++i)
	    {
    		aCampaignInfo = camps.elementAt(i);
            showCampaignInfo(aCampaignInfo,firstLine);
            firstLine=false;
	    }	
	System.out.println (camps.size () + " records\n");
    }
    
    public void showCampaignInfo(jCampaignInfo aCampaignInfo, boolean firstLine) {
        if (firstLine) {
            System.out.println ("ID  lname      |title     |PI             |CO_I           |contact");
            System.out.println ("----+----------+----------+---------------+---------------+------------------");
        }

        System.out.printf ("%4d|%-10.10s|%-10.10s|%-15.15s|%-15.15s\n",
                aCampaignInfo.ID(),
                aCampaignInfo.itsName,
                aCampaignInfo.itsTitle,
                aCampaignInfo.itsPI,
                aCampaignInfo.itsCO_I,
                aCampaignInfo.itsContact);

    }
    
    
  
    public void test () {
        try {
            // do the test
            System.out.println ("Starting... ");
	
            // create a jOTDBconnection
            jOTDBconnection conn = new jOTDBconnection("paulus","boskabouter","coolen","10.87.2.185","11501_coolen");
	
            System.out.println ("Trying to connect to the database");
            assert conn.connect () : "Connection failed";	
            assert conn.isConnected () : "Connnection flag failed";

            System.out.println ("Connection succesful!");

            System.out.println ("Trying to construct a Campaign object");
            jCampaign camp = new jCampaign (ext);
            assert camp==null : "Creation of Campaign Failed!";
        

            System.out.println ("Searching for campaigns");
            Vector<jCampaignInfo> campList = camp.getCampaignList();
            showCampaignList (campList, conn);
            assert campList.size()!=0 : "No campaign list found!";

            System.out.println("Adding my campaign");
            jCampaignInfo CI = new jCampaignInfo("my campaign", "campaign of me", "me", "no-one", "also me");
            System.out.println("new recordID = "+ camp.saveCampaign(CI));
            campList = camp.getCampaignList();
            showCampaignList (campList, conn);
            assert campList.size()!=0 : "No campaign list found!";

            System.out.println("Adding your campaign");
            jCampaignInfo YCI = new jCampaignInfo("your campaign", "campaign of you", "you", "no-one", "also you");
            System.out.println("new recordID = "+ camp.saveCampaign(YCI));
            campList = camp.getCampaignList();
            showCampaignList (campList, conn);
            assert campList.size()!=0 : "No campaign list found!";

            System.out.println("Changing my campaign");
            jCampaignInfo myCamp = camp.getCampaign("my campaign");
            myCamp.itsTitle ="my own campaign";
            myCamp.itsContact = "112";
            System.out.println("recordID = "+ camp.saveCampaign(myCamp));
            campList = camp.getCampaignList();
            showCampaignList (campList, conn);
            assert campList.size()!=0 : "No campaign list found!";


            System.out.println ("Terminated succesfully: ");
        } catch (Exception ex) {
            System.out.println("Error: "+ ex);
            ex.printStackTrace();
        }
     }
}




