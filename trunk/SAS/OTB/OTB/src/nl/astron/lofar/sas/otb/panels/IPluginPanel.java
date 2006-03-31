/*
 * IPluginPanel.java
 *
 * Created on January 16, 2006, 4:31 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.panels;

/**
 *
 * @author blaakmeer/coolen
 */
public interface IPluginPanel {
    public boolean initializePlugin(nl.astron.lofar.sas.otb.MainFrame mainframe);
    public String getFriendlyName();
    public boolean hasChanged();
    public void setChanged(boolean changed);
    public void checkChanged();
}
