/*
 * IMask.java
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
 * The IMask interface has to be implemented by Mask classes. These classes are
 * used to specify which boards or RCU's.
 *
 * @author balken
 */
public interface IMask {
    
    /**
     * Represents the Mask as a int value.
     * @return  The int value representing the Mask
     */
    public int intValue();
    
    /**
     * Sets the Mask based on the int value.
     * @param   aValue       Int value used to base Mask on
     */
    public void setMask(int aValue);
    
    /**
     * Returns the value of the bit at the given index.
     * @param   aIndex      Index of the bit to be returned.
     * @return  A boolean value representing the 0/1 state of the bit.
     */
    public boolean getBit(int aIndex);
        
    /**
     * Sets the value of the at the given index to 1.
     * @param   aIndex      Index of the bit to be set.
     */
    public void setBit(int aIndex);
        
    /**
     * Clears the bit specified by the index.
     * @param   aIndex      Index of the bit to be cleared.
     */
    public void clearBit(int aIndex);
    
    /**
     * Flips the bit specified by the index.
     * @param   aIndex      Index of the bit to be cleared.
     */
    public void flipBit(int aIndex);
        
    /**
     * Returns the size of the mask; the number of bits.
     * @return  The mask size
     */
    public int size();    
}
