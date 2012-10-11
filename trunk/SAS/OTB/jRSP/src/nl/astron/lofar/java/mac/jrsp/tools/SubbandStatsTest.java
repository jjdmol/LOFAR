package nl.astron.lofar.java.mac.jrsp.tools;

import nl.astron.lofar.java.mac.jrsp.Board;

public class SubbandStatsTest 
{
    public static void main(String[] args)
    {
        double subbandStats[] = null;
        
        Board b = new Board();
        try {
            b.connect("localhost");
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        if(b.isConnected())
        {

        }
        b.disconnect();
        
   /*     if(subbandStats != null)
        {            
            for(int i=0; i<subbandStats.length; i++)
            {
                System.out.println(i + ": " + subbandStats[i]);
            }
        }*/
    }
}