/*
 * WGRegisterTypeTest.java
 * JUnit based test
 *
 * Created on May 3, 2006, 2:10 PM
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
