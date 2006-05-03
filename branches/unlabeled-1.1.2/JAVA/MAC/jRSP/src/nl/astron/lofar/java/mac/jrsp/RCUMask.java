package nl.astron.lofar.mac.apl.gui.jrsp;

/**
 * The RCUMask can be used to construct or alter a int(eger) rcumask that is
 * used to determine which boards should be accessed by a function in the Board 
 * class.
 * The mask can be set through passing a int or editing every bit indepentedly 
 * of each other: setMask(int value), setBit(int index) and clearBit(int index).
 * The methods setBit and clearBit are used to either set or unset (clear) a
 * bit. 
 *
 * Because it's important that RCUMask doesn't generate false rcumask's a jUnit
 * testcase has been made that runs several tests on this class. RCUMaskTest,
 * the test class can be found in the Test Packages.
 * @see nl.astron.lofar.mac.apl.gui.jrsp.RCUMaskTest
 *
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
     * Creates a new instance of RCUMask based on two other RCUMasks (performs
     * bitwise or on the two board).
     */
    public RCUMask(RCUMask one, RCUMask two)
    {
        itsMask = one.getMask() | two.getMask();
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
    
    /**
     * Flips the bit specified by the index.
     * @param   index   Index of the bit to be cleared.
     */
    public void flipBit(int aIndex)
    {
        if (getBit(aIndex))
        {
            clearBit(aIndex);
        }
        else
        {
            setBit(aIndex);
        }
    }
    
    /**
     * Returns 32, the size of a Java integer.
     */
    public int getSize()
    {
        return 32;
    }
    
}
