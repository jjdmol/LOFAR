/*
 * MainFrame.java
 *
 * Created on January 11, 2006, 9:31 AM
 */

package nl.astron.lofar.sas.otb;

import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import nl.astron.lofar.sas.otb.exceptions.*;
import nl.astron.lofar.sas.otb.panels.*;
import nl.astron.lofar.sas.otb.util.*;
import nl.astron.lofar.sas.otbcomponents.*;

/**
 *
 * @author  blaakmeer
 */
public class MainFrame extends javax.swing.JFrame {

    // Create a Log4J logger instance
    static Logger logger = Logger.getLogger(MainFrame.class);

    // fields declaration
    private HashMap<String,PluginPanelInfo>  itsPlugins;
    private JPanel                           itsActivePanel;
    private OtdbRmi                          itsOtdbRmi;
    private StorageLocation                  itsStorageLocation;
    private MACNavigatorInteraction          itsMACInteraction;
    private static UserAccount               itsUserAccount;
    
    
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
    
    /** Creates new form MainFrame */
    public MainFrame() {
        itsPlugins = new HashMap<String,PluginPanelInfo>();
        itsOtdbRmi = null;
        itsStorageLocation = new StorageLocation(itsOtdbRmi);
        itsMACInteraction = new MACNavigatorInteraction(itsStorageLocation);

        initComponents();

        login();
        
        showPanel(MainPanel.getFriendlyNameStatic());
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
    public void registerPlugin(String pluginName, boolean menuitem, boolean toolbar) {
        try {
            JButton jButton = null;
            JMenuItem jMenuItem = null;
            
            JPanel p = (JPanel)Class.forName(pluginName).newInstance();
            ((IPluginPanel)p).initializePlugin(this);
            String friendlyName = ((IPluginPanel)p).getFriendlyName();

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
            
            PluginPanelInfo ppi = new PluginPanelInfo(p, pluginName,jButton,jMenuItem);
            itsPlugins.put(friendlyName, ppi);
        }
        catch(Exception e) {
            logger.fatal(e);
        }
    }
    
    /** Unregisters the panel with the given name and removes it from the 
      * toolbar and Plugin menu if necessary
      *
      * @param friendlyName name of the plugin
     */
    public void unregisterPlugin(String friendlyName) {
        try {
            PluginPanelInfo ppi = itsPlugins.get(friendlyName);
            
            // remove the toolbar button and menu item
            if(ppi.toolbarButton != null) {
                jToolBarPlugins.remove(ppi.toolbarButton);
            }
            if(ppi.menuItem != null) {
                jMenuPlugins.remove(ppi.menuItem);
            }
            itsPlugins.remove(friendlyName);
        }
        catch(Exception e) {
            logger.fatal(e);
        }
    }
    
    /** Hides the current panel and shows the the panel with the given 
      *
      * @param friendlyName name of the panel
     */
    public void showPanel(String friendlyName) {
        PluginPanelInfo ppi = itsPlugins.get(friendlyName);
        if(itsActivePanel != null) {
            getContentPane().remove(itsActivePanel);
            itsActivePanel.invalidate();
        }
        itsActivePanel = ppi.panel;
        getContentPane().add(itsActivePanel, java.awt.BorderLayout.CENTER);

        itsActivePanel.updateUI();
    }
    
    /** Returns a reference to the one and only UserAccount object
      *
      * @return reference to the UserAccount object
     */
    public UserAccount getUserAccount() {
        return itsUserAccount;
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
        logger.info("Exit requested");
        setVisible(false);
        dispose();
    }//GEN-LAST:event_jMenuItemExitActionPerformed

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
                    itsUserAccount = new UserAccount(itsOtdbRmi, userName, password);
                    accessAllowed = true;
                    itsMACInteraction.setCurrentUser(userName,password);
                    
                    statusPanelMainFrame.setText(StatusPanel.MIDDLE,userName);
                    
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
        }
        itsActivePanel.updateUI();
        itsActivePanel = null;
        
        // remove all panels
        while(itsPlugins.size() > 0) {
            Iterator i = itsPlugins.keySet().iterator();
            if(i.hasNext()) {
                String friendlyName = (String)i.next();
                unregisterPlugin(friendlyName);
            }
        }
        itsPlugins.clear();
        itsUserAccount = null;
    }

    /** Registers the panels that every user has to see
     */
    private void registerDefaultPlugins() {
        registerPlugin("nl.astron.lofar.sas.otb.panels.MainPanel", false, true);
        registerPlugin("nl.astron.lofar.sas.otb.panels.SamplePanel",true,true);
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
                new MainFrame().setVisible(true);
            }
        });
    }
    
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
