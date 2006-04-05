package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * @author balken
 */
public class RCUMask 
{
    /** The actual mask */
    int itsMask;
    
    /**
     * Creates a new instance of RCUMask.
     */
    public RCUMask()
    {
        itsMask = 0;
    }
    
    /**
     * Overloaded constructor that creates a new instance of RCUMask based on
     * the value passed as parameter.
     * @param   aMask   A already existing mask
     */
    public RCUMask(int aMask)
    {
        itsMask = aMask;
    }
    
    /**
     * Returns the mask.
     * @return  itsMask
     */
    public int getMask()
    {
        return itsMask;
    }
    
    /**
     * Sets the mask to the new value.
     * @param   aMask
     */
    public void setMask(int aMask)
    {
        itsMask = aMask;
    }
    
    /**
     * Returns the value of the bit at the given index.
     * @param   index   Index of the bit to be returned.
     */
    public boolean getBit(int aIndex)
    {
        int bitMask = 1 << aIndex;
        return (itsMask & bitMask) == bitMask;
    }
    
    /**
     * Sets the value of the at the given index to 1.
     * @param   index   Index of the bit to be set.
     */
    public void setBit(int aIndex)
    {
        itsMask = itsMask | (1 << aIndex);
    }
    
    /**
     * Clears the bit specified by the index.
     * @param   index   Index of the bit to be cleared.
     */
    public void clearBit(int aIndex)
    {
        itsMask = itsMask & ~(1 << aIndex);
    }
}
