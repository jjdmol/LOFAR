/*
 * MaskSelectionPanelTest.java
 * JUnit based test
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

package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import junit.framework.*;
import nl.astron.lofar.mac.apl.gui.jrsp.RCUMask;

/**
 *
 * @author balken
 */
public class MaskSelectionPanelTest extends TestCase {
    
    public MaskSelectionPanelTest(String testName) {
        super(testName);
    }

    public static Test suite() {
        TestSuite suite = new TestSuite(MaskSelectionPanelTest.class);
        
        return suite;
    }

    /**
     * Test of setBoard method, of class nl.astron.lofar.mac.apl.gui.jrsp.panels.control.MaskSelectionPanel.
     */
    public void testSetBoard() {
        System.out.println("setBoard");
        
        MaskSelectionPanel instance = new MaskSelectionPanel();

        instance.setBoard(2);
        //assertEquals(65536, instance.getRCUMask().getMask());
        
        instance.setBoard(1);
        //assertEquals(256, instance.getRCUMask().getMask());
                
        // dit werkt als ik de eerst checkbox geselecteerd krijg.
    }
    
}
