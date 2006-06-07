/*
 * IViewPanel.java
 *
 * Created on 16 mei 2006, 16:10
 *
 * This interface gives all the public methods for ViewPanels
 * 
 */

package nl.astron.lofar.sas.otb.util;

import java.awt.Component;
import javax.swing.JPanel;
import nl.astron.lofar.sas.otb.MainFrame;

/**
 *
 * @author coolen
 */
public interface IViewPanel {
    public JPanel getInstance();
    public void setMainFrame(MainFrame aMainFrame);
    public void setContent(Object anObject);
    public void setAllEnabled(boolean enabled);
    public void enableButtons(boolean visible);
    public void setButtonsVisible(boolean visible);
    public String getShortName();
    public boolean hasPopupMenu();
    public boolean isSingleton();
    public void createPopupMenu(Component aComponent, int x, int y);
    public void popupMenuHandler(java.awt.event.ActionEvent evt);
    public void addActionListener(java.awt.event.ActionListener listener);
    public void removeActionListener(java.awt.event.ActionListener listener);
}
