package nl.astron.lofar.mac.apl.gui.jrsp.tools;
import nl.astron.lofar.mac.apl.gui.jrsp.Board;
import nl.astron.lofar.mac.apl.gui.jrsp.BoardStatus;

public class TestClass 
{
    public static void main(String[] args)
    {
        Board b = new Board();
        b.connect("rs002");
        
        if (b.isConnected()) 
        {
            double[] d = b.getSubbandStats(3);
            System.out.println(d.length);
        }
                
        b.disconnect();        
    }
    
}
