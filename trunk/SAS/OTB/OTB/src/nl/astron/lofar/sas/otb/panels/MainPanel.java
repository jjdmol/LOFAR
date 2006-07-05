/*
 * MainPanel.java
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
 */

package nl.astron.lofar.sas.otb.panels;
import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.rmi.RemoteException;

import javax.swing.JOptionPane;
import javax.swing.ListSelectionModel;
import nl.astron.lofar.sas.otb.*;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import nl.astron.lofar.sas.otb.util.*;
import nl.astron.lofar.sas.otb.util.tablemodels.ComponentTableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.PICtableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.TemplatetableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.VICtableModel;
import nl.astron.lofar.sas.otbcomponents.LoadFileDialog;
import nl.astron.lofar.sas.otbcomponents.TreeInfoDialog;
import org.apache.log4j.Logger;

/**
 * This will be the Main entry panel for SAS. It will display all the available
 * trees, component lists and templates that are available for LOFAR, from here
 * the relevant action screens can be loaded to interact with LOFAR.
 *
 * @created 13-01-2006, 14:58
 * @author  Blaakmeer/Coolen
 * @version $Id$
 * @updated deleteComponentNode added to component panel
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
            buttonPanel1.addButton("View");
            buttonPanel1.setButtonEnabled("View",false);
        } else if (itsTabFocus.equals("Templates")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("Duplicate");
            buttonPanel1.setButtonEnabled("Duplicate",false);
            buttonPanel1.addButton("Modify");
            buttonPanel1.addButton("Delete");            
            buttonPanel1.addButton("Instanciate");
            buttonPanel1.setButtonEnabled("Modify",false);
            buttonPanel1.setButtonEnabled("Delete",false);
            buttonPanel1.setButtonEnabled("Instanciate",false);
        } else if (itsTabFocus.equals("Components")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("New");
            buttonPanel1.addButton("Modify");
            buttonPanel1.addButton("Delete");
            buttonPanel1.addButton("Build TemplateTree");
            buttonPanel1.setButtonEnabled("Delete",false);
            buttonPanel1.setButtonEnabled("Modify",false);
            buttonPanel1.setButtonEnabled("Build TemplateTree",false);
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
        PICtableModel PICmodel = new PICtableModel(itsMainFrame.getSharedVars().getOTDBrmi());
        PICPanel.setTableModel(PICmodel);
        PICPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);


        VICtableModel VICmodel = new VICtableModel(itsMainFrame.getSharedVars().getOTDBrmi());
        VICPanel.setTableModel(VICmodel);
        VICPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        
        TemplatetableModel Templatemodel = new TemplatetableModel(itsMainFrame.getSharedVars().getOTDBrmi());
        TemplatesPanel.setTableModel(Templatemodel);
        TemplatesPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        
        ComponentTableModel Componentmodel = new ComponentTableModel(itsMainFrame.getSharedVars().getOTDBrmi());
        ComponentsPanel.setTableModel(Componentmodel);
        ComponentsPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        
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
        logger.debug("Check Changed status");
        if (this.hasChanged()) {
            itsMainFrame.setHourglassCursor();
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
                    logger.debug("error filling templateTable");
                }
                // VICTree could have been changed also
                if (!((VICtableModel)VICPanel.getTableModel()).fillTable()) {
                    logger.debug("error filling VICTable");
                }
            } else if (itsTabFocus.equals("Components")) {
                if (!((ComponentTableModel)ComponentsPanel.getTableModel()).fillTable()) {
                    logger.debug("error filling ComponentsTable");
                }
                // templateTree could have been changed also
                if (!((TemplatetableModel)TemplatesPanel.getTableModel()).fillTable()) {
                    logger.debug("error filling templateTable");
                }
            }   
            this.setChanged(false);
            this.validateButtons();
            itsMainFrame.setNormalCursor();
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
                    itsMainFrame.getSharedVars().setTreeID(treeID);
                } else {
                    logger.debug("Tree not found");
                }
            }
        } else if (itsTabFocus.equals("VIC")) {
            aRow = VICPanel.getSelectedRow();
            if ( aRow > -1) {
                treeID = ((Integer)VICPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                if (treeID > 0) {
                    itsMainFrame.getSharedVars().setTreeID(treeID);
                } else {
                    logger.debug("Tree not found");
                }
            }
        } else if (itsTabFocus.equals("Templates")) {
            aRow = TemplatesPanel.getSelectedRow();
            if ( aRow > -1) {
                treeID = ((Integer)TemplatesPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                if (treeID > 0) {
                    itsMainFrame.getSharedVars().setTreeID(treeID);
                } else {
                    logger.debug("Tree not found");
                }
            }
        } else if (itsTabFocus.equals("Components")) {
            aRow = ComponentsPanel.getSelectedRow();
            if ( aRow > -1) {
                // is the node ID in the case of Components
                treeID = ((Integer)ComponentsPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                if (treeID > 0) {
                    itsMainFrame.getSharedVars().setComponentID(treeID);
                } else {
                    logger.debug("Component not found");
                }
            }        }
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
            return;
        }
        if (itsTabFocus.equals("PIC")) {
            if (treeID > 0) {
                itsMainFrame.getSharedVars().setTreeID(treeID);
            } else {
                JOptionPane.showMessageDialog(null,"You didn't select a tree",
                        "Tree selection warning",
                        JOptionPane.WARNING_MESSAGE);
                return;
            }
            if (aButton.equals("Query Panel")) {
                // TODO open Query Panel
                itsMainFrame.ToDo();
            } else if (aButton.equals("New")) {
                if (getFile("PIC-tree")) {
                    try {
                       // the file obviously resides at the client side, and needs to be transfered to the server side.
                       byte uldata[] = new byte[(int)itsNewFile.length()]; 
                       BufferedInputStream input = new BufferedInputStream(new FileInputStream(itsNewFile));   
                       input.read(uldata,0,uldata.length);
                       input.close();
                       if (itsMainFrame.getSharedVars().getOTDBrmi().getRemoteFileTrans().uploadFile(uldata,itsNewFile.getName())) {
                           logger.debug("upload finished");                       
                           // Create a new Tree from the found file.
                           int aTreeID=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().loadMasterFile(itsNewFile.getPath());
                           if (aTreeID < 1) {
                               logger.debug("Error on fileLoad: " + itsNewFile.getPath());
                           } else {
                               // set changed flag to reload mainpanel
                               itsMainFrame.setChanged(this.getFriendlyName(),true);
                               // set the new created TreeID to active and fill description stuff if needed
                               itsMainFrame.getSharedVars().setTreeID(aTreeID);
                               checkChanged();
                               if (!itsFileDescription.equals("")) {
                                  if (!itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().setDescription(aTreeID,itsFileDescription)) {
                                     logger.debug("Error during setDescription in Tree "+itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().errorMsg());
                                  }
                               }
                                  
                               if (!itsFileStatus.equals("")) {
                                  if (!itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().setTreeState(aTreeID,itsMainFrame.getSharedVars().getOTDBrmi().getRemoteTypes().getTreeState(itsFileStatus))) {
                                     logger.debug("Error during setStatus in Tree "+itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().errorMsg());
                                  }
                               }                               
                           }
                           ResultBrowserPanel aP=(ResultBrowserPanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ResultBrowserPanel", false, true);
                           if (aP != null) {
                              itsMainFrame.showPanel(aP.getFriendlyName());
                           }
                        }
                   } catch (RemoteException ex) {
                       logger.debug("Error during newPICTree creation: "+ ex);
                   } catch (FileNotFoundException ex) {
                       logger.debug("Error during newPICTree creation: "+ ex);
                   } catch (IOException ex) {
                       logger.debug("Error during newPICTree creation: "+ ex);
                   }
                }  

            } else if (aButton.equals("Delete")) {
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree ?","Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        if (itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().deleteTree(treeID)) {
                            itsMainFrame.setHourglassCursor();
                            ((PICtableModel)PICPanel.getTableModel()).fillTable();
                            itsMainFrame.setNormalCursor();
                        } else {
                            logger.debug("Failed to delete tree");
                        }
                    } catch (RemoteException ex) {
                        logger.debug("Remote error during deleteTree: "+ ex);
                    }
                }
            } else if (aButton.equals("View")) {
                ResultBrowserPanel aP=(ResultBrowserPanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ResultBrowserPanel", false, true);
                if (aP != null) {
                    itsMainFrame.showPanel(aP.getFriendlyName());
                }
            } else if (aButton.equals("Info")) {
                if (itsMainFrame.getSharedVars().getTreeID() > 0) {
                    if (viewInfo(itsMainFrame.getSharedVars().getTreeID())) {
                        logger.debug("Tree has been changed, reloading tableline");
                        ((PICtableModel)PICPanel.getTableModel()).refreshRow(PICPanel.getSelectedRow());
                    }
                }
            }
        } else if (itsTabFocus.equals("VIC")) {
            if (treeID > 0) {
                itsMainFrame.getSharedVars().setTreeID(treeID);
            } else {
                JOptionPane.showMessageDialog(null,"You didn't select a tree",
                        "Tree selection warning",
                        JOptionPane.WARNING_MESSAGE);
                return;
            }
            if (aButton.equals("Query Panel")) {
                // TODO open Query Panel
                itsMainFrame.ToDo();
            } else if (aButton.equals("Delete")) {
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree ?","Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        if (itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().deleteTree(treeID)) {
                            itsMainFrame.setHourglassCursor();
                            ((VICtableModel)VICPanel.getTableModel()).fillTable();
                            itsMainFrame.setNormalCursor();
                        } else {
                            logger.debug("Failed to delete tree");
                        }
                    } catch (RemoteException ex) {
                        logger.debug("Remote error during deleteTree: "+ ex);
                    }
                }
            } else if (aButton.equals("View")) {
                ResultBrowserPanel aP=(ResultBrowserPanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ResultBrowserPanel", false, true);
                if (aP != null) {
                    itsMainFrame.showPanel(aP.getFriendlyName());
                }
            } else if (aButton.equals("Info")) {
                if (itsMainFrame.getSharedVars().getTreeID() > 0) {
                    if (viewInfo(itsMainFrame.getSharedVars().getTreeID()) ) {
                        logger.debug("Tree has been changed, reloading tableline");
                        ((VICtableModel)VICPanel.getTableModel()).refreshRow(VICPanel.getSelectedRow());
                    }
                }
            }
        } else if (itsTabFocus.equals("Templates")) {
            jOTDBtree aTree=null;
            String aTreeState="";
        
            if (treeID > 0) {
                itsMainFrame.getSharedVars().setTreeID(treeID);
                try {
                    aTree =    itsMainFrame.getSharedVars().getOTDBrmi().getRemoteOTDB().getTreeInfo(treeID,false);
                    aTreeState=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteTypes().getTreeState(aTree.state);
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
                itsMainFrame.ToDo();
            } else if (aButton.equals("Duplicate")) {
                if (itsMainFrame.getSharedVars().getTreeID() < 1) {
                    JOptionPane.showMessageDialog(null,"Select a tree to duplicate first",
                        "No Tree Selected",
                        JOptionPane.WARNING_MESSAGE);
                } else {
                    try {
                        int newTreeID=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().copyTemplateTree(itsMainFrame.getSharedVars().getTreeID());
                        if (newTreeID > 0) {
                            JOptionPane.showMessageDialog(null,"New Tree created with ID: "+newTreeID,
                                "New Tree Message",
                                JOptionPane.INFORMATION_MESSAGE);
                            itsMainFrame.getSharedVars().setTreeID(newTreeID);
                            // set changed flag to reload mainpanel
                            itsMainFrame.setChanged(this.getFriendlyName(),true);
                            checkChanged();
                        } else {
                            logger.debug("No Template Tree created!!!");
                        }
           
                    } catch (RemoteException ex) {
                        logger.debug("Remote error during Build TemplateTree: "+ ex);
                    }
                }
            } else if (aButton.equals("Modify")) {
                TemplateMaintenancePanel aP =(TemplateMaintenancePanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.TemplateMaintenancePanel", false, true);
                if (aP != null) {
                    itsMainFrame.showPanel(aP.getFriendlyName());
                }
            } else if (aButton.equals("Delete")) {
                if (itsMainFrame.getSharedVars().getTreeID() < 1) {
                    JOptionPane.showMessageDialog(null,"Select a tree to delete first",
                        "No Tree Selected",
                        JOptionPane.WARNING_MESSAGE);
                } else {
                    try {
                        if (itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().deleteTree(itsMainFrame.getSharedVars().getTreeID())) {
                            JOptionPane.showMessageDialog(null,"Template Tree Deleted",
                                "Delete Tree Message",
                                JOptionPane.INFORMATION_MESSAGE);
                            itsMainFrame.getSharedVars().setTreeID(-1);
                            // set changed flag to reload mainpanel
                            itsMainFrame.setChanged(this.getFriendlyName(),true);
                            checkChanged();
                        } else {
                            logger.debug("Template Tree not deleted!!! : "+ itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().errorMsg());
                        }
           
                    } catch (RemoteException ex) {
                        logger.debug("Remote error during Delete Template Tree: "+ ex);
                    }
                }
                
            } else if (aButton.equals("Instanciate")) {
                if (itsMainFrame.getSharedVars().getTreeID() < 1) {
                    JOptionPane.showMessageDialog(null,"Select a tree to instanciate first",
                        "No Tree Selected",
                        JOptionPane.WARNING_MESSAGE);
                } else {
                    try {
                        int newTreeID=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().instanciateTree(itsMainFrame.getSharedVars().getTreeID());
                        if (newTreeID > 0) {
                            JOptionPane.showMessageDialog(null,"New VICTree created with ID: "+newTreeID,
                                "New Tree Message",
                                JOptionPane.INFORMATION_MESSAGE);
                            itsMainFrame.getSharedVars().setTreeID(newTreeID);
                            // set changed flag to reload mainpanel
                            itsMainFrame.setChanged(this.getFriendlyName(),true);
                            checkChanged();
                        } else {
                            logger.debug("No VIC Tree created!!! : "+ itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().errorMsg());
                        }
           
                    } catch (RemoteException ex) {
                        logger.debug("Remote error during Build VICTree: "+ ex);
                    }
                }
                
            } else if (aButton.equals("Info")) {
                if (itsMainFrame.getSharedVars().getTreeID() > 0) {
                    if (viewInfo(itsMainFrame.getSharedVars().getTreeID()) ) {
                        logger.debug("Tree has been changed, reloading table line");
                        ((TemplatetableModel)TemplatesPanel.getTableModel()).refreshRow(TemplatesPanel.getSelectedRow());
                    }
                }
            }
        } else if (itsTabFocus.equals("Components")) {
            if (aButton.equals("Query Panel")) {
                itsMainFrame.ToDo();
            } else if (aButton.equals("New")) {
                if (getFile("VIC-component") ) {
                    try {
                        // the file obviously resides at the client side, and needs to be transfered to the server side.
                        byte uldata[] = new byte[(int)itsNewFile.length()]; 
                        BufferedInputStream input = new BufferedInputStream(new FileInputStream(itsNewFile));   
                        input.read(uldata,0,uldata.length);
                        input.close();
                        if (itsMainFrame.getSharedVars().getOTDBrmi().getRemoteFileTrans().uploadFile(uldata,itsNewFile.getName())) {
                            logger.debug("upload finished");
                            // Create a new Tree from the found file.
                            int anID=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().loadComponentFile(itsNewFile.getName());
                            if (anID < 1) {
                                logger.debug("Error on ComponentfileLoad: " + itsNewFile.getPath());
                            } else {
                                // set the new created fill description stuff if needed
                                itsMainFrame.getSharedVars().setComponentID(anID);
                                if (!itsFileDescription.equals("")) {
                                    jVICnodeDef aND=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getComponentNode(anID);
                                    aND.description=itsFileDescription;
                                    if (!itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().saveComponentNode(aND)) {
                                        logger.debug("Error during setDescription in Component "+itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().errorMsg());
                                    }
                                }
                                // set changed flag to reload mainpanel
                                itsMainFrame.setChanged(this.getFriendlyName(),true);
                                checkChanged();
                            }
                            ComponentMaintenancePanel aP=(ComponentMaintenancePanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ComponentMaintenancePanel", false, true);
                            if (aP != null) {
                                itsMainFrame.showPanel(aP.getFriendlyName());
                            }
                        } else {
                            logger.debug("upload failed");
                        }
                    } catch (FileNotFoundException ex) {
                        logger.debug("Error during new Component creation: "+ ex);
                    } catch (RemoteException ex) {
                        logger.debug("Error during new Component creation: "+ ex);
                    } catch (IOException ex) {
                        logger.debug("Error during new Component creation: "+ ex);
                    }
                }
            } else if (aButton.equals("Modify")) {
                ComponentMaintenancePanel aP = (ComponentMaintenancePanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ComponentMaintenancePanel", false, true);
                if (aP != null) {
                    itsMainFrame.showPanel(aP.getFriendlyName());
                }
            } else if (aButton.equals("Build TemplateTree")) {
                int nodeID=itsMainFrame.getSharedVars().getComponentID();
                short classifID;
                try {
                    classifID = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteTypes().getClassif("operational");
                    if (itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().isTopComponent(nodeID)) {
                        int newTreeID=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().buildTemplateTree(nodeID,classifID);
                        if (newTreeID > 0) {
                            JOptionPane.showMessageDialog(null,"New Tree created with ID: "+newTreeID,
                                "New Tree Message",
                                JOptionPane.INFORMATION_MESSAGE);
                            itsMainFrame.getSharedVars().setTreeID(newTreeID);
                            // set changed flag to reload mainpanel
                            itsMainFrame.setChanged(this.getFriendlyName(),true);
                            checkChanged();
                        } else {
                            logger.debug("No Template Tree created!!!");
                        }
                    }
                } catch (RemoteException ex) {
                    logger.debug("Remote error during Build TemplateTree: "+ ex);
                }
            } else if (aButton.equals("Delete")) {
                int nodeID=itsMainFrame.getSharedVars().getComponentID();
                try {
                    if (itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().deleteComponentNode(nodeID)) {
                        // set changed flag to reload mainpanel
                       itsMainFrame.setChanged(this.getFriendlyName(),true);
                       checkChanged();
                     } else {
                        logger.debug("Component not deleted");
                     }
                } catch (RemoteException ex) {
                    logger.debug("Remote error during component deletion: "+ ex);
                }
            }
        } else if (itsTabFocus.equals("Query Results")) {
            itsMainFrame.ToDo();
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
            jOTDBtree aSelectedTree=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteOTDB().getTreeInfo(aTreeID, false);
            
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
    private boolean getFile(String aType) {
        File aNewFile=null;
        String aFileStatus="";
        String aFileDescription="";
        
        // Reset the File info fields
        itsNewFile = null;
        itsFileDescription="";
        itsFileStatus="";
        
        // show login dialog
        loadFileDialog = new LoadFileDialog(itsMainFrame,true,aType);
        loadFileDialog.setLocationRelativeTo(this);
        if (aType.equals("VIC-component")) {
            loadFileDialog.setStatusVisible(false);
        }
        loadFileDialog.setVisible(true);
        if(loadFileDialog.isOk()) {
            aFileDescription = loadFileDialog.getDescription();
            aFileStatus = loadFileDialog.getStatus();
            aNewFile = loadFileDialog.getFile();       
        } else {
            logger.info("No File chosen");
            return false;
        }
        if (aNewFile != null && aNewFile.exists()) {
            logger.debug("File to load: " + aNewFile.getName()); 
            logger.debug("Status: " + aFileStatus);
            logger.debug("Description: "+ aFileDescription);
            itsNewFile = aNewFile;
            if (aType.equals("VIC-component")) {
                itsFileStatus = "";
            } else {
                itsFileStatus = aFileStatus;
            }
            itsFileDescription = aFileDescription;
            return true;
        }
        return false;
    }
    
    private void validateButtons() {
        // depending on the tabfocus and the info the selected row contains
        // certain buttons are valid and others need to be grey out
        jOTDBtree aTree=null;
        String aTreeState="";
        String aClassif="";
        
        int treeID=getSelectedTreeID();
        int componentID=itsMainFrame.getSharedVars().getComponentID();
        logger.debug("Selected Tree: "+treeID);
        if (treeID > 0) {
            try {
                aTree =    itsMainFrame.getSharedVars().getOTDBrmi().getRemoteOTDB().getTreeInfo(treeID,false);
                aTreeState=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteTypes().getTreeState(aTree.state);
                aClassif=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteTypes().getClassif(aTree.classification);
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
                buttonPanel1.setButtonEnabled("View",true);
                buttonPanel1.setButtonEnabled("Info",true);
            } else {
                buttonPanel1.setButtonEnabled("Delete",false);
                buttonPanel1.setButtonEnabled("View",false);
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
                    if (aTreeState.equals("approved")) {
                        buttonPanel1.setButtonEnabled("Instanciate",true);
                    } else {
                        buttonPanel1.setButtonEnabled("Instanciate",false);
                    }
                } else {
                    buttonPanel1.setButtonEnabled("Duplicate",false);
                    buttonPanel1.setButtonEnabled("Modify",false);                                        
                }
                buttonPanel1.setButtonEnabled("Info",true);
                buttonPanel1.setButtonEnabled("Delete",true);                
            } else {
                buttonPanel1.setButtonEnabled("Duplicate",false);
                buttonPanel1.setButtonEnabled("Modify",false);
                buttonPanel1.setButtonEnabled("Delete",false);                
                buttonPanel1.setButtonEnabled("Info",false);  
                buttonPanel1.setButtonEnabled("Instanciate",false);
            }
        } else if (itsTabFocus.equals("Components")) {
            if (componentID > 0 ) {
                buttonPanel1.setButtonEnabled("Modify",true);
                buttonPanel1.setButtonEnabled("Delete",true);
                try {
                    if (itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().isTopComponent(componentID)) {
                        buttonPanel1.setButtonEnabled("Build TemplateTree",true);                    
                    } else {
                        buttonPanel1.setButtonEnabled("Build TemplateTree",false);                                        
                    }
                } catch (RemoteException ex) {
                    logger.debug("Error checking isTopComponent");
                }
            } else {
                buttonPanel1.setButtonEnabled("Modify",false);                
                buttonPanel1.setButtonEnabled("Delete",false);
            }
        } else if (itsTabFocus.equals("Query Results")) {
        }
    }
    
    private MainFrame      itsMainFrame;
    private String         itsTabFocus="PIC";
    private boolean        buttonsInitialized=false;
    private LoadFileDialog loadFileDialog;
    private TreeInfoDialog treeInfoDialog;
    private boolean        changed=false;
    
    // File to be loaded info
    File itsNewFile=null;
    String itsFileDescription="";
    String itsFileStatus = "";
            
    
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
