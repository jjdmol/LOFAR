package nl.astron.lofar.java.mac.jrsp.tools;

import nl.astron.lofar.java.mac.jrsp.Board;
import nl.astron.lofar.java.mac.jrsp.RCUMask;

public class TestClass 
{
    public static void main(String[] args) throws Exception
    {
        System.out.println(">>--> TEST CLASS <--<<");
        
        Board b = new Board();
        b.connect("rs002");
        
        if (b.isConnected()) 
        {
            double[] dd = b.getSubbandStats(new RCUMask(1));//b.getBeamletStats(new RSPMask(1));
            System.out.println("DD LENGTH: " + dd.length);
            
            for (int i = 0; i < dd.length; i++) {
                System.out.println(dd[i]);
            }
        }
                
        b.disconnect();
    }
    
    public static void doSomething() {
        
        Board b = new Board();

        System.out.println(b.isConnected());
        
        if (b.isConnected()) 
        {


            
            b.disconnect();
        }        
    }
    
    
}
