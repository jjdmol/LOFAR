package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * The Board class is used to connect to the C++ side of the whole jRSP project.
 * 
 * @TODO: The method setHostname should do more things than just change hostName.
 *
 * @author  balken
 */
public class Board
{
    /** Hostname */
    private String hostname;
    
    /** Array of BoardStatus */
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
    
    // -- START NATIVE FUNCTIONS --
    private native BoardStatus[] retrieveStatus(String hostname);
    
    static
    {
        System.loadLibrary("jrsp");
    }
    // -- END NATIVE FUNCTIONS --
    
    /**
     * @return  hostname
     */
    public String getHostname()
    {
        return hostname;
    }
    
    /**
     * Sets hostname to a the given value.
     * @param   hostname    The new hostname.
     */
    public void setHostname(String hostname)
    {
        this.hostname = hostname;
    }
    
    /**
     * Returns the status of the Boards.
     * @return  BoardStatus[]   A array of BoardStatus.
     */
    public BoardStatus[] getStatus()
    {
        boardStatus = retrieveStatus(hostname);
        return boardStatus;
    }
}
