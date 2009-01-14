/*
 * MainFrame.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 
 */

package nl.astron.lofar.sas.otb;

import java.awt.Color;
import java.awt.Cursor;
import java.rmi.RemoteException;
import java.util.*;
import javax.swing.*;
import nl.astron.lofar.lofarutils.LofarUtils;
import org.apache.log4j.Logger;
import nl.astron.lofar.sas.otb.exceptions.*;
import nl.astron.lofar.sas.otb.panels.*;
import nl.astron.lofar.sas.otb.util.*;
import nl.astron.lofar.sas.otbcomponents.*;

/**
 *
 * @created on 11-01-2006, 9:31
 * @author  blaakmeer
 * @version $Id$
 * @updated 
 */
public class MainFrame extends javax.swing.JFrame {

    // Create a Log4J logger instance
    static Logger logger = Logger.getLogger(MainFrame.class);

    // fields declaration
    private HashMap<String,PluginPanelInfo>  itsPlugins;
    private JPanel                           itsActivePanel;
    private StorageLocation                  itsStorageLocation;
    private MACNavigatorInteraction          itsMACInteraction;
    private SharedVars                       itsSharedVars;
    private static UserAccount               itsUserAccount;
    private String itsServer               = "";
    private String itsPort                 = "";
    private String itsDBName               = "No Database";
    
    
    /** nested class for plugin panel administration */
    public class PluginPanelInfo {
        /** Creates new PluginPanelInfo */
        public PluginPanelInfo(JPanel p, String s) {
            this(p,s,null,null);
        }
        
        public PluginPanelInfo(JPanel p, String s, JButton b) {
            this(p,s,b,null);
        }
        
        public PluginPanelInfo(JPanel p, String s, JMenuItem m) {
            this(p,s,null,m);
        }
        
        public PluginPanelInfo(JPanel p, String s, JButton b, JMenuItem m) {
            panel = p;
            className = s;
            toolbarButton = b;
            menuItem = m;
        }
        
        /** panel reference */
        public JPanel panel;
        /** classname including complete package name */
        public String className;
        /** toolbar button reference */
        public JButton toolbarButton;
        /** menuitem reference */
        public JMenuItem menuItem;
    }
    
    /** Todo Stub can be used to place ToDo panels on methods that still need to be coded during development */
    public void ToDo() {
        JOptionPane.showMessageDialog(this,"This code still needs to be implemented","Warning",JOptionPane.WARNING_MESSAGE);
    }
    
    /** Creates new form MainFrame */
    public MainFrame(String server, String port) {
        itsServer=server;
        itsPort=port;
        itsPlugins = new HashMap<String,PluginPanelInfo>();
        itsSharedVars = new SharedVars(this);
        itsStorageLocation = new StorageLocation(SharedVars.getOTDBrmi());
        itsMACInteraction = new MACNavigatorInteraction(itsStorageLocation);

        initComponents();

        login();
        

        // Since the default disabledForegroundColor might be a bit unreadable
        // we set it to black for all object in this look and feel.
        
        UIManager.put("TextArea.inactiveForeground",Color.gray);
        UIManager.put("TextField.inactiveForeground",Color.gray);
        UIManager.put("FormattedTextField.inactiveForeground",Color.gray);
        UIManager.put("ComboBox.disabledForeground",Color.gray);
        UIManager.put("CheckBoxMenuItem.disabledForeground",Color.gray);
        UIManager.put("RadioButtonMenuItem.disabledForeground",Color.gray);
        UIManager.put("Menu.disabledForeground",Color.gray);
        UIManager.put("MenuItem.disabledForeground",Color.gray);
        

        
        
        
        showPanel(MainPanel.getFriendlyNameStatic());
    }
    
    /** sets the serverName */
    public void setServer(String aServer) {
        itsServer=aServer;
    }
    
    /** gets the serverName */

    public String getServer() {
        return itsServer;
    }
    
    /** sets the portNr */
    public void setPort(String aPort) {
        itsPort=aPort;
    }
    
    /** gets the Port */

    public String getPort() {
        return itsPort;
    }
    
    /** returns the SharedVars Object that holds shared variables
     *
     */
    public SharedVars getSharedVars() {
        return itsSharedVars;
    }

    
    /** Registers the panel with the given name without adding it to the 
      * toolbar or Plugin menu
      *
      * @param pluginName full classname of the plugin
     */
    public void registerPlugin(String pluginName) {
        registerPlugin(pluginName, false, false);
    }
    
    /** Registers the panel with the given name and adds it to the 
      * toolbar and Plugin menu if requested
      *
      * @param pluginName full classname of the plugin
      * @param menuitem true if a menuitem must be added
      * @param toolbar true if a toolbarbutton must be added
     */
    public Object registerPlugin(String pluginName, boolean menuitem, boolean toolbar) {
        try {
            JButton jButton = null;
            JMenuItem jMenuItem = null;
            
            JPanel p = (JPanel)Class.forName(pluginName).newInstance();

            if ( p == null || ! ((IPluginPanel)p).initializePlugin(this)) {
                return null;
            }
            String friendlyName = ((IPluginPanel)p).getFriendlyName();

            // unregister this name (just to avoid dangling toolbar/button instances)
            this.unregisterPlugin(friendlyName);
            

            
            /** add the toolbar button and menu item */
            if(toolbar) {
                jButton = new javax.swing.JButton();
                jButton.setText(friendlyName);
                jButton.addActionListener(new java.awt.event.ActionListener() {
                    public void actionPerformed(java.awt.event.ActionEvent evt) {
                        pluginButtonActionPerformed(evt);
                    }
                });
                jToolBarPlugins.add(jButton);
            }
            if(menuitem) {
                jMenuItem = new javax.swing.JMenuItem();
                jMenuItem.setText(friendlyName);
                jMenuItem.addActionListener(new java.awt.event.ActionListener() {
                    public void actionPerformed(java.awt.event.ActionEvent evt) {
                        pluginButtonActionPerformed(evt);
                    }
                });
                jMenuPlugins.add(jMenuItem);
            }
            
            // create a new pluginpanel and add to pool
            PluginPanelInfo ppi = new PluginPanelInfo(p, pluginName,jButton,jMenuItem);
            itsPlugins.put(friendlyName, ppi);

            return p;
        }
        catch(Exception e) {
            logger.fatal(e);
        }
        return null;
    }
    
    /** Unregisters the panel with the given name and removes it from the 
      * toolbar and Plugin menu if necessary
      *
      * @param friendlyName name of the plugin
     */
    public void unregisterPlugin(String friendlyName) {
        
        try {
            PluginPanelInfo ppi = itsPlugins.get(friendlyName);
            if (ppi != null) {
                // remove the toolbar button and menu item
                if(ppi.toolbarButton != null) {
                    logger.debug("Trying to remove from toolbar");
                    jToolBarPlugins.remove(ppi.toolbarButton);
                    jToolBarPlugins.repaint();
                }
               if(ppi.menuItem != null) {
                    logger.debug("Trying to remove from menu");
                    jMenuPlugins.remove(ppi.menuItem);
                    jMenuPlugins.repaint();
                }
                itsPlugins.remove(friendlyName);
            }
            // set focus to Home
            showPanel(MainPanel.getFriendlyNameStatic());
        }
        catch(Exception e) {
            logger.fatal(e);
        }
    }
    
    /** sets the changed flag on a given panel
     * @param friendlyName  name of the panel
     * @param flag          state flag
     */
    public void setChanged(String friendlyName,boolean flag) {
        
        // for now we will need to set the (treeid-less) mainpanel to haschanged
        // and the panels that are based on the same treeid as the one that fires this
        String aTreeID="";
        if (friendlyName.contains("(") && friendlyName.contains(")")) {
            aTreeID="("+friendlyName.substring(friendlyName.indexOf('(')+1,friendlyName.indexOf(')'))+")";
        }
        Iterator it=itsPlugins.keySet().iterator();
        while (it.hasNext()) {
            String aKey=(String)it.next();
            PluginPanelInfo ppi = itsPlugins.get(aKey);
            if (ppi == null || ppi.panel == null ) {
                return;
            }
            
            // Check if panelname contains (), if not then change anyway
            // if so check if treeid is wanted treeid, 
            if (aKey.contains("(") && aKey.contains(")")) {
//                if (aKey.contains(aTreeID) && !aKey.equals(friendlyName)) {
                if (aKey.contains(aTreeID)) {
                    ((IPluginPanel)ppi.panel).setChanged(flag);
                    logger.debug("Setting changed flag for: "+aKey+" to "+flag);
                }
            } else {
                ((IPluginPanel)ppi.panel).setChanged(flag);     
                logger.debug("Setting changed flag for: "+aKey+" to "+ flag);
           }
        }
    }
    
    /** calls checkChanged on a given panel
     * @param friendlyName  name of the panel
     * @param flag          state flag
     */
    public void checkChanged(String friendlyName) {
        
        Iterator it=itsPlugins.keySet().iterator();
        while (it.hasNext()) {
            String aKey=(String)it.next();
            PluginPanelInfo ppi = itsPlugins.get(aKey);
            if (ppi == null || ppi.panel == null ) {
                return;
            }
            
            if (aKey.equals(friendlyName)) {
                // call checkchanged
                ((IPluginPanel)ppi.panel).checkChanged();

                logger.debug("Called changed for: "+aKey);
            }
        }
    }

    /** Hides the current panel and shows the the panel with the given 
      *
      * @param friendlyName name of the panel
     */
    public void showPanel(String friendlyName) {
        if (friendlyName.contains("(") && friendlyName.contains(")")) {
            String aTreeID=friendlyName.substring(friendlyName.indexOf('(')+1,friendlyName.indexOf(')'));
            itsSharedVars.setTreeID(Integer.valueOf(aTreeID));
        }
        PluginPanelInfo ppi = itsPlugins.get(friendlyName);
        if(itsActivePanel != null) {
            getContentPane().remove(itsActivePanel);
            itsActivePanel.invalidate();
        }
        if (ppi == null || ppi.panel == null ) {
            return;
        }
        itsActivePanel = ppi.panel;
        getContentPane().add(itsActivePanel, java.awt.BorderLayout.CENTER);
        ((IPluginPanel)itsActivePanel).checkChanged();
        itsActivePanel.updateUI();
    }
    
    /** Returns a reference to the one and only UserAccount object
      *
      * @return reference to the UserAccount object
     */
    public UserAccount getUserAccount() {
        return itsUserAccount;
    }
    
    /** Sets hourglassCursor
     *
     */
    public void setHourglassCursor() {
        setCursor(hourglassCursor);
    }
    
    /** Sets NormalCursor
     *
     */
    public void setNormalCursor() {
        setCursor(normalCursor);
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        statusPanelMainFrame = new nl.astron.lofar.sas.otbcomponents.StatusPanel();
        jToolBarPlugins = new javax.swing.JToolBar();
        jMenuBarMainFrame = new javax.swing.JMenuBar();
        jMenuFile = new javax.swing.JMenu();
        jMenuItemLogout = new javax.swing.JMenuItem();
        jSeparator1 = new javax.swing.JSeparator();
        jMenuItemExit = new javax.swing.JMenuItem();
        jMenuPlugins = new javax.swing.JMenu();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("Lofar Observation Tree Browser");
        getContentPane().add(statusPanelMainFrame, java.awt.BorderLayout.SOUTH);

        getContentPane().add(jToolBarPlugins, java.awt.BorderLayout.NORTH);

        jMenuFile.setMnemonic('f');
        jMenuFile.setText("File");
        jMenuItemLogout.setMnemonic('l');
        jMenuItemLogout.setText("Logout");
        jMenuItemLogout.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuItemLogoutActionPerformed(evt);
            }
        });

        jMenuFile.add(jMenuItemLogout);

        jMenuFile.add(jSeparator1);

        jMenuItemExit.setMnemonic('x');
        jMenuItemExit.setText("Exit");
        jMenuItemExit.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuItemExitActionPerformed(evt);
            }
        });

        jMenuFile.add(jMenuItemExit);

        jMenuBarMainFrame.add(jMenuFile);

        jMenuPlugins.setMnemonic('p');
        jMenuPlugins.setText("Plugins");
        jMenuBarMainFrame.add(jMenuPlugins);

        setJMenuBar(jMenuBarMainFrame);

        pack();
    }
    // </editor-fold>//GEN-END:initComponents

    private void jMenuItemLogoutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuItemLogoutActionPerformed
        logger.info("Logout requested");
        logout();
        setVisible(false);
        login();
        setVisible(true);
        showPanel(MainPanel.getFriendlyNameStatic());        
    }//GEN-LAST:event_jMenuItemLogoutActionPerformed

    private void jMenuItemExitActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuItemExitActionPerformed
        exit();
    }//GEN-LAST:event_jMenuItemExitActionPerformed

    public void exit() {
        logger.info("Exit requested");
        logout();
        setVisible(false);
        dispose();        
    }
    
    /** Event handler called when a button in the button panel is called
      *
      * @param evt 
     */
    private void pluginButtonActionPerformed(java.awt.event.ActionEvent evt) {
        logger.debug("Plugin selected: " + evt.getActionCommand());
        showPanel(evt.getActionCommand());
    }
    
    /** Shows the login dialog and creates the UserAccount object. If the user
     *  has access, the default panels are registered, the user configured
     *  plugin panels are registered and the mainframe and the initial panel 
     *  are shown.
     */
    private void login() {
        boolean accessAllowed = false;
        while(!accessAllowed) {
            // show login dialog
            LoginDialog loginDialog = new LoginDialog(this,true);
            loginDialog.setLocationRelativeTo(this);
            loginDialog.setVisible(true);
            if(loginDialog.isOk()) {
                String userName = loginDialog.getUserName();
                String password = loginDialog.getPassword();

                logger.info("User: " + userName);

                // create a useraccount object
                try {
                    itsUserAccount = new UserAccount(SharedVars.getOTDBrmi(), userName, password);
                    accessAllowed = true;
                    itsMACInteraction.setCurrentUser(userName,password);
                    
                    statusPanelMainFrame.setText(StatusPanel.MIDDLE,"User: "+userName);
                    String aC = "NO Main DB connection";
                    // Start the actual RMI connection
                    if (! SharedVars.getOTDBrmi().isConnected()) {
                        if (! SharedVars.getOTDBrmi().openConnections()) {
                            logger.debug("Error: failed to open RMI Connections");
                        } else {
                            aC = "DB connection to: "+OtdbRmi.getRMIServerName()+" Port: "+OtdbRmi.getRMIServerPort();
                        }
                    }
                    statusPanelMainFrame.setText(StatusPanel.LEFT,aC);
                    logger.debug("Trying to get DatabaseName");
                    if (itsSharedVars == null || SharedVars.getOTDBrmi() == null || 
                           OtdbRmi.getRemoteOTDB() == null) {
                        logger.info("Can't connect to Server. Session cancelled");
                        System.exit(0);
                    }
                    try {
                         itsDBName=OtdbRmi.getRemoteOTDB().getDBName();
                    } catch (RemoteException ex) {
                        logger.error("Couldn't get DatabaseName"+ ex);
                    }

                    logger.debug("DatabaseName found: "+ itsDBName);
                    statusPanelMainFrame.setText(StatusPanel.RIGHT,"Used Database: " +itsDBName);
                    registerDefaultPlugins();
                    registerUserPlugins();
                }
                catch(NoAccessException e) {
                    logger.fatal("Access Violation: " + e.getMessage());
                    JOptionPane.showMessageDialog(this,"The supplied username/password combination is unknown","Unknown username/password",JOptionPane.ERROR_MESSAGE);
                    accessAllowed = false;
                }
            }
            else {
                logger.info("Login cancelled");
                System.exit(0);
            }
        }        
    }
    
    /** Removes all panels and destroys the UserAccount object
     */
    private void logout() {
        if(itsActivePanel != null) {
            getContentPane().remove(itsActivePanel);
            itsActivePanel.invalidate();
            itsActivePanel.updateUI();
            itsActivePanel = null;
        }
        
        // remove all panels
        while(itsPlugins != null && itsPlugins.size() > 0) {
            Iterator i = itsPlugins.keySet().iterator();
            if(i.hasNext()) {
                String friendlyName = (String)i.next();
                unregisterPlugin(friendlyName);
            }
        }
        if (itsPlugins != null) {
            itsPlugins.clear();
        }
        itsUserAccount = null;
    }

    /** Registers the panels that every user has to see
     */
    private void registerDefaultPlugins() {
        registerPlugin("nl.astron.lofar.sas.otb.panels.MainPanel", false, true);
//        registerPlugin("nl.astron.lofar.sas.otb.panels.SamplePanel",true,true);
    }

    /** Registers other panels
     */
    private void registerUserPlugins() {
        // TODO: get the user's plugin panels from the OTDB and register them
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new MainFrame("","").setVisible(true);
            }
        });
    }
    
    Cursor normalCursor = new Cursor(Cursor.DEFAULT_CURSOR);
    Cursor hourglassCursor = new Cursor(Cursor.WAIT_CURSOR);
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JMenuBar jMenuBarMainFrame;
    private javax.swing.JMenu jMenuFile;
    private javax.swing.JMenuItem jMenuItemExit;
    private javax.swing.JMenuItem jMenuItemLogout;
    private javax.swing.JMenu jMenuPlugins;
    private javax.swing.JSeparator jSeparator1;
    private javax.swing.JToolBar jToolBarPlugins;
    private nl.astron.lofar.sas.otbcomponents.StatusPanel statusPanelMainFrame;
    // End of variables declaration//GEN-END:variables
    
}
