/*
 * IPluginPanel.java
 *
 * Created on January 16, 2006, 4:31 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.otb.panels;

/**
 *
 * @author blaakmeer
 */
public interface IPluginPanel {
    public void initializePlugin(nl.astron.lofar.otb.MainFrame mainframe);
    public String getFriendlyName();

}
