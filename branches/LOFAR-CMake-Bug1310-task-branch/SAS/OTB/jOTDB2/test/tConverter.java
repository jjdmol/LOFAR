import nl.astron.lofar.sas.otb.jotdb2.jOTDBconnection;
import nl.astron.lofar.sas.otb.jotdb2.jClassifConv;
import nl.astron.lofar.sas.otb.jotdb2.jParamTypeConv;
import nl.astron.lofar.sas.otb.jotdb2.jTreeStateConv;
import nl.astron.lofar.sas.otb.jotdb2.jTreeTypeConv;
import nl.astron.lofar.sas.otb.jotdb2.jUnitConv;
import java.util.HashMap;
import java.util.Iterator;


class tConverter
{

    private HashMap<Short,String> aM;
    private Iterator it;

    private String dbName="otdbtest";

  static
     {
	System.loadLibrary("jotdb2");
     }
   
   
   public static void main(String[] args)
     {
	tConverter tconv = new tConverter();
	tconv.test();
     }
   
    public void test() {
        try {
            // do the test
            System.out.println("Starting... using Database: "+dbName);
	
            // create a jOTDBconnection
            jOTDBconnection conn = new jOTDBconnection("paulus","boskabouter",dbName);
	
            System.out.println("Trying to connect to the database");
            assert conn.connect() : "Connection failed";	
            assert conn.isConnected() : "Connnection flag failed";

            System.out.println("Connection succesful!");

            System.out.println("Constructing a Classif-type converter");
            jClassifConv cf = new jClassifConv();

            System.out.println("Converting 2: "+ cf.get((short)2));

            System.out.println("Converting test: "+ cf.get("test"));

            System.out.println("Reset the list:");
	
            System.out.println("getTypes Map");
            aM=cf.getTypes();
            it = aM.keySet().iterator();
            while (it.hasNext()) {
                Short key = (Short)it.next();
                System.out.println(key.toString() + "  <->  " + aM.get(key));
            }

            System.out.println("Constructing a ParamType converter");
            jParamTypeConv ptf = new jParamTypeConv();

            System.out.println("Converting 101: "+ ptf.get((short)101));

            System.out.println("Converting text: "+ ptf.get("text"));

            System.out.println("Reset the list:");
	
            ptf.top();
            System.out.println("getTypes Map");
            aM=ptf.getTypes();
            it = aM.keySet().iterator();
            while (it.hasNext()) {
                Short key = (Short)it.next();
                System.out.println(key.toString() + "  <->  " + aM.get(key));
            }

            System.out.println("Constructing a TreeState-type converter");
            jTreeStateConv ts = new jTreeStateConv();

            System.out.println("Converting 1100: "+ ts.get((short)1100));

            System.out.println("Converting active: "+ ts.get("active"));

            System.out.println("Reset the list:");
	
            ts.top();
            System.out.println("getTypes Map");
            aM=ts.getTypes();
            it = aM.keySet().iterator();
            while (it.hasNext()) {
                Short key = (Short)it.next();
                System.out.println(key.toString() + "  <->  " + aM.get(key));
            }


            System.out.println("Constructing a TreeType-type converter");
            jTreeTypeConv tt = new jTreeTypeConv();

            System.out.println("Converting 30: "+ tt.get((short)30));

            System.out.println("Converting hardware: "+ tt.get("hardware"));

            System.out.println("Reset the list:");
	
            tt.top();
            System.out.println("getTypes Map");
            aM=tt.getTypes();
            it = aM.keySet().iterator();
            while (it.hasNext()) {
                Short key = (Short)it.next();
                System.out.println(key.toString() + "  <->  " + aM.get(key));
            }


            System.out.println("Constructing a Unit-type converter");
            jUnitConv ut = new jUnitConv();

            System.out.println("Converting 7: "+ ut.get((short)7));

            System.out.println("Converting RAM: "+ ut.get("RAM"));

            System.out.println("Reset the list:");
	
            ut.top();
            System.out.println("getTypes Map");
            aM=ut.getTypes();
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



