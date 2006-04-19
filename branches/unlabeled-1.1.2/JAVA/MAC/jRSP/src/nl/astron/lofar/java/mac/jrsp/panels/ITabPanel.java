/*
 * ITabPanel.java
 *
 * Created on April 19, 2006, 12:03 PM
 */

package nl.astron.lofar.mac.apl.gui.jrsp.panels;

/**
 * This interface declares some methods that are needed for interaction with the
 * main panel.
 *
 * @author balken
 */
public interface ITabPanel 
{
    /**
     * Used to initialize the ITabPanel and give it a refrence to the main panel.
     * 
     * @param mainPanel   The MainPanel.
     */
    public void init(MainPanel mainPanel);
    
    /**
     * Method that can be called by the main panel to update this panel.
     */
    public void update();    
}
