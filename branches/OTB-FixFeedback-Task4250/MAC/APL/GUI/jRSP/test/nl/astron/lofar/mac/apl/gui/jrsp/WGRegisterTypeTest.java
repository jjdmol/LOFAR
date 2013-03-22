/*
 * WGRegisterTypeTest.java
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
public class WGRegisterTypeTest extends TestCase {
    
    public WGRegisterTypeTest(String testName) {
        super(testName);
    }

    protected void setUp() throws Exception {
    }

    protected void tearDown() throws Exception {
    }

    public static Test suite() {
        TestSuite suite = new TestSuite(WGRegisterTypeTest.class);
        
        return suite;
    }

    /**
     * Test of getPhase method, of class nl.astron.lofar.mac.apl.gui.jrsp.WGRegisterType.
     */
    public void testGetPhase() {
        System.out.println("getPhase");
        
        WGRegisterType instance = new WGRegisterType();
        
        instance.phase = 0;
        short expResult = 0;
        short result = instance.getPhase();
        assertEquals(expResult, result);
        
        instance.phase = 128;
        expResult = 180;
        result = instance.getPhase();
        assertEquals(expResult, result);
        
        instance.phase = 255;
        expResult = 360;
        result = instance.getPhase();
        assertEquals(expResult, result);        
    }

    /**
     * Test of setPhase method, of class nl.astron.lofar.mac.apl.gui.jrsp.WGRegisterType.
     */
    public void testSetPhase() {
        System.out.println("setPhase");
        
        WGRegisterType instance = new WGRegisterType();
        
        short phase = 0;
        short expResult = 0;
        instance.setPhase(phase);
        assertEquals(expResult, instance.phase);
        
        phase = 180;
        expResult = 127;
        instance.setPhase(phase);
        assertEquals(expResult, instance.phase); 
        
        phase = 360;
        expResult = 255;
        instance.setPhase(phase);
        assertEquals(expResult, instance.phase);        
    }
    
}
