package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * The Board class is used to connect to the C++ side of the whole jRSP project.
 *
 * This class should be used as follows:
 * First you can initialize the Board, at this point you *don't* have a connection
 * on the C++ side with the boards.
 * Then you can call the connect method with the hostname that should be used.
 * At the end you call the disconnect method to end/delete our connection in the
 * native code.
 * For example, if you want to get the status:
 * 
 * String sHostName = %hostname%;
 * int iRcuMask = %rcumask%;
 *
 * Board b = new Board();                         // initialize board
 * b.connect(sHostName);                          // connect to board @ hostname
 * BoardStatus[] arrBs = b.getStatus(iRcuMask);   // get the boardStatus
 * b.disconnect();                                // disconnect the board
 * 
 *
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
    public Board()
    {
        ptrRSPport = 0;
        // @TODO activeren: ptrRSPport = init(hostname);
        boardStatus = null;
    }
    
    // Connect functions
    /**
     * Makes a connection to the RSPdriver via RSPport.
     * @param   hostname    hostname of the board.
     */
    public void connect(String hostname)
    {
        ptrRSPport = 1;
        // ptrRSPport = init(hostname);
    }
    
    /**
     * Deletes the reference to the RSPport we have in C++.
     */
    public void disconnect()
    {
        // delete(ptrRSPport);
        ptrRSPport = 0;
    }
    
    /**
     * Returns if there is a connection with the RSPport.
     * @return              True if connected, False if not.
     */
    public boolean isConnected()
    {
        return (ptrRSPport > 0);
    }
    // End of connect functions
    
    // Native functions
    private native int init(String hostname); // initializes RSPport on the C++ side, for further use.
    private native void delete(int ptrRSPport); // deletes the RSPport instance on the c++ side.
    private native BoardStatus[] retrieveStatus(int rcuMask, int ptrRSPport); // retrieves the boards.
    private native int retrieveNofBoards(int ptrRSPport); // retrieves the number of boards.
    private native boolean setWaveformSettings(int rcuMask, int mode, int frequency, int amplitude, int ptrRSPport); // Sets the waveform settings.
    private native double[] getSubbandStats(int rcuMask, int ptrRSPport); // retrieves subbandstats
    // End of native functions
    
    static
    {
        System.loadLibrary("jrsp");
    }
    
    /**
     * @return  hostname
     */
    public String getHostname()
    {
        return hostname;
    }
        
    /**
     * Returns the status of the Boards.
     * @return  BoardStatus[]   A array of BoardStatus.
     */
    public BoardStatus[] getStatus(int rcuMask)
    {
        boardStatus = retrieveStatus(rcuMask, ptrRSPport);
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
    
    /**
     * Returns the subband stats.
     * @param   rcuMask
     * @return              array of doubles.
     */
    public double[] getSubbandStats(int rcuMask)
    {
        return getSubbandStats(rcuMask, ptrRSPport);
    }
}
