/*
 * IViewPanel.java
 *
 * Created on 16 mei 2006, 16:10
 *
 * This interface gives all the public methods for ViewPanels
 * 
 */

package nl.astron.lofar.sas.otb.util;

import nl.astron.lofar.sas.otb.MainFrame;

/**
 *
 * @author coolen
 */
public interface IViewPanel {
    public void setMainFrame(MainFrame aMainFrame);
    public void setContent(Object anObject);
    public void setAllEnabled(boolean enabled);
    public void enableButtons(boolean visible);
    public void setButtonsVisible(boolean visible);
    public String getShortName();
    public void addActionListener(java.awt.event.ActionListener listener);
    public void removeActionListener(java.awt.event.ActionListener listener);
}
