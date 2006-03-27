package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * @TODO    Commentaar toevoegen.
 * @author  balken
 */
public class Board
{
    private String hostname;
    private BoardStatus[] boardStatus;
    
    /**
     * Makes a new instance of the Board class.
     * @param   hostname    The hostname of the RSPBoard to connect to.
     */   
    public Board(String hostname)
    {
        this.hostname = hostname;
        boardStatus = null;
    }
    
    // -- NATIVE FUNCTIONS --
    private native BoardStatus[] retrieveStatus(String hostname);
    
    static
    {
        System.loadLibrary("jrsp");
    }
        
    
    public BoardStatus[] getStatus()
    {
        boardStatus = retrieveStatus(hostname);
        return boardStatus;
    }
}
