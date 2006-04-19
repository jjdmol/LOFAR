package nl.astron.lofar.mac.apl.gui.jrsp.tools;

import nl.astron.lofar.mac.apl.gui.jrsp.Board;

public class SubbandStatsTest 
{
    public static void main(String[] args)
    {
        double subbandStats[] = null;
        
        Board b = new Board();
        b.connect("localhost");
        if(b.isConnected())
        {
            subbandStats = b.getSubbandStats(0);
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