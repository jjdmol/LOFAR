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
        boardStatus = new BoardStatus();
    }
    
    // -- NATIVE FUNCTIONS --
    private native void retrieveStatus(BoardStatus boardStatus);
    
    static
    {
        System.loadLibrary("jrsplib");
    }
        
    
    public BoardStatus getStatus()
    {
        retrieveStatus(boardStatus);
        return boardStatus;
    }
}
