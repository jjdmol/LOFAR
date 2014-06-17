/*
 * RCUMaskTest.java
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

package nl.astron.lofar.mac.apl.gui.jrsp;

import junit.framework.*;

/**
 *
 * @author balken
 */
public class RCUMaskTest extends TestCase 
{    
    public RCUMaskTest(String testName) 
    {
        super(testName);
    }

    protected void setUp() throws Exception 
    {
        
    }

    protected void tearDown() throws Exception 
    {
        
    }

    public static Test suite() 
    {
        TestSuite suite = new TestSuite(RCUMaskTest.class);
        
        return suite;
    }

    /**
     * Test of getBit method, of class nl.astron.lofar.mac.apl.gui.jrsp.RCUMask.
     */
    public void testGetBit() 
    {
        System.out.println("getBit");
        
        RCUMask instance = new RCUMask();
        
        for(int i=0; i<32; i++)
        {
            boolean expResult = false;
            boolean result = instance.getBit(i);
            assertEquals(expResult, result);
        }
        
        instance.setMask(0); // 000(...)000
        for(int i=0; i<32; i++)
        {
            assertEquals(false, instance.getBit(i));
        }
        
        instance.setMask(-1); // 111(...)111
        for(int i=0; i<32; i++)
        {
            assertEquals(true, instance.getBit(i));
        }
        
        instance.setMask(Integer.MIN_VALUE); // 100(...)000
        assertEquals(true, instance.getBit(31));
        for(int i=0; i<31; i++)
        {
            assertEquals(false, instance.getBit(i));
        }
        
        instance.setMask(Integer.MAX_VALUE); // 011(...)111
        assertEquals(false, instance.getBit(31));
        for(int i=0; i<31; i++)
        {
            assertEquals(true, instance.getBit(i));
        }        
    }

    /**
     * Test of setBit method, of class nl.astron.lofar.mac.apl.gui.jrsp.RCUMask.
     */
    public void testSetBit() 
    {
        System.out.println("setBit");
        
        RCUMask instance = new RCUMask();
        
        instance.setBit(0); // 000(...)001 = 1
        assertEquals(1, instance.intValue());
        
        instance = new RCUMask();
        instance.setBit(31); // 100(...)000 = MIN_VALUE
        assertEquals(Integer.MIN_VALUE, instance.intValue());
        
        instance = new RCUMask();
        for(int i=0; i<32; i++) // 111(...)111 = -1
        {
            instance.setBit(i);
        }
        assertEquals(-1, instance.intValue());
        
        instance = new RCUMask();
        for(int i=0; i<31; i++) // 011(...)111 = MAX_VALUE
        {
            instance.setBit(i);
        }
        assertEquals(Integer.MAX_VALUE, instance.intValue());   
    }

    /**
     * Test of clearBit method, of class nl.astron.lofar.mac.apl.gui.jrsp.RCUMask.
     */
    public void testClearBit() 
    {
        System.out.println("clearBit");
        
        RCUMask instance = new RCUMask(-1);
    
        instance.clearBit(31);
        assertEquals(Integer.MAX_VALUE, instance.intValue());
        
        for(int i=0; i<32; i++)
        {
            instance.clearBit(i);
        }
        assertEquals(0, instance.intValue());        
    }
    
    /**
     * Test of flipBit method, of class nl.astron.lofar.mac.apl.gui.jrsp.RCUMask.
     */
    public void testFlipBit() 
    {
        System.out.println("flipBit");
        
        RCUMask instance = new RCUMask(-1);
    
        instance.flipBit(31);
        assertEquals(Integer.MAX_VALUE, instance.intValue());    
    }
}
