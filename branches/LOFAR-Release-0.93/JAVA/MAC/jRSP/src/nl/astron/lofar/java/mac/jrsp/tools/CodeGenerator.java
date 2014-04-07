package nl.astron.lofar.java.mac.jrsp.tools;

/**
 * This class is just a quick and dirty way of making the jni code.
 */
import java.util.ArrayList;

public class CodeGenerator
{
    public static void main(String args[])
    {
        ArrayList<String> al = new ArrayList<String>();
        
        al.add("voltage1V2");
        al.add("voltage2V5");
        al.add("voltage3V3");
        al.add("pcbTemp");
        al.add("bpTemp");
        al.add("ap0Temp");
        al.add("ap1Temp");
        al.add("ap2Temp");
        al.add("ap3Temp");
        al.add("bpClock");
        al.add("nofFrames");
        al.add("nofErrors");
        al.add("lastError");
        al.add("seqNr");        
        al.add("error");
        al.add("ifUnderTest");
        al.add("mode");
        al.add("riErrors");
        al.add("rcuxErrors");
        al.add("lcuErrors");
        al.add("cepErrors");
        al.add("serdesErrors");
        al.add("ap0RiErrors");
        al.add("ap1RiErrors");
        al.add("ap2RiErrors");
        al.add("ap3RiErrors");
        al.add("blp0Sync");
        al.add("blp1Sync");
        al.add("blp2Sync");
        al.add("blp3Sync");
        al.add("blp0Rcu");
        al.add("blp1Rcu");
        al.add("blp2Rcu");
        al.add("blp3Rcu");
        al.add("cpStatus");
        al.add("blp0AdcOffset");
        al.add("blp1AdcOffset");
        al.add("blp2AdcOffset");
        al.add("blp3AdcOffset");
        
        System.out.println("-- GETTING THE FIELD ID --");
        for(int i = 0; i<al.size(); i++)
        {
            char[] fid = al.get(i).toCharArray();
            fid[0] = java.lang.Character.toUpperCase(fid[0]);
            String sFid = "fid"+new String(fid);
                        
            System.out.println("\tjfieldID " + sFid + " = env->GetFieldID(clsStatus, \"" + al.get(i) + "\", \"I\");");
            al.set(i, sFid);
        }
        
        System.out.println("\n\n");
        for(int i = 0; i<al.size(); i++)
        {
            System.out.println("\tif("+al.get(i)+" != 0)\n\t{\n\t\tenv->SetIntField(status, "+al.get(i)+", testData);\n\t}");
        }
    }
}
