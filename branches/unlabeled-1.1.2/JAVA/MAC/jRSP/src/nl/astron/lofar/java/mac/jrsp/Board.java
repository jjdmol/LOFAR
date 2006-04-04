package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * The Board class is used to connect to the C++ side of the whole jRSP project.
 * 
 * @TODO: The method setHostname should do more things than just change hostName.
 * @author  balken
 */
public class Board
{
    /** Hostname */
    private String hostname;
    
    /** Array of BoardStatus */
    private BoardStatus[] boardStatus;
    
    /** Pointer to the RSPport class in C++ */
    private int ptrRSPport;
    
    /**
     * Makes a new instance of the Board class.
     * @param   hostname    The hostname of the RSPBoard to connect to.
     */   
    public Board(String hostname)
    {
        this.hostname = hostname;
        ptrRSPport = 0;
        // @TODO activeren: ptrRSPport = init(hostname);
        boardStatus = null;
    }
    
    // -- START NATIVE FUNCTIONS --
    private native int init(String hostname); // initializes RSPport on the C++ side, for further use.
    private native void delete(int ptrRSPport); // deletes the RSPport instance on the c++ side.
    private native BoardStatus[] retrieveStatus(int rcuMask, int ptrRSPport); // retrieves the boards.
    private native int retrieveNofBoards(int ptrRSPport); // retrieves the number of boards.
    private native boolean setWaveformSettings(int rcuMask, int mode, int frequency, int amplitude, int ptrRSPport); // Sets the waveform settings.
    
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
        // @TODO delete(ptrRSPport);
        // @TODO ptrRSPport = init(hostname);
        this.hostname = hostname;
    }
    
    /**
     * Returns the status of the Boards.
     * @return  BoardStatus[]   A array of BoardStatus.
     */
    public BoardStatus[] getStatus()
    {
        boardStatus = retrieveStatus(0, ptrRSPport);
        return boardStatus;
    }
    
    /**
     * Returns the number of boards that can be reached by the RSP driver.
     * @return  nofBoards   The number of boards.
     */
    public int getNofBoards()
    {
        //@TODO activeren: return retrieveNofBoards(ptrRSPport);
        return 2;
    }
    
    /**
     * Sets the waveform settings.
     * @param   rcuMask
     * @param   mode
     * @param   frequency
     * @param   amplitude
     */
    public boolean setWaveformSettings(int rcuMask, int mode, int frequency, int amplitude)
    {
        //@TODO remove!
        System.out.println("RCUMask: "+rcuMask+"\nMode: "+mode+"\nFrequency: "+frequency+"\nAmplitude: "+amplitude);
        return setWaveformSettings(rcuMask, mode, frequency, amplitude, ptrRSPport);
    }
}
