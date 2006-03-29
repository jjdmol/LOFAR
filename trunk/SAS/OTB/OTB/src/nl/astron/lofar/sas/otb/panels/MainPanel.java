/*
 * MainPanel.java
 *
 * Created on January 13, 2006, 2:58 PM
 */

package nl.astron.lofar.sas.otb.panels;
import java.io.File;
import java.rmi.RemoteException;

import javax.swing.JOptionPane;
import nl.astron.lofar.sas.otb.*;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.*;
import nl.astron.lofar.sas.otbcomponents.LoadFileDialog;
import nl.astron.lofar.sas.otbcomponents.TreeInfoDialog;
import org.apache.log4j.Logger;

/**
 *
 * @author  blaakmeer/coolen
 */
public class MainPanel extends javax.swing.JPanel 
                       implements IPluginPanel {

    static Logger logger = Logger.getLogger(MainPanel.class);
    static String name = "Home";

    /** Creates new form MainPanel */
    public MainPanel() {
        initComponents();
        initializeButtons();
    }
    
           
    /** 
     * Initializes the buttonpanel. Every tab has different buttons 
     */
    public void initializeButtons() {
        buttonPanel1.removeAllButtons();
        if (itsTabFocus.equals("PIC")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("New");
            buttonPanel1.addButton("Delete");
            buttonPanel1.setButtonEnabled("Delete",false);
            buttonPanel1.addButton("View");
            buttonPanel1.setButtonEnabled("View",false);
        } else if (itsTabFocus.equals("VIC")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("Delete");
            buttonPanel1.setButtonEnabled("Delete",false);
        } else if (itsTabFocus.equals("Templates")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("New");
            buttonPanel1.addButton("Duplicate");
            buttonPanel1.setButtonEnabled("Duplicate",false);
            buttonPanel1.addButton("Modify");
            buttonPanel1.setButtonEnabled("Modify",false);
        } else if (itsTabFocus.equals("Components")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("New");
            buttonPanel1.addButton("Modify");            
            buttonPanel1.setButtonEnabled("Modify",false);
        } else if (itsTabFocus.equals("Query Results")) {
        
        }
        if (!itsTabFocus.equals("Components")) {
            buttonPanel1.addButton("Info");
            buttonPanel1.setButtonEnabled("Info",false);
        }
        buttonPanel1.addButton("Quit");
        buttonsInitialized=true;
    }

    /** 
     * Perform initialization of the plugin. The OTDB is accessed
     * in this method to get user information and to initialize
     * the tables. 
     *
     * @param mainframe MainFrame reference
     */
    public boolean initializePlugin(MainFrame mainframe) {
        itsMainFrame = mainframe;
        
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }
        
        initializeTabs();
        
        return true;
        
    }
    
    /** 
     * Initializes the tab-panels. Each tab has a specific table model that
     * contains the data for the table in the tab
     */
    public void initializeTabs() {
        PICtableModel PICmodel = new PICtableModel(itsMainFrame.getOTDBrmi());
        PICPanel.setTableModel(PICmodel);

        VICtableModel VICmodel = new VICtableModel(itsMainFrame.getOTDBrmi());
        VICPanel.setTableModel(VICmodel);
        
        TemplatetableModel Templatemodel = new TemplatetableModel(itsMainFrame.getOTDBrmi());
        TemplatesPanel.setTableModel(Templatemodel);
        
        
        //TODO: do the same for the other tabs
    }
    
    /** 
     * Returns the human-readable name of this panel
     *
     * @return human-readable name of the panel
     */
    public String getFriendlyName() {
        return getFriendlyNameStatic();
    }
    
    public boolean hasChanged() {
        return changed;
    }
    
    public void setChanged(boolean flag) {
        changed = flag;
    }
    
    public void checkChanged() {
        if (this.hasChanged()) {
            if (itsTabFocus.equals("PIC")) {
                if (!((PICtableModel)PICPanel.getTableModel()).fillTable()) {
                    logger.debug("error filling PICtable");
                }
            } else if (itsTabFocus.equals("VIC")) {
                if (!((VICtableModel)VICPanel.getTableModel()).fillTable()) {
                    logger.debug("error filling VICtable");
                }            
            } else if (itsTabFocus.equals("Templates")) {
                if (!((TemplatetableModel)TemplatesPanel.getTableModel()).fillTable()) {
                    logger.debug("error filling templatetable");
                }
            }   
            this.setChanged(false);
            this.validateButtons();
        } 
    }
    
    /** 
     * Static method that returns the human-readable name of this panel
     *
     * @return human-readable name of the panel
     */
    public static String getFriendlyNameStatic() {
        return name;
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jTabbedPane1 = new javax.swing.JTabbedPane();
        PICPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        VICPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        TemplatesPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        ComponentsPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        QueryResultsPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        jPanel1 = new javax.swing.JPanel();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jTabbedPane1.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
            public void propertyChange(java.beans.PropertyChangeEvent evt) {
                jTabbedPane1PropertyChange(evt);
            }
        });
        jTabbedPane1.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                jTabbedPane1StateChanged(evt);
            }
        });

        PICPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                PICPanelMouseClicked(evt);
            }
        });

        jTabbedPane1.addTab("PIC", PICPanel);

        VICPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                VICPanelMouseClicked(evt);
            }
        });

        jTabbedPane1.addTab("VIC", VICPanel);

        TemplatesPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                TemplatesPanelMouseClicked(evt);
            }
        });

        jTabbedPane1.addTab("Templates", TemplatesPanel);

        ComponentsPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                ComponentsPanelMouseClicked(evt);
            }
        });

        jTabbedPane1.addTab("Components", ComponentsPanel);

        QueryResultsPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                QueryResultsPanelMouseClicked(evt);
            }
        });

        jTabbedPane1.addTab("Query Results", QueryResultsPanel);

        jTabbedPane1.addTab("Admin", jPanel1);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

    }// </editor-fold>//GEN-END:initComponents

    private void jTabbedPane1PropertyChange(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_jTabbedPane1PropertyChange
        logger.debug("MAIN property changed reload active tab when needed");
        this.checkChanged();
    }//GEN-LAST:event_jTabbedPane1PropertyChange

    private void QueryResultsPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_QueryResultsPanelMouseClicked
        validateButtons();
    }//GEN-LAST:event_QueryResultsPanelMouseClicked

    private void ComponentsPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_ComponentsPanelMouseClicked
        validateButtons();
    }//GEN-LAST:event_ComponentsPanelMouseClicked

    private void TemplatesPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_TemplatesPanelMouseClicked
        validateButtons();
    }//GEN-LAST:event_TemplatesPanelMouseClicked

    private void VICPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_VICPanelMouseClicked
        validateButtons();
    }//GEN-LAST:event_VICPanelMouseClicked

    private void PICPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_PICPanelMouseClicked
        validateButtons();
    }//GEN-LAST:event_PICPanelMouseClicked

    private void jTabbedPane1StateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_jTabbedPane1StateChanged
        itsTabFocus=jTabbedPane1.getTitleAt(jTabbedPane1.getSelectedIndex());
        logger.debug("Tab changed: "+ itsTabFocus);
        if (buttonsInitialized) {        
            initializeButtons();
            validateButtons();
        }
    }//GEN-LAST:event_jTabbedPane1StateChanged

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt.getActionCommand());
        buttonPanelAction(evt.getActionCommand());
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    
    /** Returns the id of the selected Tree */
    private int getSelectedTreeID() {
        int treeID=0;
        int aRow=-1;
        if (itsTabFocus.equals("PIC")) {
            aRow = PICPanel.getSelectedRow();
            if ( aRow > -1) {
                treeID = ((Integer)PICPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                if (treeID > 0) {
                    itsMainFrame.setTreeID(treeID);
                } else {
                    logger.debug("Tree not found");
                }
            }
        } else if (itsTabFocus.equals("VIC")) {
            aRow = VICPanel.getSelectedRow();
            if ( aRow > -1) {
                treeID = ((Integer)VICPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                if (treeID > 0) {
                    itsMainFrame.setTreeID(treeID);
                } else {
                    logger.debug("Tree not found");
                }
            }
        } else if (itsTabFocus.equals("Templates")) {
            aRow = TemplatesPanel.getSelectedRow();
            if ( aRow > -1) {
                treeID = ((Integer)TemplatesPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                if (treeID > 0) {
                    itsMainFrame.setTreeID(treeID);
                } else {
                    logger.debug("Tree not found");
                }
            }
        }
        return treeID;
    }
    
    /** Perform actions depending on the Button pressed and the Tab active
     *
     * @param   aButton     Name of the pressed button
     */
    private void buttonPanelAction(String aButton) {
        logger.debug("Button pressed: "+aButton+ "  ActiveTab: " + itsTabFocus);
        int treeID=getSelectedTreeID();
        if (aButton.equals("Quit")) {
            itsMainFrame.exit();
        }
        if (itsTabFocus.equals("PIC")) {
            if (treeID > 0) {
                itsMainFrame.setTreeID(treeID);
            } else {
                JOptionPane.showMessageDialog(null,"You didn't select a tree",
                        "Tree selection warning",
                        JOptionPane.WARNING_MESSAGE);
                return;
            }
            if (aButton.equals("Query Panel")) {
                // TODO open Query Panel
                ToDo();
            } else if (aButton.equals("New")) {
                File aFile=getFile("PIC-tree");
                // TODO go to Resultbrowser panel with new treeid
                ToDo();
            } else if (aButton.equals("Delete")) {
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree ?","Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        if (itsMainFrame.getOTDBrmi().getRemoteMaintenance().deleteTree(treeID)) {
                            ((PICtableModel)PICPanel.getTableModel()).fillTable();
                        } else {
                            logger.debug("Failed to delete tree");
                        }
                    } catch (RemoteException ex) {
                        logger.debug("Remote error during deleteTree: "+ ex);
                    }
                }
            } else if (aButton.equals("View")) {
                // TODO  goto result browser with selected TreeID
                ToDo();
            } else if (aButton.equals("Info")) {
                if (itsMainFrame.getTreeID() > 0) {
                    if (viewInfo(itsMainFrame.getTreeID())) {
                        logger.debug("Tree has been changed, reloading tableline");
                        ((PICtableModel)PICPanel.getTableModel()).refreshRow(PICPanel.getSelectedRow());
                    }
                }
            }
        } else if (itsTabFocus.equals("VIC")) {
            if (treeID > 0) {
                itsMainFrame.setTreeID(treeID);
            } else {
                JOptionPane.showMessageDialog(null,"You didn't select a tree",
                        "Tree selection warning",
                        JOptionPane.WARNING_MESSAGE);
                return;
            }
            if (aButton.equals("Query Panel")) {
                // TODO open Query Panel
                ToDo();
            } else if (aButton.equals("Delete")) {
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree ?","Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        if (itsMainFrame.getOTDBrmi().getRemoteMaintenance().deleteTree(treeID)) {
                            ((VICtableModel)VICPanel.getTableModel()).fillTable();
                        } else {
                            logger.debug("Failed to delete tree");
                        }
                    } catch (RemoteException ex) {
                        logger.debug("Remote error during deleteTree: "+ ex);
                    }
                }
            } else if (aButton.equals("Info")) {
                if (itsMainFrame.getTreeID() > 0) {
                    if (viewInfo(itsMainFrame.getTreeID()) ) {
                        logger.debug("Tree has been changed, reloading tableline");
                        ((VICtableModel)VICPanel.getTableModel()).refreshRow(VICPanel.getSelectedRow());
                    }
                }
            }
        } else if (itsTabFocus.equals("Templates")) {
            jOTDBtree aTree=null;
            String aTreeState="";
        
            if (treeID > 0) {
                itsMainFrame.setTreeID(treeID);
                try {
                    aTree =    itsMainFrame.getOTDBrmi().getRemoteOTDB().getTreeInfo(treeID,false);
                    aTreeState=itsMainFrame.getOTDBrmi().getRemoteTypes().getTreeState(aTree.state);
                } catch (RemoteException ex) {
                    logger.debug("Error during Remote treeMaintenance");
                }
            } else {
                JOptionPane.showMessageDialog(null,"You didn't select a tree",
                        "Tree selection warning",
                        JOptionPane.WARNING_MESSAGE);
                return;
            }
            if (aButton.equals("Query Panel")) {
                ToDo();
            } else if (aButton.equals("New")) {
                TemplateConstructionPanel aP=(TemplateConstructionPanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.TemplateConstructionPanel", false, true);
                if (aP != null) {
                    itsMainFrame.showPanel(aP.getFriendlyName());
                }
            } else if (aButton.equals("Duplicate")) {
                
                // TODO look if a template was chosen to duplicate first
                // and set it in the panel
                ToDo();
            } else if (aButton.equals("Modify")) {
                if (aTreeState.equals("idle")) {
                    TemplateConstructionPanel aP =(TemplateConstructionPanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.TemplateConstructionPanel", false, true);
                    if (aP != null) {
                        itsMainFrame.showPanel(aP.getFriendlyName());
                    }
                } else {
                    TemplateMaintenancePanel aP =(TemplateMaintenancePanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.TemplateMaintenancePanel", false, true);
                    if (aP != null) {
                        itsMainFrame.showPanel(aP.getFriendlyName());
                    }
                }
            } else if (aButton.equals("Info")) {
                if (itsMainFrame.getTreeID() > 0) {
                    if (viewInfo(itsMainFrame.getTreeID()) ) {
                        logger.debug("Tree has been changed, reloading table line");
                        ((TemplatetableModel)TemplatesPanel.getTableModel()).refreshRow(TemplatesPanel.getSelectedRow());
                    }
                }
            }
        } else if (itsTabFocus.equals("Components")) {
            if (aButton.equals("Query Panel")) {
                ToDo();
            } else if (aButton.equals("New")) {
                File aFile=getFile("VIC-component");
                // TODO Component Maintenance Panel with new id
                ToDo();
            } else if (aButton.equals("Modify")) {
                ComponentMaintenancePanel aP = (ComponentMaintenancePanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ComponentMaintenancePanel", false, true);
                if (aP != null) {
                    itsMainFrame.showPanel(aP.getFriendlyName());
                }
            }
        } else if (itsTabFocus.equals("Query Results")) {
            ToDo();
        } else {
            logger.debug("Other command found: "+aButton);
        }
    }
    
    /** Launch TreeInfoDialog,
     *
     * @param  aTreeID  The ID of the chosen tree.
     */
    private boolean viewInfo(int aTreeID) {
        logger.debug("viewInfo for treeID: " + aTreeID);
        //get the selected tree from the database
        
        try {
            jOTDBtree aSelectedTree=itsMainFrame.getOTDBrmi().getRemoteOTDB().getTreeInfo(aTreeID, false);
            
            if (aSelectedTree != null) {
                // show treeInfo dialog
                treeInfoDialog = new TreeInfoDialog(itsMainFrame,true,aSelectedTree, itsMainFrame);
                treeInfoDialog.setLocationRelativeTo(this);
                treeInfoDialog.setVisible(true);

                if (treeInfoDialog.isChanged()) {
                    logger.debug("tree has been changed and saved");
                } else {
                    logger.debug("tree has not been changed");
                }
               
            } else {
                logger.debug("no tree selected");
            }
        } catch (Exception e) {
            logger.debug("Error in viewInfo: " + e);
        }
        return treeInfoDialog.isChanged();
    }
    
    /** Launch LoadFileDialog to get a file to work with.
     *
     * @param   aType   PIC-tree or VIC-component
     *
     */
    private File getFile(String aType) {
        File aFile=null;
        String aStatus="";
        String aDescription="";
        
        // show login dialog
        loadFileDialog = new LoadFileDialog(itsMainFrame,true,aType);
        loadFileDialog.setLocationRelativeTo(this);
        loadFileDialog.setVisible(true);
        if(loadFileDialog.isOk()) {
            aDescription = loadFileDialog.getDescription();
            aStatus = loadFileDialog.getStatus();
            aFile = loadFileDialog.getFile();       
        } else {
            logger.info("No File chosen");
        }
        if (aFile != null && aFile.exists()) {
            logger.debug("File to load: " + aFile.getName()); 
            logger.debug("Status: " + aStatus);
            logger.debug("Description: "+ aDescription);
        }
        return aFile;        
    }
    
    private void validateButtons() {
        // depending on the tabfocus and the info the selected row contains
        // certain buttons are valid and others need to be grey out
        jOTDBtree aTree=null;
        String aTreeState="";
        String aClassif="";
        
        int treeID=getSelectedTreeID();
        logger.debug("Selected Tree: "+treeID);
        if (treeID > 0) {
            try {
                aTree =    itsMainFrame.getOTDBrmi().getRemoteOTDB().getTreeInfo(treeID,false);
                aTreeState=itsMainFrame.getOTDBrmi().getRemoteTypes().getTreeState(aTree.state);
                aClassif=itsMainFrame.getOTDBrmi().getRemoteTypes().getClassif(aTree.classification);
            } catch (RemoteException ex) {
                logger.debug("Couldn't get Tree");
                return;
            }
        }
        if (itsTabFocus.equals("PIC")) {
            if (treeID>0) {
                if (aTreeState.equals("active")) {
                    buttonPanel1.setButtonEnabled("Delete",false);
                } else {
                    buttonPanel1.setButtonEnabled("Delete",true);                    
                }
                buttonPanel1.setButtonEnabled("View",true);
                buttonPanel1.setButtonEnabled("Info",true);
            } else {
                buttonPanel1.setButtonEnabled("Delete",false);
                buttonPanel1.setButtonEnabled("View",false);
                buttonPanel1.setButtonEnabled("Info",false);
            }
        } else if (itsTabFocus.equals("VIC")) {
            if (treeID>0) {
                if (aTreeState.equals("idle") || aTreeState.endsWith("being specified") || aTreeState.equals("finished")
                || aTreeState.equals("aborted") || aTreeState.equals("obsolete")) {
                    buttonPanel1.setButtonEnabled("Delete",true);
                } else {
                    buttonPanel1.setButtonEnabled("Delete",false);                    
                }
                buttonPanel1.setButtonEnabled("Info",true);
            } else {
                buttonPanel1.setButtonEnabled("Delete",false);
                buttonPanel1.setButtonEnabled("Info",false);
            }
        } else if (itsTabFocus.equals("Templates")) {
            if (treeID > 0) {
                if (aTreeState.equals("idle") ||
                        aTreeState.equals("being specified") ||
                        aTreeState.equals("specified") ||
                        aTreeState.equals("approved")) {
                    buttonPanel1.setButtonEnabled("Duplicate",true);
                    buttonPanel1.setButtonEnabled("Modify",true);                    
                } else {
                    buttonPanel1.setButtonEnabled("Duplicate",false);
                    buttonPanel1.setButtonEnabled("Modify",false);                                        
                }
                buttonPanel1.setButtonEnabled("Info",true);
            } else {
                buttonPanel1.setButtonEnabled("Duplicate",false);
                buttonPanel1.setButtonEnabled("Modify",false);                    
                buttonPanel1.setButtonEnabled("Info",false);                
            }
        } else if (itsTabFocus.equals("Components")) {
            buttonPanel1.setButtonEnabled("Modify",true);
        } else if (itsTabFocus.equals("Query Results")) {
        }
    }
   
    private void ToDo() {
        JOptionPane.showMessageDialog(this,"This code still needs to be implemented","Warning",JOptionPane.WARNING_MESSAGE);
    }
    
    private MainFrame      itsMainFrame;
    private String         itsTabFocus="PIC";
    private boolean        buttonsInitialized=false;
    private LoadFileDialog loadFileDialog;
    private TreeInfoDialog treeInfoDialog;
    private boolean        changed=false;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.TablePanel ComponentsPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel PICPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel QueryResultsPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel TemplatesPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel VICPanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JTabbedPane jTabbedPane1;
    // End of variables declaration//GEN-END:variables
    
}
