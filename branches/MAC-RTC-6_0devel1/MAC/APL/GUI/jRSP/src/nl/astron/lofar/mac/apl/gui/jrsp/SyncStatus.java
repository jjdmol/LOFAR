package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * This class is used by BoardStatus to store data about the Sync status.
 * @see nl.astron.lofar.mac.apl.gui.jrps.Board
 *
 * @author  balken
 */
public class SyncStatus
{
    /** ext count. */
    public int extCount;
    
    /** sync count. */
    public int syncCount;
    
    /** sample count. */
    public int sampleOffset;
    
    /** slice count. */
    public int sliceCount;
}
