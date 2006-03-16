package nl.astron.lofar.mac.apl.gui.jrsp.tools;

/**
 * This class is just a quick and dirty way of making the jni code.
 */
import java.util.ArrayList;

public class CodeGenerator
{
    public static void main(String args[])
    {
        ArrayList<String> al = new ArrayList<String>();
        
        al.add("setLastError");
        al.add("setSeqnr");
        al.add("setError");
        al.add("setInterface");
        al.add("setMode");
        al.add("setRiErrors");
        al.add("setRcuxErrors");
        al.add("setLcuErrors");
        al.add("setCepErrors");
        al.add("setSerdesErrors");
        al.add("setAp0RiErrors");
        al.add("setAp1RiErrors");
        al.add("setAp2RiErrors");
        al.add("setAp3RiErrors");
        al.add("setBlp0Sync");
        al.add("setBlp1Sync");
        al.add("setBlp2Sync");
        al.add("setBlp3Sync");
        al.add("setBlp0Rcu");
        al.add("setBlp1Rcu");
        al.add("setBlp2Rcu");
        al.add("setBlp3Rcu");
        al.add("setCpStatus");
        al.add("setBlp0AdcOffset");
        al.add("setBlp1AdcOffset");
        al.add("setBlp2AdcOffset");
        al.add("setBlp3AdcOffset");
        
        System.out.println("-- GETTING THE METHOD ID --");
        for(int i = 0; i<al.size(); i++)
        {
            char[] mid = al.get(i).toCharArray();
            mid[0] = java.lang.Character.toUpperCase(mid[0]);
            String sMid = "mid"+new String(mid);
                        
            System.out.println("\tjmethodID " + sMid + " = env->GetMethodID(clsStatus, \"" + al.get(i) + "\", \"(I)V\");");
            al.set(i, sMid);
        }
        
        System.out.println("\n\n-- TESTING METHOD --");
        for(int i = 0; i<al.size(); i++)
        {
            System.out.println("\tif("+al.get(i)+" != 0)\n\t{\n\t\tenv->CallVoidMethod(status, "+al.get(i)+", testData);\n\t}");
        }
    }
}
