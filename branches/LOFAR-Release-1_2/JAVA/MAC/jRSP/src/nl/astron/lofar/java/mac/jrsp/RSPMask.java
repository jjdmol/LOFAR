/*
 * RSPMask.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.java.mac.jrsp;

/**
 *
 * @author balken
 */
public class RSPMask implements IMask {
    /** The actual mask */
    int itsMask;
    
    /**
     * Creates a new instance of RCUMask.
     */
    public RSPMask()
    {
        itsMask = 1;
    }
        
    /**
     * Creates a new instance of RCUMask based on two other RCUMasks (performs
     * bitwise or on the two board).
     * @param   one         One of the two RCUMasks used to create a new one.
     * @param   two         The other of the two masks to create the new one.
     */
    public RSPMask(RSPMask one, RSPMask two)
    {
        itsMask = one.intValue() | two.intValue();
    }
    
    /**
     * Overloaded constructor that creates a new instance of RCUMask based on
     * the value passed as parameter.
     * @param   aMask       A already existing mask
     */
    public RSPMask(int aMask)
    {
        itsMask = aMask;
    }
    
    /**
     * Represents the Mask as a int value.
     * @return  The int value representing the Mask
     */
    public int intValue()
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
    public int size()
    {
        return 32;
    }    
}
