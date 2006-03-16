package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * @TODO    Commentaar toevoegen.
 * @author  balken
 */
public class Board
{
    private BoardStatus boardStatus;
    
    public Board()
    {
        boardStatus = null;
    }
    
    // -- NATIVE FUNCTIONS --
    private native BoardStatus retrieveStatus();
    
    static
    {
        System.loadLibrary("jrsplib");
    }
        
    
    public BoardStatus getStatus()
    {
        boardStatus = retrieveStatus();
        return boardStatus;
    }
}
