/*
 * MainPanel.java
 *
 * Created on January 13, 2006, 2:58 PM
 */

package nl.astron.lofar.sas.otb.panels;
import java.io.File;
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
    static String name = "Main";

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
            buttonPanel1.addButton("Duplicate");
        } else if (itsTabFocus.equals("VIC")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("Delete");
            buttonPanel1.addButton("Duplicate");
        } else if (itsTabFocus.equals("Templates")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("New");
            buttonPanel1.addButton("Duplicate");
            buttonPanel1.addButton("Modify");
        } else if (itsTabFocus.equals("Components")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("New");
            buttonPanel1.addButton("Modify");            
        } else if (itsTabFocus.equals("Query Results")) {
        
        }
        if (!itsTabFocus.equals("Components")) {
            buttonPanel1.addButton("View");
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
    public void initializePlugin(MainFrame mainframe) {
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
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        PICPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        VICPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        TemplatesPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        ComponentsPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        QueryResultsPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();

        setLayout(new java.awt.BorderLayout());

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

        jTabbedPane1.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                jTabbedPane1StateChanged(evt);
            }
        });

        jTabbedPane1.addTab("PIC", PICPanel);

        jTabbedPane1.addTab("VIC", VICPanel);

        jTabbedPane1.addTab("Templates", TemplatesPanel);

        jTabbedPane1.addTab("Components", ComponentsPanel);

        jTabbedPane1.addTab("Query Results", QueryResultsPanel);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

    }
    // </editor-fold>//GEN-END:initComponents

    private void jTabbedPane1StateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_jTabbedPane1StateChanged
        if (jTabbedPane1.getTitleAt(jTabbedPane1.getSelectedIndex())  != null && buttonsInitialized) {        
            itsTabFocus=jTabbedPane1.getTitleAt(jTabbedPane1.getSelectedIndex());
            initializeButtons();
        }
    }//GEN-LAST:event_jTabbedPane1StateChanged

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt.getActionCommand());
        buttonPanelAction(evt.getActionCommand());
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    /** Perform actions depending on the Button pressed and the Tab active
     *
     * @param   aButton     Name of the pressed button
     */
    private void buttonPanelAction(String aButton) {
        logger.debug("Button pressed: "+aButton+ "  ActiveTab: " + itsTabFocus);
        int treeID=-1;
        int aRow=-1;
        
        if (itsTabFocus.equals("PIC")) {
            if (aButton.equals("Query Panel")) {
            } else if (aButton.equals("New")) {
                File aFile=getFile("PIC-tree");
            } else if (aButton.equals("Delete")) {
            } else if (aButton.equals("Duplicate")) {
            } else if (aButton.equals("View")) {
                aRow = PICPanel.getSelectedRow();
                if ( aRow > -1) {
                    treeID = ((Integer)PICPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                    if (treeID>-1) {
                        if (viewInfo(treeID)) {
                            logger.debug("Tree has been changed, reloading table");
                        }
                    } else {
                        logger.debug("Tree not found");
                    }
                 } else {
                    JOptionPane.showMessageDialog(null,"You didn't select a tree",
                            "Tree selection warning",
                            JOptionPane.WARNING_MESSAGE);
                }
            }
        } else if (itsTabFocus.equals("VIC")) {
            if (aButton.equals("Query Panel")) {
            } else if (aButton.equals("Delete")) {
            } else if (aButton.equals("Duplicate")) {
            } else if (aButton.equals("View")) {
                aRow = VICPanel.getSelectedRow();
                if ( aRow > -1) {
                    treeID = ((Integer)VICPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                    if (treeID>-1) {
                        if (viewInfo(treeID) ) {
                            logger.debug("Tree has been changed, reloading table");
                        }
                    } else {
                        logger.debug("Tree not found");
                    }
                 } else {
                    JOptionPane.showMessageDialog(null,"You didn't select a tree",
                            "Tree selection warning",
                            JOptionPane.WARNING_MESSAGE);
                }
            }
        } else if (itsTabFocus.equals("Templates")) {
            if (aButton.equals("Query Panel")) {
            } else if (aButton.equals("New")) {
                itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.TemplateConstructionPanel", false, true);
                itsMainFrame.showPanel(TemplateConstructionPanel.getFriendlyNameStatic());
            } else if (aButton.equals("Duplicate")) {
                
                // TODO look if a template was chosen to duplicate first
                // and set it in the panel
                
                itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.TemplateConstructionPanel", false, true);
                itsMainFrame.showPanel(TemplateConstructionPanel.getFriendlyNameStatic());
            } else if (aButton.equals("Modify")) {

                // TODO look if a template was chosen to modify first
                // and set it in the panel
                
                itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.TemplateConstructionPanel", false, true);
                itsMainFrame.showPanel(TemplateConstructionPanel.getFriendlyNameStatic());
            } else if (aButton.equals("View")) {
                aRow = TemplatesPanel.getSelectedRow();
                if ( aRow > -1 ) {
                    treeID = ((Integer)TemplatesPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                    if (treeID>-1) {
                        if (viewInfo(treeID)) {
                            logger.debug("Tree has been changed, reloading table");
                        }
                    } else {
                        logger.debug("Tree not found in database");
                    }
                } else {
                    JOptionPane.showMessageDialog(null,"You didn't select a tree",
                            "Tree selection warning",
                            JOptionPane.WARNING_MESSAGE);
                }
            }
        } else if (itsTabFocus.equals("Components")) {
            if (aButton.equals("Query Panel")) {
            } else if (aButton.equals("New")) {
                File aFile=getFile("VIC-component");
            } else if (aButton.equals("Modify")) {
                itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ComponentMaintenancePanel", false, true);
                itsMainFrame.showPanel(ComponentMaintenancePanel.getFriendlyNameStatic());
            }
        } else if (itsTabFocus.equals("Query Results")) {
        
        } else {
            logger.debug("Other command found: "+aButton);
        }
    }
    
    /** Launch TreeInfoDialog, and if changes made save them to database
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

                
                // Check if something chnaged, if so save the tree
                if (treeInfoDialog.isChanged() ) {
                    logger.debug("Tree changed so saving to DB");
                    if (!itsMainFrame.getOTDBrmi().getRemoteMaintenance().setClassification(aSelectedTree.treeID(),treeInfoDialog.getTree().classification)) {
                        logger.debug("failed to save new classification");
                    }
                    if (!itsMainFrame.getOTDBrmi().getRemoteMaintenance().setTreeState(aSelectedTree.treeID(),treeInfoDialog.getTree().state)) {
                        logger.debug("failed to save new treeState");
                    }
//                  if( !itsMainFrame.getOTDBrmi().getRemoteMaintenance().setDescription(aSelectedTree.treeID(),treeInfoDialog.getTree().description)) {
//                        logger.debug("failed to save new description");
//                    }
                }
                
            } else {
                logger.debug("no tree found for this ID");
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
   
    
    private MainFrame      itsMainFrame;
    private String         itsTabFocus="PIC";
    private boolean        buttonsInitialized=false;
    private LoadFileDialog loadFileDialog;
    private TreeInfoDialog treeInfoDialog;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.TablePanel ComponentsPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel PICPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel QueryResultsPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel TemplatesPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel VICPanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JTabbedPane jTabbedPane1;
    // End of variables declaration//GEN-END:variables
    
}
