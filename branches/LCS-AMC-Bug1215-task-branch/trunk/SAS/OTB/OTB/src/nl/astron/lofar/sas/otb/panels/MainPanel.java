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
            buttonPanel1.addButton("Refresh");
            buttonPanel1.setButtonEnabled("Delete",false);
            buttonPanel1.addButton("View");
            buttonPanel1.setButtonEnabled("View",false);
            buttonPanel1.addButton("Info");
            buttonPanel1.setButtonEnabled("Info",false);
        } else if (itsTabFocus.equals("VIC")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("Delete");
            buttonPanel1.addButton("Refresh");
            buttonPanel1.setButtonEnabled("Delete",false);
            buttonPanel1.addButton("View");
            buttonPanel1.setButtonEnabled("View",false);
            buttonPanel1.addButton("Schedule");
            buttonPanel1.setButtonEnabled("Schedule",false);
        } else if (itsTabFocus.equals("Templates")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("Duplicate");
            buttonPanel1.setButtonEnabled("Duplicate",false);
            buttonPanel1.addButton("Modify");
            buttonPanel1.addButton("Delete");            
            buttonPanel1.addButton("Refresh");
            buttonPanel1.addButton("Build VIC tree");
            buttonPanel1.setButtonEnabled("Modify",false);
            buttonPanel1.setButtonEnabled("Delete",false);
            buttonPanel1.setButtonEnabled("Build VIC tree",false);
            buttonPanel1.addButton("Change Status");
            buttonPanel1.setButtonEnabled("Change Status",false);            
        } else if (itsTabFocus.equals("Components")) {
            buttonPanel1.addButton("Query Panel");
            buttonPanel1.addButton("New");
            buttonPanel1.addButton("Modify");
            buttonPanel1.addButton("Delete");
            buttonPanel1.addButton("Refresh");
            buttonPanel1.addButton("Build TemplateTree");
            buttonPanel1.setButtonEnabled("Delete",false);
            buttonPanel1.setButtonEnabled("Modify",false);
            buttonPanel1.setButtonEnabled("Build TemplateTree",false);
        } else if (itsTabFocus.equals("Query Results")) {
        
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
        PICtableModel PICmodel = new PICtableModel(SharedVars.getOTDBrmi());
        PICPanel.setTableModel(PICmodel);
        PICPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        PICPanel.setColumnSize("ID",35);
        PICPanel.setColumnSize("Description",700);
        PICPanel.setAutoCreateRowSorter(true);
        

        VICtableModel VICmodel = new VICtableModel(SharedVars.getOTDBrmi());
        VICPanel.setTableModel(VICmodel);
        VICPanel.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        VICPanel.setColumnSize("ID",35);
        VICPanel.setColumnSize("OriginalTree",75);
        VICPanel.setColumnSize("MoMID",50);
        VICPanel.setColumnSize("StartTime",175);
        VICPanel.setColumnSize("StopTime",175);
        VICPanel.setColumnSize("Description",700);
        VICPanel.setAutoCreateRowSorter(true);
        
        TemplatetableModel Templatemodel = new TemplatetableModel(SharedVars.getOTDBrmi());
        TemplatesPanel.setTableModel(Templatemodel);
        TemplatesPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        TemplatesPanel.setColumnSize("ID",35);
        TemplatesPanel.setColumnSize("OriginalTree",75);
        TemplatesPanel.setColumnSize("MoMID",50);
        TemplatesPanel.setColumnSize("Description",700);
        TemplatesPanel.setAutoCreateRowSorter(true);
        
        ComponentTableModel Componentmodel = new ComponentTableModel(SharedVars.getOTDBrmi());
        ComponentsPanel.setTableModel(Componentmodel);
        ComponentsPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        ComponentsPanel.setColumnSize("ID",35);
        ComponentsPanel.setColumnSize("Description",700);
        ComponentsPanel.setAutoCreateRowSorter(true);
        
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
            // keep selected tree
            int aSavedID=itsMainFrame.getSharedVars().getTreeID();
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
            if (aSavedID > 0) {
                itsMainFrame.getSharedVars().setTreeID(aSavedID);
                this.setSelectedID(aSavedID);
            }   
            this.setChanged(false);
            this.validate();
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
    
    /** Returns the selected row in the present tree */
    private int getSelectedRow() {
        int aRow=-1;
        if (itsTabFocus.equals("PIC")) {
            aRow = PICPanel.getSelectedRow();
        } else if (itsTabFocus.equals("VIC")) {
            aRow = VICPanel.getSelectedRow();
        } else if (itsTabFocus.equals("Templates")) {
            aRow = TemplatesPanel.getSelectedRow();
        } else if (itsTabFocus.equals("Components")) {
            aRow = ComponentsPanel.getSelectedRow();
        }
        return aRow;
    }

    /** Returns the selected rows in the present tree */
    private int[] getSelectedRows() {
        int [] rows=null;
        if (itsTabFocus.equals("PIC")) {
            rows = PICPanel.getSelectedRows();
        } else if (itsTabFocus.equals("VIC")) {
            rows = VICPanel.getSelectedRows();
        } else if (itsTabFocus.equals("Templates")) {
            rows = TemplatesPanel.getSelectedRows();
        } else if (itsTabFocus.equals("Components")) {
            rows = ComponentsPanel.getSelectedRows();
        }
        return rows;
    }

    /** Sets the selected row in the present tree */
    private void setSelectedID(int aTreeID) {
        if (aTreeID > -1) {
            if (itsTabFocus.equals("PIC")) {
                PICPanel.setSelectedID(aTreeID);
            } else if (itsTabFocus.equals("VIC")) {
                VICPanel.setSelectedID(aTreeID);
            } else if (itsTabFocus.equals("Templates")) {
                TemplatesPanel.setSelectedID(aTreeID);
            } else if (itsTabFocus.equals("Components")) {
                ComponentsPanel.setSelectedID(aTreeID);
            }
        }
    }
    
    /** Returns the ids of the selected Trees in case of a multiple selection */
    private int [] getSelectedTreeIDs() {
        int [] rows=this.getSelectedRows();
        int [] treeIDs=new int[rows.length];
        if (itsTabFocus.equals("PIC")) {
            for (int i=0; i < rows.length; i++) {
                treeIDs[i] = ((Integer)PICPanel.getTableModel().getValueAt(rows[i], 0)).intValue();
            }
        } else if (itsTabFocus.equals("VIC")) {
            for (int i=0; i < rows.length; i++) {
                treeIDs[i] = ((Integer)VICPanel.getTableModel().getValueAt(rows[i], 0)).intValue();
            }
        } else if (itsTabFocus.equals("Templates")) {
            for (int i=0; i < rows.length; i++) {
                treeIDs[i] = ((Integer)TemplatesPanel.getTableModel().getValueAt(rows[i], 0)).intValue();
            }
        } else if (itsTabFocus.equals("Components")) {
            for (int i=0; i < rows.length; i++) {
                // is the node ID in the case of Components
                treeIDs[i] = ((Integer)ComponentsPanel.getTableModel().getValueAt(rows[i], 0)).intValue();
            }
        }

        return treeIDs;   
    }

    /** Returns the id of the selected Tree */
    private int getSelectedTreeID() {
        int treeID=0;
        int aRow=this.getSelectedRow();
        if (itsTabFocus.equals("PIC")) {
            if ( aRow > -1) {
                treeID = ((Integer)PICPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                if (treeID > 0) {
                    itsMainFrame.getSharedVars().setTreeID(treeID);
                } else {
                    logger.debug("Tree not found");
                }
            }
        } else if (itsTabFocus.equals("VIC")) {
            if ( aRow > -1) {
                treeID = ((Integer)VICPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                if (treeID > 0) {
                    itsMainFrame.getSharedVars().setTreeID(treeID);
                } else {
                    logger.debug("Tree not found");
                }
            }
        } else if (itsTabFocus.equals("Templates")) {
            if ( aRow > -1) {
                treeID = ((Integer)TemplatesPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                if (treeID > 0) {
                    itsMainFrame.getSharedVars().setTreeID(treeID);
                } else {
                    logger.debug("Tree not found");
                }
            }
        } else if (itsTabFocus.equals("Components")) {
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
        } else if (aButton.equals("Refresh")) {
            //set changed flag, we want to refresh the tree
            itsMainFrame.setChanged(this.getFriendlyName(),true);
            checkChanged();
            return;
        }
        if (itsTabFocus.equals("PIC")) {
            if (treeID > 0) {
                itsMainFrame.getSharedVars().setTreeID(treeID);
            } else if (!aButton.equals("New")) {
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
                       String aFileName= "/tmp/"+itsMainFrame.getUserAccount().getUserName()+"_"+itsNewFile.getName();
                       if (OtdbRmi.getRemoteFileTrans().uploadFile(uldata,aFileName)) {
                           logger.debug("upload finished");                       
                           // Create a new Tree from the found file.
                           int aTreeID=OtdbRmi.getRemoteMaintenance().loadMasterFile(aFileName);
                           if (aTreeID < 1) {
                               logger.debug("Error on fileLoad: " + aFileName);
                           } else {
                               // set changed flag to reload mainpanel
                               itsMainFrame.setChanged(this.getFriendlyName(),true);
                               // set the new created TreeID to active and fill description stuff if needed
                               itsMainFrame.getSharedVars().setTreeID(aTreeID);
                               checkChanged();
                               if (!itsFileDescription.equals("")) {
                                  if (!OtdbRmi.getRemoteMaintenance().setDescription(aTreeID,itsFileDescription)) {
                                     logger.debug("Error during setDescription in Tree "+OtdbRmi.getRemoteMaintenance().errorMsg());
                                  }
                               }
                                  
                               if (!itsFileStatus.equals("")) {
                                  if (!OtdbRmi.getRemoteMaintenance().setTreeState(aTreeID,OtdbRmi.getRemoteTypes().getTreeState(itsFileStatus))) {
                                     logger.debug("Error during setStatus in Tree "+OtdbRmi.getRemoteMaintenance().errorMsg());
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
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree: ?"+treeID,"Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        if (OtdbRmi.getRemoteMaintenance().deleteTree(treeID)) {
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
                if (treeID > 0) {
                    int [] id=new int[1];
                    id[0]=treeID;
                    if (viewInfo(id)) {
                        
                        logger.debug("Tree has been changed, reloading tableline");
                          itsMainFrame.setChanged(this.getFriendlyName(),true);
                          checkChanged();
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
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree: ?"+treeID,"Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        if (OtdbRmi.getRemoteMaintenance().deleteTree(treeID)) {
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
            } else if (aButton.equals("Schedule")) {
                
                // in case of VICtree we have the possibility of changing a multiple selection
                // so things like start and/or stoptimes can be set for a few entries at once
                
                if (this.VICPanel.getSelectedRowCount() > 0) {
                    if (viewInfo(this.getSelectedTreeIDs()) ) {
                        logger.debug("Tree has been changed, reloading tableline");
                          itsMainFrame.setChanged(this.getFriendlyName(),true);
                          checkChanged();
                    }
                    
                }
            }
        } else if (itsTabFocus.equals("Templates")) {
            jOTDBtree aTree=null;
            String aTreeState="";
        
            if (treeID > 0) {
                itsMainFrame.getSharedVars().setTreeID(treeID);
                try {
                    aTree =    OtdbRmi.getRemoteOTDB().getTreeInfo(treeID,false);
                    aTreeState=OtdbRmi.getRemoteTypes().getTreeState(aTree.state);
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
                if (treeID < 1) {
                    JOptionPane.showMessageDialog(null,"Select a tree to duplicate first",
                        "No Tree Selected",
                        JOptionPane.WARNING_MESSAGE);
                } else {
                    try {
                        int newTreeID=OtdbRmi.getRemoteMaintenance().copyTemplateTree(treeID);
                        if (newTreeID > 0) {
                            JOptionPane.showMessageDialog(null,"New Tree created with ID: "+newTreeID,
                                "New Tree Message",
                                JOptionPane.INFORMATION_MESSAGE);
                            // set back treestate to being specified
                            jOTDBtree aT=OtdbRmi.getRemoteOTDB().getTreeInfo(newTreeID, false); 
                            String aState=OtdbRmi.getTreeState().get(aT.state);
                            if (aT.state != OtdbRmi.getRemoteTypes().getTreeState("being specified") ) {
                                aT.state=OtdbRmi.getRemoteTypes().getTreeState("being specified");
                                if (!OtdbRmi.getRemoteMaintenance().setTreeState(aT.treeID(), aT.state)) {
                                    logger.debug("Error during setTreeState: "+OtdbRmi.getRemoteMaintenance().errorMsg());                      
                                }
                            }
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
               if (treeID < 1) {
                 JOptionPane.showMessageDialog(null,"Select a tree to delete first",
                     "No Tree Selected",
                     JOptionPane.WARNING_MESSAGE);
                } else {
                 if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree: ?"+treeID,"Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                      try {
                          if (OtdbRmi.getRemoteMaintenance().deleteTree(treeID)) {
                            JOptionPane.showMessageDialog(null,"Template Tree Deleted",
                                "Delete Tree Message",
                                JOptionPane.INFORMATION_MESSAGE);
                            itsMainFrame.getSharedVars().setTreeID(-1);                              
                            itsMainFrame.setHourglassCursor();
                            ((VICtableModel)VICPanel.getTableModel()).fillTable();
                            itsMainFrame.setNormalCursor();
                            // set changed flag to reload mainpanel
                            itsMainFrame.setChanged(this.getFriendlyName(),true);
                            checkChanged();
                          } else {
                            logger.debug("Failed to delete tree");
                          }
                      } catch (RemoteException ex) {
                          logger.debug("Remote error during deleteTree: "+ ex);
                      }
                  }
                }
                
            } else if (aButton.equals("Build VIC tree")) {
                if (treeID < 1) {
                    JOptionPane.showMessageDialog(null,"Select a tree to build first",
                        "No Tree Selected",
                        JOptionPane.WARNING_MESSAGE);
                } else {
                    try {
                        int newTreeID=OtdbRmi.getRemoteMaintenance().instanciateTree(itsMainFrame.getSharedVars().getTreeID());
                        if (newTreeID > 0) {
                            JOptionPane.showMessageDialog(null,"New VICTree created with ID: "+newTreeID,
                                "New Tree Message",
                                JOptionPane.INFORMATION_MESSAGE);
                            itsMainFrame.getSharedVars().setTreeID(newTreeID);
                            // set changed flag to reload mainpanel
                            itsMainFrame.setChanged(this.getFriendlyName(),true);
                            checkChanged();
                        } else {
                            logger.debug("No VIC Tree created!!! : "+ OtdbRmi.getRemoteMaintenance().errorMsg());
                        }
           
                    } catch (RemoteException ex) {
                        logger.debug("Remote error during Build VICTree: "+ ex);
                    }
                }
                
            } else if (aButton.equals("Change Status")) {
                if (itsMainFrame.getSharedVars().getTreeID() > 0) {
                    int [] id = new int[1];
                    id[0]=itsMainFrame.getSharedVars().getTreeID();
                    if (viewInfo(id)) {
                        logger.debug("Tree has been changed, reloading table line");
                          itsMainFrame.setChanged(this.getFriendlyName(),true);
                          checkChanged();
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
                        String aFileName= "/tmp/"+itsMainFrame.getUserAccount().getUserName()+"_"+itsNewFile.getName();
                        if (OtdbRmi.getRemoteFileTrans().uploadFile(uldata,aFileName)) {
                            logger.debug("upload finished");
                            // Create a new Tree from the found file.
                            int anID=OtdbRmi.getRemoteMaintenance().loadComponentFile(aFileName);
                            if (anID < 1) {
                                logger.debug("Error on ComponentfileLoad: " + itsNewFile.getPath());
                            } else {
                                // set the new created fill description stuff if needed
                                itsMainFrame.getSharedVars().setComponentID(anID);
                                if (!itsFileDescription.equals("")) {
                                    jVICnodeDef aND=OtdbRmi.getRemoteMaintenance().getComponentNode(anID);
                                    aND.description=itsFileDescription;
                                    if (!OtdbRmi.getRemoteMaintenance().saveComponentNode(aND)) {
                                        logger.debug("Error during setDescription in Component "+OtdbRmi.getRemoteMaintenance().errorMsg());
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
                    classifID = OtdbRmi.getRemoteTypes().getClassif("operational");
                    if (OtdbRmi.getRemoteMaintenance().isTopComponent(nodeID)) {
                        int newTreeID=OtdbRmi.getRemoteMaintenance().buildTemplateTree(nodeID,classifID);
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
                    if (OtdbRmi.getRemoteMaintenance().deleteComponentNode(nodeID)) {
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
     * @param  treeIDs  The IDs of the chosen trees.
     */
    private boolean viewInfo(int[] treeIDs) {
        logger.debug("viewInfo for treeID: " + treeIDs);
        //get the selected tree from the database
        boolean multiple=false;

        
        if (treeIDs.length > 0) {
            // show treeInfo dialog
            if (treeInfoDialog == null ) {
                treeInfoDialog = new TreeInfoDialog(true,treeIDs, itsMainFrame);
            } else {
                treeInfoDialog.setTree(treeIDs);
            }
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
        if (loadFileDialog == null ) {
            loadFileDialog = new LoadFileDialog(itsMainFrame,true,aType);
        } else {
            loadFileDialog.setType(aType);
        }
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
                aTree      = OtdbRmi.getRemoteOTDB().getTreeInfo(treeID,false);
                aTreeState = OtdbRmi.getRemoteTypes().getTreeState(aTree.state);
                aClassif   = OtdbRmi.getRemoteTypes().getClassif(aTree.classification);
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
            boolean infoOnly=false;
            if (VICPanel.getSelectedRowCount() > 1) {
                infoOnly=true;
            }
            if (treeID>0) {
                if ((aTreeState.equals("idle") || aTreeState.endsWith("being specified") || aTreeState.equals("finished")
                || aTreeState.equals("aborted") || aTreeState.equals("obsolete")) && !infoOnly) {
                    buttonPanel1.setButtonEnabled("Delete",true);
                } else {
                    buttonPanel1.setButtonEnabled("Delete",false);                    
                }
                if (!infoOnly) {
                    buttonPanel1.setButtonEnabled("View",true);
                    buttonPanel1.setButtonEnabled("Query Panel",true);
                    buttonPanel1.setButtonEnabled("Refresh",true);
                } else {
                    buttonPanel1.setButtonEnabled("Query Panel",false);
                    buttonPanel1.setButtonEnabled("Refresh",false);
                    buttonPanel1.setButtonEnabled("View",false);
                }
                buttonPanel1.setButtonEnabled("Schedule",true);
            } else {
                buttonPanel1.setButtonEnabled("Delete",false);
                buttonPanel1.setButtonEnabled("View",false);
                buttonPanel1.setButtonEnabled("Schedule",false);
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
                        buttonPanel1.setButtonEnabled("Build VIC tree",true);
                    } else {
                        buttonPanel1.setButtonEnabled("Build VIC tree",false);
                    }
                } else {
                    buttonPanel1.setButtonEnabled("Duplicate",false);
                    buttonPanel1.setButtonEnabled("Modify",false);                                        
                }
                buttonPanel1.setButtonEnabled("Change Status",true);
                buttonPanel1.setButtonEnabled("Delete",true);                
            } else {
                buttonPanel1.setButtonEnabled("Duplicate",false);
                buttonPanel1.setButtonEnabled("Modify",false);
                buttonPanel1.setButtonEnabled("Delete",false);                
                buttonPanel1.setButtonEnabled("Change Status",false);  
                buttonPanel1.setButtonEnabled("Build VIC tree",false);
            }
        } else if (itsTabFocus.equals("Components")) {
            if (componentID > 0 ) {
                buttonPanel1.setButtonEnabled("Modify",true);
                buttonPanel1.setButtonEnabled("Delete",true);
                try {
                    if (OtdbRmi.getRemoteMaintenance().isTopComponent(componentID)) {
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
    
    private MainFrame      itsMainFrame = null;
    private String         itsTabFocus="PIC";
    private boolean        buttonsInitialized=false;
    private LoadFileDialog loadFileDialog = null;
    private TreeInfoDialog treeInfoDialog = null;
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
