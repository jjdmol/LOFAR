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
import java.util.ArrayList;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.ListSelectionModel;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.lofarutils.inputfieldbuilder.inputFieldBuilder;
import nl.astron.lofar.sas.otb.*;
import nl.astron.lofar.sas.otb.jotdb3.jDefaultTemplate;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.jotdb3.jVICnodeDef;
import nl.astron.lofar.sas.otb.util.*;
import nl.astron.lofar.sas.otb.util.tablemodels.ComponentTableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.DefaultTemplatetableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.PICtableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.StateChangeHistoryTableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.TemplatetableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.VICtableModel;
import nl.astron.lofar.sas.otbcomponents.CreateDefaultTemplateDialog;
import nl.astron.lofar.sas.otbcomponents.LoadFileDialog;
import nl.astron.lofar.sas.otbcomponents.MultiEditDialog;
import nl.astron.lofar.sas.otbcomponents.TableDialog;
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
        switch (itsTabFocus) {
            case "PIC":
                buttonPanel1.addButton("State History");
                buttonPanel1.setButtonIcon("State History",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_info.gif")));
                buttonPanel1.addButton("Query Panel");
                buttonPanel1.setButtonIcon("Query Panel",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_help.png")));
                buttonPanel1.addButton("New");
                buttonPanel1.setButtonIcon("New",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_new.png")));
                buttonPanel1.addButton("Delete");
                buttonPanel1.setButtonIcon("Delete",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_delete.png")));
                buttonPanel1.addButton("Refresh");
                buttonPanel1.setButtonIcon("Refresh",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_refresh_e.gif")));
                buttonPanel1.addButton("View");
                buttonPanel1.setButtonIcon("View",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif")));
                buttonPanel1.addButton("Info");
                buttonPanel1.setButtonIcon("Info",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_fileopen.gif")));
                buttonPanel1.setButtonEnabled("Delete",false);
                buttonPanel1.setButtonEnabled("View",false);
                buttonPanel1.setButtonEnabled("Info",false);
                buttonPanel1.setButtonEnabled("State History",false);
                break;
            case "VIC":
                buttonPanel1.addButton("State History");
                buttonPanel1.setButtonIcon("State History",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_info.gif")));
                buttonPanel1.addButton("Query Panel");
                buttonPanel1.setButtonIcon("Query Panel",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_help.png")));
                buttonPanel1.addButton("Delete");
                buttonPanel1.setButtonIcon("Delete",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_delete.png")));
                buttonPanel1.addButton("Refresh");
                buttonPanel1.setButtonIcon("Refresh",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_refresh_e.gif")));
                buttonPanel1.addButton("View");
                buttonPanel1.setButtonIcon("View",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif")));
                buttonPanel1.addButton("Schedule");
                buttonPanel1.setButtonIcon("Schedule",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_fileopen.gif")));
                buttonPanel1.setButtonEnabled("State History",false);
                buttonPanel1.setButtonEnabled("Delete",false);
                buttonPanel1.setButtonEnabled("View",false);
                buttonPanel1.setButtonEnabled("Schedule",false);
                break;
            case "Templates":
                buttonPanel1.addButton("State History");
                buttonPanel1.setButtonIcon("State History",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_info.gif")));
                buttonPanel1.addButton("Query Panel");
                buttonPanel1.setButtonIcon("Query Panel",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_help.png")));
                buttonPanel1.addButton("Duplicate");
                buttonPanel1.setButtonIcon("Duplicate",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_copy.png")));
                buttonPanel1.addButton("Modify");
                buttonPanel1.setButtonIcon("Modify",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif")));
                buttonPanel1.addButton("Delete");
                buttonPanel1.setButtonIcon("Delete",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_delete.png")));
                buttonPanel1.addButton("Refresh");
                buttonPanel1.setButtonIcon("Refresh",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_refresh_e.gif")));
                buttonPanel1.addButton("Build VIC tree");
                buttonPanel1.setButtonIcon("Build VIC tree",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_new.png")));
                buttonPanel1.addButton("Change Status");
                buttonPanel1.setButtonIcon("Change Status",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_fileopen.gif")));
                buttonPanel1.addButton("MultiEdit");
                buttonPanel1.setButtonIcon("MultiEdit",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif" )));
                buttonPanel1.addButton("Set to Default");
                buttonPanel1.setButtonIcon("Set to Default",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_redo.png")));
                buttonPanel1.setButtonEnabled("Duplicate",false);
                buttonPanel1.setButtonEnabled("Modify",false);
                buttonPanel1.setButtonEnabled("Delete",false);
                buttonPanel1.setButtonEnabled("Build VIC tree",false);
                buttonPanel1.setButtonEnabled("Change Status",false);
                buttonPanel1.setButtonEnabled("MultiEdit",false);
                buttonPanel1.setButtonEnabled("Set to Default",false);
                buttonPanel1.setButtonEnabled("State History",false);
                break;
            case "Default Templates":
                buttonPanel1.addButton("State History");
                buttonPanel1.setButtonIcon("State History",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_info.gif")));
                buttonPanel1.addButton("Duplicate");
                buttonPanel1.setButtonIcon("Duplicate",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_copy.png")));
                buttonPanel1.addButton("Modify");
                buttonPanel1.setButtonIcon("Modify",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif")));
                buttonPanel1.addButton("Refresh");
                buttonPanel1.setButtonIcon("Refresh",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_refresh_e.gif")));
                buttonPanel1.addButton("Change Status");
                buttonPanel1.setButtonIcon("Change Status",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_fileopen.gif")));
                buttonPanel1.setButtonEnabled("Modify",false);
                buttonPanel1.setButtonEnabled("Duplicate",false);
                buttonPanel1.setButtonEnabled("Change Status",false);
                buttonPanel1.setButtonEnabled("State History",false);
                break;
            case "Components":
                buttonPanel1.addButton("Query Panel");
                buttonPanel1.setButtonIcon("Query Panel",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_help.png")));
                buttonPanel1.addButton("New");
                buttonPanel1.setButtonIcon("New",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_new.png")));
                buttonPanel1.addButton("Modify");
                buttonPanel1.setButtonIcon("Modify",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif")));
                buttonPanel1.addButton("Delete");
                buttonPanel1.setButtonIcon("Delete",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_delete.png")));
                buttonPanel1.addButton("Refresh");
                buttonPanel1.setButtonIcon("Refresh",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_refresh_e.gif")));
                buttonPanel1.addButton("Build TemplateTree");
                buttonPanel1.setButtonIcon("Build TemplateTree",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_redo.png")));
                //            buttonPanel1.setButtonEnabled("Delete",false);
                buttonPanel1.setButtonEnabled("Modify",false);
                buttonPanel1.setButtonEnabled("Build TemplateTree",false);
                break;
            case "Query Results":
                break;
        }
        buttonPanel1.addButton("Quit");
        buttonPanel1.setButtonIcon("Quit",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_exit2.png")));
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
        PICPanel.setTableCellAlignment(JLabel.LEFT);
        

        VICtableModel VICmodel = new VICtableModel(SharedVars.getOTDBrmi());
        VICPanel.setTableModel(VICmodel);
        VICPanel.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        VICPanel.setColumnSize("ID",40);
        VICPanel.setColumnSize("PType", 60);
        VICPanel.setColumnSize("PStype", 60);
        VICPanel.setColumnSize("Strat", 60);
        VICPanel.setColumnSize("StartTime",150);
        VICPanel.setColumnSize("StopTime",150);
        VICPanel.setColumnSize("Description",300);
        VICPanel.setAutoCreateRowSorter(true);
        VICPanel.setTableCellAlignment(JLabel.LEFT);
        
        TemplatetableModel Templatemodel = new TemplatetableModel(SharedVars.getOTDBrmi());
        TemplatesPanel.setTableModel(Templatemodel);
        TemplatesPanel.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        TemplatesPanel.setColumnSize("ID",40);
        TemplatesPanel.setColumnSize("PType", 60);
        TemplatesPanel.setColumnSize("PStype", 60);
        TemplatesPanel.setColumnSize("Strat", 60);
        TemplatesPanel.setColumnSize("Description",300);
        TemplatesPanel.setAutoCreateRowSorter(true);
        TemplatesPanel.setTableCellAlignment(JLabel.LEFT);

        DefaultTemplatetableModel DefaultTemplatemodel = new DefaultTemplatetableModel(SharedVars.getOTDBrmi());
        DefaultTemplatesPanel.setTableModel(DefaultTemplatemodel);
        DefaultTemplatesPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        DefaultTemplatesPanel.setColumnSize("ID",40);
        DefaultTemplatesPanel.setColumnSize("Name",150);
        DefaultTemplatesPanel.setColumnSize("PType", 60);
        DefaultTemplatesPanel.setColumnSize("PStype", 60);
        DefaultTemplatesPanel.setColumnSize("Strat", 60);
        DefaultTemplatesPanel.setColumnSize("Description",300);
        DefaultTemplatesPanel.setAutoCreateRowSorter(true);
        DefaultTemplatesPanel.setTableCellAlignment(JLabel.LEFT);


        ComponentTableModel Componentmodel = new ComponentTableModel(SharedVars.getOTDBrmi());
        ComponentsPanel.setTableModel(Componentmodel);
        ComponentsPanel.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        ComponentsPanel.setColumnSize("ID",50);
        ComponentsPanel.setColumnSize("Description",685);
        ComponentsPanel.setAutoCreateRowSorter(true);
        ComponentsPanel.setTableCellAlignment(JLabel.LEFT);
        
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
        if (inputFieldBuilder.currentInputField != null) {
            inputFieldBuilder.currentInputField.checkPopup();
        }
        logger.debug("Check Changed status");
        if (this.hasChanged()) {
            // keep selected tree
            int aSavedID=itsMainFrame.getSharedVars().getTreeID();
            itsMainFrame.setHourglassCursor();
            switch (itsTabFocus) {
                case "PIC":
                    if (!((PICtableModel)PICPanel.getTableModel()).fillTable()) {
                        logger.error("error filling PICtable");
                    }
                    break;
                case "VIC":
                    if (!((VICtableModel)VICPanel.getTableModel()).fillTable()) {
                        logger.error("error filling VICtable");
                    }
                    break;
                case "Templates":
                    if (!((TemplatetableModel)TemplatesPanel.getTableModel()).fillTable()) {
                        logger.error("error filling templateTable");
                    }
                    break;
                case "Default Templates":
                    if (!((DefaultTemplatetableModel)DefaultTemplatesPanel.getTableModel()).fillTable()) {
                        logger.error("error filling Default templateTable");
                    }
                    break;
                case "Components":
                    if (!((ComponentTableModel)ComponentsPanel.getTableModel()).fillTable()) {
                        logger.error("error filling ComponentsTable");
                    }
                    break;
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
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jTabbedPane1 = new javax.swing.JTabbedPane();
        PICPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        VICPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        TemplatesPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        DefaultTemplatesPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        ComponentsPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        QueryResultsPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        AdminPanel = new javax.swing.JPanel();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jTabbedPane1.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                jTabbedPane1StateChanged(evt);
            }
        });
        jTabbedPane1.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
            public void propertyChange(java.beans.PropertyChangeEvent evt) {
                jTabbedPane1PropertyChange(evt);
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

        DefaultTemplatesPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                DefaultTemplatesPanelMouseClicked(evt);
            }
        });
        jTabbedPane1.addTab("Default Templates", DefaultTemplatesPanel);

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
        jTabbedPane1.addTab("Admin", AdminPanel);

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
        if(evt.getClickCount() == 1) {
            validateButtons();
        } else {

        }
    }//GEN-LAST:event_QueryResultsPanelMouseClicked

    private void ComponentsPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_ComponentsPanelMouseClicked
        if(evt.getClickCount() == 1) {
            validateButtons();
        } else {
            if (buttonPanel1.isButtonEnabled("Modify") && buttonPanel1.isButtonVisible("Modify")) {
                buttonPanelAction("Modify");
            }
        }
    }//GEN-LAST:event_ComponentsPanelMouseClicked

    private void TemplatesPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_TemplatesPanelMouseClicked
        if(evt.getClickCount() == 1) {
            validateButtons();
        } else {
            if (buttonPanel1.isButtonEnabled("Modify") && buttonPanel1.isButtonVisible("Modify")) {
                buttonPanelAction("Modify");
            }
        }
    }//GEN-LAST:event_TemplatesPanelMouseClicked

    private void VICPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_VICPanelMouseClicked
        if(evt.getClickCount() == 1) {
            validateButtons();
        } else {
            if (buttonPanel1.isButtonEnabled("View") && buttonPanel1.isButtonVisible("View")) {
                buttonPanelAction("View");
            }
        }
    }//GEN-LAST:event_VICPanelMouseClicked

    private void PICPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_PICPanelMouseClicked
        if(evt.getClickCount() == 1) {
            validateButtons();
        } else {
            if (buttonPanel1.isButtonEnabled("View") && buttonPanel1.isButtonVisible("View")) {
                buttonPanelAction("View");
            }
        }
    }//GEN-LAST:event_PICPanelMouseClicked

    private void jTabbedPane1StateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_jTabbedPane1StateChanged
        if (!itsTabFocus.equals(jTabbedPane1.getTitleAt(jTabbedPane1.getSelectedIndex()))) {
            itsTabFocus=jTabbedPane1.getTitleAt(jTabbedPane1.getSelectedIndex());
            // force reload
            logger.debug("Tab changed, force reload: "+ itsTabFocus);
            this.setChanged(true);
        }
        if (buttonsInitialized) {        
            initializeButtons();
            validateButtons();
           // Force a refresh since MoM/scheduler might have added something to the trees
           //set changed flag, we want to refresh the tree
           logger.debug("Refresh table");
           itsMainFrame.setChanged(this.getFriendlyName(),true);
           checkChanged();
        }


    }//GEN-LAST:event_jTabbedPane1StateChanged

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt.getActionCommand());
        buttonPanelAction(evt.getActionCommand());
    }//GEN-LAST:event_buttonPanel1ActionPerformed

    private void DefaultTemplatesPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_DefaultTemplatesPanelMouseClicked
        if(evt.getClickCount() == 1) {
            validateButtons();
        } else {
            if (buttonPanel1.isButtonEnabled("Modify") && buttonPanel1.isButtonVisible("Modify")) {
                buttonPanelAction("Modify");
            }
        }
    }//GEN-LAST:event_DefaultTemplatesPanelMouseClicked
    
    /** Returns the selected row in the present tree */
    private int getSelectedRow() {
        int aRow=-1;
        switch (itsTabFocus) {
            case "PIC":
                aRow = PICPanel.getSelectedRow();
                break;
            case "VIC":
                aRow = VICPanel.getSelectedRow();
                break;
            case "Templates":
                aRow = TemplatesPanel.getSelectedRow();
                break;
            case "Default Templates":
                aRow = DefaultTemplatesPanel.getSelectedRow();
                break;
            case "Components":
                aRow = ComponentsPanel.getSelectedRow();
                break;
        }
        return aRow;
    }

    /** Returns the selected rows in the present tree */
    private int[] getSelectedRows() {
        int [] rows=null;
        switch (itsTabFocus) {
            case "PIC":
                rows = PICPanel.getSelectedRows();
                break;
            case "VIC":
                rows = VICPanel.getSelectedRows();
                break;
            case "Templates":
                rows = TemplatesPanel.getSelectedRows();
                break;
            case "Default Templates":
                rows = DefaultTemplatesPanel.getSelectedRows();
                break;
            case "Components":
                rows = ComponentsPanel.getSelectedRows();
                break;
        }
        return rows;
    }

    /** Sets the selected row in the present tree */
    private void setSelectedID(int aTreeID) {
        if (aTreeID > -1) {
            switch (itsTabFocus) {
                case "PIC":
                    PICPanel.setSelectedID(aTreeID);
                    break;
                case "VIC":
                    VICPanel.setSelectedID(aTreeID);
                    break;
                case "Templates":
                    TemplatesPanel.setSelectedID(aTreeID);
                    break;
                case "Default Templates":
                    DefaultTemplatesPanel.setSelectedID(aTreeID);
                    break;
                case "Components":
                    ComponentsPanel.setSelectedID(aTreeID);
                    break;
            }
        }
    }
    
    /** Returns the ids of the selected Trees in case of a multiple selection */
    private int [] getSelectedTreeIDs() {
        int [] rows=this.getSelectedRows();
        int [] treeIDs=new int[rows.length];
        switch (itsTabFocus) {
            case "PIC":
                for (int i=0; i < rows.length; i++) {
                    treeIDs[i] = ((Integer)PICPanel.getTableModel().getValueAt(rows[i], 0)).intValue();
                }
                break;
            case "VIC":
                for (int i=0; i < rows.length; i++) {
                    treeIDs[i] = ((Integer)VICPanel.getTableModel().getValueAt(rows[i], 0)).intValue();
                }
                break;
            case "Templates":
                for (int i=0; i < rows.length; i++) {
                    treeIDs[i] = ((Integer)TemplatesPanel.getTableModel().getValueAt(rows[i], 0)).intValue();
                }
                break;
            case "Default Templates":
                for (int i=0; i < rows.length; i++) {
                    treeIDs[i] = ((Integer)DefaultTemplatesPanel.getTableModel().getValueAt(rows[i], 0)).intValue();
                }
                break;
            case "Components":
                for (int i=0; i < rows.length; i++) {
                    // is the node ID in the case of Components
                    treeIDs[i] = ((Integer)ComponentsPanel.getTableModel().getValueAt(rows[i], 0)).intValue();
                }
                break;
        }

        return treeIDs;   
    }

    /** Returns the id of the selected Tree */
    private int getSelectedTreeID() {
        int treeID=0;
        int aRow=this.getSelectedRow();
        switch (itsTabFocus) {
            case "PIC":
                if ( aRow > -1) {
                    treeID = ((Integer)PICPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                    if (treeID > 0) {
                        itsMainFrame.getSharedVars().setTreeID(treeID);
                    } else {
                        logger.debug("Tree not found");
                    }
                }
                break;
            case "VIC":
                if ( aRow > -1) {
                    treeID = ((Integer)VICPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                    if (treeID > 0) {
                        itsMainFrame.getSharedVars().setTreeID(treeID);
                    } else {
                        logger.debug("Tree not found");
                    }
                }
                break;
            case "Templates":
                if ( aRow > -1) {
                    treeID = ((Integer)TemplatesPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                    if (treeID > 0) {
                        itsMainFrame.getSharedVars().setTreeID(treeID);
                    } else {
                        logger.debug("Tree not found");
                    }
                }
                break;
            case "Default Templates":
                if ( aRow > -1) {
                    treeID = ((Integer)DefaultTemplatesPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                    if (treeID > 0) {
                        itsMainFrame.getSharedVars().setTreeID(treeID);
                    } else {
                        logger.debug("DefaultTree not found");
                    }
                }
                break;
            case "Components":
                if ( aRow > -1) {
                    // is the node ID in the case of Components
                    treeID = ((Integer)ComponentsPanel.getTableModel().getValueAt(aRow, 0)).intValue();
                    if (treeID > 0) {
                        itsMainFrame.getSharedVars().setComponentID(treeID);
                    } else {
                        logger.debug("Component not found");
                    }
                }
                break;
        }
        return treeID;
    }
    
    /** Perform actions depending on the Button pressed and the Tab active
     *
     * @param   aButton     Name of the pressed button
     */
    private void buttonPanelAction(String aButton) {
        logger.debug("Button pressed: "+aButton+ "  ActiveTab: " + itsTabFocus);
        if (inputFieldBuilder.currentInputField != null) {
            inputFieldBuilder.currentInputField.checkPopup();
        }
        int treeID=getSelectedTreeID();
        switch (aButton) {
            case "Quit":
                itsMainFrame.exit();
                return;
            case "Refresh":
                //set changed flag, we want to refresh the tree
                itsMainFrame.setChanged(this.getFriendlyName(),true);
                checkChanged();
                return;
        }
        switch (itsTabFocus) {
            case "PIC":
                if (treeID > 0) {
                    itsMainFrame.getSharedVars().setTreeID(treeID);
                } else if (!aButton.equals("New")) {
                    LofarUtils.showErrorPanel(this,"You didn't select a tree",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));

                    return;
                }
        switch (aButton) {
            case "Query Panel":
                // TODO open Query Panel
                itsMainFrame.ToDo();
                break;
            case "State History":
                if (treeID > 0) {
                    viewStateChanges(treeID);
                }
                break;
            case "New":
                if (getFile("PIC-tree")) {
                    try {
                       // the file obviously resides at the client side, and needs to be transfered to the server side.
                       byte uldata[] = new byte[(int)itsNewFile.length()]; 
                try (BufferedInputStream input = new BufferedInputStream(new FileInputStream(itsNewFile))) {
                    input.read(uldata,0,uldata.length);
                }
                       String aFileName= "/tmp/"+itsMainFrame.getUserAccount().getUserName()+"_"+itsNewFile.getName();
                       if (OtdbRmi.getRemoteFileTrans().uploadFile(uldata,aFileName)) {
                           logger.debug("upload finished");                       
                           // Create a new Tree from the found file.
                           int aTreeID=OtdbRmi.getRemoteMaintenance().loadMasterFile(aFileName);
                           if (aTreeID < 1) {
                               String aS="Error on fileLoad: " + aFileName;
                               logger.error(aS);
                               LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                           } else {
                               // set changed flag to reload mainpanel
                               itsMainFrame.setChanged(this.getFriendlyName(),true);
                               // set the new created TreeID to active and fill description stuff if needed
                               itsMainFrame.getSharedVars().setTreeID(aTreeID);
                               checkChanged();
                               if (!itsFileDescription.equals("")) {
                                  if (!OtdbRmi.getRemoteMaintenance().setDescription(aTreeID,itsFileDescription)) {
                                      String aS="Error during setDescription in Tree "+OtdbRmi.getRemoteMaintenance().errorMsg();
                                     logger.error(aS);
                                     LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                                  }
                               }
                                  
                               if (!itsFileStatus.equals("")) {
                                  if (!OtdbRmi.getRemoteMaintenance().setTreeState(aTreeID,OtdbRmi.getRemoteTypes().getTreeState(itsFileStatus))) {
                                      String aS="Error during setStatus in Tree "+OtdbRmi.getRemoteMaintenance().errorMsg();
                                     logger.error(aS);
                                     LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                                  }
                               }                               
                           }
                           ResultBrowserPanel aP=(ResultBrowserPanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ResultBrowserPanel", true, true);
                           if (aP != null) {
                              itsMainFrame.showPanel(aP.getFriendlyName());
                           }
                        }
                   } catch (RemoteException ex) {
                       String aS="Error during newPICTree creation: "+ ex;
                       logger.error(aS);
                       LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                   } catch (FileNotFoundException ex) {
                       String aS="Error during newPICTree creation: "+ ex;
                       logger.error(aS);
                       LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                   } catch (IOException ex) {
                       String aS="Error during newPICTree creation: "+ ex;
                       logger.error(aS);
                       LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                   }
                }
                break;
            case "Delete":
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree(s): ?","Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        int[] treeIDs=getSelectedTreeIDs();
                        for (int i=0;i< treeIDs.length;i++) {
                            if (!OtdbRmi.getRemoteMaintenance().deleteTree(treeIDs[i])) {
                                String aS="Failed to delete tree: "+treeIDs[i];
                                logger.error(aS);
                                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            }
                        }
                    } catch (RemoteException ex) {
                        String aS="Remote error during deleteTree: "+ ex;
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    }
                    itsMainFrame.setHourglassCursor();
                    ((PICtableModel)PICPanel.getTableModel()).fillTable();
                    itsMainFrame.setNormalCursor();
                }
                break;
            case "View":
                ResultBrowserPanel aP=(ResultBrowserPanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ResultBrowserPanel", true, true);
                if (aP != null) {
                    itsMainFrame.showPanel(aP.getFriendlyName());
                }
                break;
            case "Info":
                if (treeID > 0) {
                    int [] id=new int[1];
                    id[0]=treeID;
                    if (viewInfo(id)) {
                        
                        logger.debug("Tree has been changed, reloading tableline");
                          itsMainFrame.setChanged(this.getFriendlyName(),true);
                          checkChanged();
                    }
                }
                break;
        }
                break;
            case "VIC":
                if (treeID > 0) {
                    itsMainFrame.getSharedVars().setTreeID(treeID);
                } else {
                    LofarUtils.showErrorPanel(this, "You didn't select a tree",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    return;
                }
        switch (aButton) {
            case "Query Panel":
                // TODO open Query Panel
                itsMainFrame.ToDo();
                break;
            case "State History":
                if (treeID > 0) {
                    viewStateChanges(treeID);
                }
                break;
            case "Delete":
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree(s): ?","Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        int[] treeIDs=getSelectedTreeIDs();
                        for (int i=0;i< treeIDs.length;i++) {
                            if (!OtdbRmi.getRemoteMaintenance().deleteTree(treeIDs[i])) {
                                String aS="Failed to delete tree: "+treeIDs[i];
                                logger.error(aS);
                                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            }
                        }
                    } catch (RemoteException ex) {
                        String aS="Remote error during deleteTree: "+ ex;
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this, aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    }
                    itsMainFrame.setHourglassCursor();
                    ((VICtableModel)VICPanel.getTableModel()).fillTable();
                    itsMainFrame.setNormalCursor();
                }
                break;
            case "View":
                ResultBrowserPanel aP=(ResultBrowserPanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ResultBrowserPanel", true, true);
                if (aP != null) {
                    itsMainFrame.showPanel(aP.getFriendlyName());
                }
                break;
            case "Schedule":
                // in case of VICtree we have the possibility of changing a multiple selection
                // so things like start and/or stoptimes can be set for a few entries at once
                if (this.VICPanel.getSelectedRowCount() > 0) {
                    if (viewInfo(this.getSelectedTreeIDs()) ) {
                        logger.debug("Tree has been changed, reloading tableline");
                          itsMainFrame.setChanged(this.getFriendlyName(),true);
                          checkChanged();
                    }
                    
                }
                break;
        }
                break;
            case "Templates":
                {
                    jOTDBtree aTree=null;
                    String aTreeState="";
                    if (treeID > 0) {
                        itsMainFrame.getSharedVars().setTreeID(treeID);
                        try {
                            aTree =    OtdbRmi.getRemoteOTDB().getTreeInfo(treeID,false);
                            aTreeState=OtdbRmi.getRemoteTypes().getTreeState(aTree.state);
                        } catch (RemoteException ex) {
                            String aS="Error during Remote treeMaintenance";
                            logger.debug(aS);
                            LofarUtils.showErrorPanel(this, aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    } else {

                        LofarUtils.showErrorPanel(this, "You didn't select a tree",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        return;
                    }
            switch (aButton) {
                case "Query Panel":
                    itsMainFrame.ToDo();
                    break;
                case "State History":
                    if (treeID > 0) {
                        viewStateChanges(treeID);
                    }
                    break;
                case "Duplicate":
                    if (treeID < 1) {
                        LofarUtils.showErrorPanel(this, "You didn't select a tree",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    } else {
                        try {
                            int newTreeID=OtdbRmi.getRemoteMaintenance().copyTemplateTree(treeID);
                            if (newTreeID > 0) {
                                JOptionPane.showMessageDialog(this,"New Tree created with ID: "+newTreeID,
                                    "New Tree Message",
                                    JOptionPane.INFORMATION_MESSAGE);
                                // set back treestate to described
                                jOTDBtree aT=OtdbRmi.getRemoteOTDB().getTreeInfo(newTreeID, false); 
                                if (aT.state != OtdbRmi.getRemoteTypes().getTreeState("described") ) {
                                    aT.state=OtdbRmi.getRemoteTypes().getTreeState("described");
                                    if (!OtdbRmi.getRemoteMaintenance().setTreeState(aT.treeID(), aT.state)) {
                                        String aS="Error during setTreeState: "+OtdbRmi.getRemoteMaintenance().errorMsg();
                                        logger.error(aS);
                                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                                    }
                                }
                                // check momID, if not zero set to zero
                                if (aT.momID() != 0) {
                                    if (!OtdbRmi.getRemoteMaintenance().setMomInfo(aT.treeID(),0,aT.groupID,aT.campaign)) {
                                        String aS="Error during setMomInfo: "+OtdbRmi.getRemoteMaintenance().errorMsg();
                                        logger.debug(aS);
                                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
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
                            String aS="Remote error during Build TemplateTree: "+ ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    }
                    break;
                case "Modify":
                    TemplateMaintenancePanel aP =(TemplateMaintenancePanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.TemplateMaintenancePanel", true, true);
                    if (aP != null) {
                        itsMainFrame.showPanel(aP.getFriendlyName());
                    }
                    break;
                case "Delete":
                    if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this tree(s) ?","Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                        try {
                           int[] treeIDs=getSelectedTreeIDs();
                           for (int i=0;i< treeIDs.length;i++) {
                               if (!OtdbRmi.getRemoteMaintenance().deleteTree(treeIDs[i])) {
                                   String aS="Failed to delete tree: "+treeIDs[i];
                                   logger.error(aS);
                                   LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                               }
                           }
                        } catch (RemoteException ex) {
                           String aS="Remote error during deleteTree: "+ ex;
                           logger.error(aS);
                           LofarUtils.showErrorPanel(this, aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                       }
                       itsMainFrame.getSharedVars().setTreeID(-1);                              
                       itsMainFrame.setHourglassCursor();
                       ((TemplatetableModel)TemplatesPanel.getTableModel()).fillTable();
                       itsMainFrame.setNormalCursor();
                       // set changed flag to reload mainpanel
   //                  itsMainFrame.setChanged(this.getFriendlyName(),true);
   //                    checkChanged();
                   }break;
                case "Build VIC tree":
                    if (treeID < 1) {
                        LofarUtils.showErrorPanel(this, "You didn't select a tree",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    } else {
                        try {
                            int newTreeID=OtdbRmi.getRemoteMaintenance().instanciateTree(itsMainFrame.getSharedVars().getTreeID());
                            if (newTreeID > 0) {
                                JOptionPane.showMessageDialog(this,"New VICTree created with ID: "+newTreeID,
                                    "New Tree Message",
                                    JOptionPane.INFORMATION_MESSAGE);
                                itsMainFrame.getSharedVars().setTreeID(newTreeID);
                                // set changed flag to reload mainpanel
                                itsMainFrame.setChanged(this.getFriendlyName(),true);
                                checkChanged();
                            } else {
                                String aS="No VIC Tree created!!! : "+ OtdbRmi.getRemoteMaintenance().errorMsg();
                                logger.error(aS);
                                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            }
               
                        } catch (RemoteException ex) {
                            String aS="Remote error during Build VICTree: "+ ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    }
                    break;
                case "Change Status":
                    // in case of templatetree we have the possibility of changing a multiple selection
                    // so things status can be set for a few entries at once
                    if (TemplatesPanel.getSelectedRowCount() > 0) {
                        if (viewInfo(this.getSelectedTreeIDs()) ) {
                            logger.debug("Tree has been changed, reloading tableline");
                              itsMainFrame.setChanged(this.getFriendlyName(),true);
                              checkChanged();
                        }

                    }
                    break;
                case "MultiEdit":
                    // in case of templatetree we have the possibility to change a few crucial settings for all trees chosen
                    if (TemplatesPanel.getSelectedRowCount() > 0) {
                        if (viewMultiEditDialog(this.getSelectedTreeIDs()) ) {
                            logger.debug("Trees have been changed, reloading tablelines");
                              itsMainFrame.setChanged(this.getFriendlyName(),true);
                              checkChanged();
                        }

                    }
                    break;
                case "Set to Default":
                    if (itsMainFrame.getSharedVars().getTreeID() > 0) {
                        try {
                            ArrayList<jDefaultTemplate> aDFList = new ArrayList(OtdbRmi.getRemoteOTDB().getDefaultTemplates());
                            if (createDefaultTemplateDialog(aTree, aDFList)) {
                                // check momID, if not zero set to zero
                                if (aTree.momID() != 0) {
                                    if (!OtdbRmi.getRemoteMaintenance().setMomInfo(aTree.treeID(), 0, aTree.groupID, aTree.campaign)) {
                                        String aS = "Error during setMomInfo: " + OtdbRmi.getRemoteMaintenance().errorMsg();
                                        logger.error(aS);
                                        LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                                    }
                                }
                                itsMainFrame.setChanged(this.getFriendlyName(), true);
                                checkChanged();
                            }
                        } catch (RemoteException ex) {
                            String aS="Error during Remote treeMaintenance" + ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    }
                    break;
            }
                    break;
                }
            case "Default Templates":
                {
                    jOTDBtree aTree=null;
                    String aTreeState="";
                    if (treeID > 0) {
                        itsMainFrame.getSharedVars().setTreeID(treeID);
                        try {
                            aTree =    OtdbRmi.getRemoteOTDB().getTreeInfo(treeID,false);
                            aTreeState=OtdbRmi.getRemoteTypes().getTreeState(aTree.state);
                        } catch (RemoteException ex) {
                            String aS="Error during Remote treeMaintenance" + ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    } else {
                        LofarUtils.showErrorPanel(this, "You didn't select a tree",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        return;
                    }
            switch (aButton) {
                case "Query Panel":
                    itsMainFrame.ToDo();
                    break;
                case "State History":
                    if (treeID > 0) {
                        viewStateChanges(treeID);
                    }
                    break;
                case "Duplicate":
                    if (treeID < 1) {
                        LofarUtils.showErrorPanel(this, "You didn't select a tree",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    } else {
                        try {
                            int newTreeID=OtdbRmi.getRemoteMaintenance().copyTemplateTree(treeID);
                            if (newTreeID > 0) {
                                JOptionPane.showMessageDialog(this,"New Tree (Not Default!!!) created with ID: "+newTreeID,
                                    "New Tree Message",
                                    JOptionPane.INFORMATION_MESSAGE);
                                // set back treestate to described
                                jOTDBtree aT=OtdbRmi.getRemoteOTDB().getTreeInfo(newTreeID, false);
                                if (aT.state != OtdbRmi.getRemoteTypes().getTreeState("described") ) {
                                    aT.state=OtdbRmi.getRemoteTypes().getTreeState("described");
                                    if (!OtdbRmi.getRemoteMaintenance().setTreeState(aT.treeID(), aT.state)) {
                                        String aS="Error during setTreeState: "+OtdbRmi.getRemoteMaintenance().errorMsg();
                                        logger.error(aS);
                                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                                    }
                                }
                                // check momID, if not zero set to zero
                                if (aT.momID() != 0) {
                                    if (!OtdbRmi.getRemoteMaintenance().setMomInfo(aT.treeID(),0, aT.groupID, aT.campaign)) {
                                        String aS="Error during setMomInfo: "+OtdbRmi.getRemoteMaintenance().errorMsg();
                                        logger.error(aS);
                                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                                    }

                                }
                                itsMainFrame.getSharedVars().setTreeID(newTreeID);
                                // set changed flag to reload mainpanel
                                itsMainFrame.setChanged(this.getFriendlyName(),true);
                                checkChanged();
                            } else {
                                String aS="No Template Tree created!!!";
                                logger.error(aS);
                                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            }

                        } catch (RemoteException ex) {
                            String aS="Remote error during Build TemplateTree: "+ ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    }
                    break;
                case "Modify":
                    TemplateMaintenancePanel aP =(TemplateMaintenancePanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.TemplateMaintenancePanel", true, true);
                    if (aP != null) {
                        itsMainFrame.showPanel(aP.getFriendlyName());
                    }
                    break;
                case "Change Status":
                    if (itsMainFrame.getSharedVars().getTreeID() > 0) {
                        int [] id = new int[1];
                        id[0]=itsMainFrame.getSharedVars().getTreeID();
                        if (viewInfo(id)) {
                            logger.debug("Tree has been changed, reloading table line");
                              itsMainFrame.setChanged(this.getFriendlyName(),true);
                              checkChanged();
                        }
                    }
                    break;
            }
                    break;
                }
            case "Components":
        switch (aButton) {
            case "Query Panel":
                itsMainFrame.ToDo();
                break;
            case "New":
                if (getFile("VIC-component") ) {
                    try {
                        // the file obviously resides at the client side, and needs to be transfered to the server side.
                        byte uldata[] = new byte[(int)itsNewFile.length()]; 
                try (BufferedInputStream input = new BufferedInputStream(new FileInputStream(itsNewFile))) {
                    input.read(uldata,0,uldata.length);
                }
                        String aFileName= "/tmp/"+itsMainFrame.getUserAccount().getUserName()+"_"+itsNewFile.getName();
                        if (OtdbRmi.getRemoteFileTrans().uploadFile(uldata,aFileName)) {
                            logger.debug("upload finished");
                            // Create a new Tree from the found file.
                            int anID=OtdbRmi.getRemoteMaintenance().loadComponentFile(aFileName,"","");
                            if (anID < 1) {
                                String aS="Error on ComponentfileLoad: " + itsNewFile.getPath();
                                logger.error(aS);
                                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            } else {
                                // set the new created fill description stuff if needed
                                itsMainFrame.getSharedVars().setComponentID(anID);
                                if (!itsFileDescription.equals("")) {
                                    jVICnodeDef aND=OtdbRmi.getRemoteMaintenance().getComponentNode(anID);
                                    aND.description=itsFileDescription;
                                    if (!OtdbRmi.getRemoteMaintenance().saveComponentNode(aND)) {
                                        String aS="Error during setDescription in Component "+OtdbRmi.getRemoteMaintenance().errorMsg();
                                        logger.error(aS);
                                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                                    }
                                }
                                // set changed flag to reload mainpanel
                                itsMainFrame.setChanged(this.getFriendlyName(),true);
                                checkChanged();
                            }
                            ComponentMaintenancePanel aP=(ComponentMaintenancePanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ComponentMaintenancePanel", true, true);
                            if (aP != null) {
                                itsMainFrame.showPanel(aP.getFriendlyName());
                            }
                        } else {
                            String aS="upload failed";
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/tb/icons/16_warn.gif")));
                        }
                    } catch (FileNotFoundException ex) {
                        String aS="Error during new Component creation: "+ ex;
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    } catch (RemoteException ex) {
                        String aS="Error during new Component creation: "+ ex;
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    } catch (IOException ex) {
                        String aS="Error during new Component creation: "+ ex;
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    }
                }
                break;
            case "Modify":
                ComponentMaintenancePanel aP = (ComponentMaintenancePanel)itsMainFrame.registerPlugin("nl.astron.lofar.sas.otb.panels.ComponentMaintenancePanel", true, true);
                if (aP != null) {
                    itsMainFrame.showPanel(aP.getFriendlyName());
                }
                break;
            case "Build TemplateTree":
                int nodeID=itsMainFrame.getSharedVars().getComponentID();
                short classifID;
                try {
                    classifID = OtdbRmi.getRemoteTypes().getClassif("operational");
                    if (OtdbRmi.getRemoteMaintenance().isTopComponent(nodeID)) {
                        int newTreeID=OtdbRmi.getRemoteMaintenance().buildTemplateTree(nodeID,classifID);
                        if (newTreeID > 0) {
                            JOptionPane.showMessageDialog(this,"New Tree created with ID: "+newTreeID,
                                "New Tree Message",
                                JOptionPane.INFORMATION_MESSAGE);
                            itsMainFrame.getSharedVars().setTreeID(newTreeID);
                            // set changed flag to reload mainpanel
                            itsMainFrame.setChanged(this.getFriendlyName(),true);
                            checkChanged();
                        } else {
                            String aS="No Template Tree created!!!";
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    }
                } catch (RemoteException ex) {
                    String aS="Remote error during Build TemplateTree: "+ ex;
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                }
                break;
            case "Delete":
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this component(s): ?","Delete Component",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        int[] componentIDs=getSelectedTreeIDs();
                        for (int i=0;i< componentIDs.length;i++) {
                            if (!OtdbRmi.getRemoteMaintenance().deleteComponentNode(componentIDs[i])) {
                                String aS="Failed to delete component: "+componentIDs[i];
                                logger.error(aS);
                                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            } else {
                                // set changed flag to reload mainpanel
                               itsMainFrame.setChanged(this.getFriendlyName(),true);
                               checkChanged();
                            }
                        }
                    } catch (RemoteException ex) {
                        String aS="Remote error during deleteComponents: "+ ex;
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                    }
                    itsMainFrame.setHourglassCursor();
                    ((ComponentTableModel)ComponentsPanel.getTableModel()).fillTable();
                    itsMainFrame.setNormalCursor();
                }
                break;
        }
                break;
            case "Query Results":
                itsMainFrame.ToDo();
                break;
            default:
                logger.debug("Other command found: "+aButton);
                break;
        }
    }

    /** Launch the statechanges window
     *
     * @param  treeID  The treeId of the chosen tree
     */
    private void viewStateChanges(int treeID) {
        // create tableModel
        itsStateChangeModel = new StateChangeHistoryTableModel(SharedVars.getOTDBrmi(),treeID);
        // showstateChangeInfo
        if (stateChangeHistoryDialog == null) {
            stateChangeHistoryDialog = new TableDialog(itsMainFrame,true,itsStateChangeModel, "State change history");
        } else {
            itsStateChangeModel.setTree(treeID);
            stateChangeHistoryDialog.setModel(itsStateChangeModel);
        }
        stateChangeHistoryDialog.showCancelButton(false);
        stateChangeHistoryDialog.setLocationRelativeTo(this);
        stateChangeHistoryDialog.setTableCellAlignment(JLabel.LEFT);

        stateChangeHistoryDialog.setVisible(true);

    }


    /** Launch multiEditDialog,
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

    /** Launch TreeInfoDialog,
     *
     * @param  treeIDs  The IDs of the chosen trees.
     */
    private boolean viewMultiEditDialog(int[] treeIDs) {
        logger.debug("viewMultiEditDialog for treeID: " + treeIDs);
        //get the selected tree from the database
       
        if (treeIDs.length > 0) {
            // show multiEdit dialog
            if (multiEditDialog == null ) {
                multiEditDialog = new MultiEditDialog(true,treeIDs, itsMainFrame);
            } else {
                multiEditDialog.setTree(treeIDs);
            }
            multiEditDialog.setLocationRelativeTo(this);
            multiEditDialog.setVisible(true);

            if (multiEditDialog.isChanged()) {
                logger.debug("trees have been changed and saved");
            } else {
                logger.debug("trees have not been changed");
            }
               
        } else {
            logger.debug("no trees selected");
        }
        return multiEditDialog.isChanged();
    }

    /** Launch CreateDefaultTemplateDialog,
     *
     * @param  aList Vector<jDefaultTemplate> all default template names.
     */
    private boolean createDefaultTemplateDialog(jOTDBtree aTree,ArrayList<jDefaultTemplate> aList) {
        logger.debug("createDefaultTemplateDialog started");

        if (itsMainFrame.getSharedVars().getTreeID() > 0) {
            // show multiEdit dialog
            if (defaultTemplateDialog == null ) {
                defaultTemplateDialog = new CreateDefaultTemplateDialog(true,aList,aTree,itsMainFrame);
            } else {
                defaultTemplateDialog.setNew(aList,aTree);
            }
            defaultTemplateDialog.setLocationRelativeTo(this);
            defaultTemplateDialog.setVisible(true);

            if (defaultTemplateDialog.isOk()) {
                logger.debug("defaultTemplate is created");
                setChanged(true);
            } else {
                logger.debug("defaultTemplate is not created");
            }

        } else {
            logger.debug("no trees selected");
        }
        return defaultTemplateDialog.isOk();
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
            logger.trace("File to load: " + aNewFile.getName());
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
                String aS="Couldn't get Tree "+ treeID;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                return;
            }
        }
        switch (itsTabFocus) {
            case "PIC":
                if (treeID>0) {
                    if (aTreeState.equals("active")) {
                        buttonPanel1.setButtonEnabled("Delete",false);
                    } else {
                        buttonPanel1.setButtonEnabled("Delete",true);                    
                    }
                    buttonPanel1.setButtonEnabled("View",true);
                    buttonPanel1.setButtonEnabled("Info",true);
                    buttonPanel1.setButtonEnabled("State History",true);
                } else {
                    buttonPanel1.setButtonEnabled("State History",false);
                    buttonPanel1.setButtonEnabled("Delete",false);
                    buttonPanel1.setButtonEnabled("View",false);
                    buttonPanel1.setButtonEnabled("Info",false);
                }
                break;
            case "VIC":
                if (VICPanel.getSelectedRowCount() > 1) {
                    multipleSelection=true;
                } else {
                    multipleSelection=false;
                }
                if (treeID>0) {
                    // !!!!!!!!!!!!!!
                    // Need to see if buttons need to be invalidated under certain states....
                    //
                    if (!multipleSelection) {
                        buttonPanel1.setButtonEnabled("State History",true);
                        buttonPanel1.setButtonEnabled("View",true);
                        buttonPanel1.setButtonEnabled("Query Panel",true);
                        buttonPanel1.setButtonEnabled("Refresh",true);
                    } else {
                        buttonPanel1.setButtonEnabled("Query Panel",false);
                        buttonPanel1.setButtonEnabled("Refresh",false);
                        buttonPanel1.setButtonEnabled("State History",false);
                        buttonPanel1.setButtonEnabled("View",false);
                    }
                    buttonPanel1.setButtonEnabled("Delete",true);
                    buttonPanel1.setButtonEnabled("Schedule",true);
                } else {
                    buttonPanel1.setButtonEnabled("Delete",false);
                    buttonPanel1.setButtonEnabled("View",false);
                    buttonPanel1.setButtonEnabled("State History",false);
                    buttonPanel1.setButtonEnabled("Schedule",false);
                }
                break;
            case "Templates":
                if (TemplatesPanel.getSelectedRowCount() > 1) {
                    multipleSelection=true;
                } else {
                    multipleSelection=false;
                }
                if (treeID > 0) {
                    if ((aTreeState.equals("idle") ||
                            aTreeState.equals("described") ||
                            aTreeState.equals("prepared") ||
                            aTreeState.equals("approved")) && !multipleSelection) {
                        buttonPanel1.setButtonEnabled("Duplicate",true);
                        buttonPanel1.setButtonEnabled("Modify",true);
                        buttonPanel1.setButtonEnabled("Set to Default",true);
                        buttonPanel1.setButtonEnabled("Query Panel",true);
                        if (aTreeState.equals("approved") || aTreeState.equals("on_hold") || aTreeState.equals("prescheduled")) {
                            buttonPanel1.setButtonEnabled("Build VIC tree",true);
                        } else {
                            buttonPanel1.setButtonEnabled("Build VIC tree",false);
                        }
                    } else {
                        buttonPanel1.setButtonEnabled("Duplicate",false);
                        buttonPanel1.setButtonEnabled("Modify",false);                                        
                        buttonPanel1.setButtonEnabled("Set to Default",false);
                    }
                    if (multipleSelection) {
                        buttonPanel1.setButtonEnabled("State History",false);
                        buttonPanel1.setButtonEnabled("Duplicate",false);
                        buttonPanel1.setButtonEnabled("Modify",false);
                        buttonPanel1.setButtonEnabled("MultiEdit",true);
                        buttonPanel1.setButtonEnabled("Query Panel",false);
                        buttonPanel1.setButtonEnabled("Refresh",false);
                    } else {
                        buttonPanel1.setButtonEnabled("State History",true);
                        buttonPanel1.setButtonEnabled("Duplicate",true);
                        buttonPanel1.setButtonEnabled("Modify",true);
                        buttonPanel1.setButtonEnabled("MultiEdit",false);
                        buttonPanel1.setButtonEnabled("Query Panel",true);
                        buttonPanel1.setButtonEnabled("Refresh",true);
                    }
                    buttonPanel1.setButtonEnabled("Delete",true);
                    buttonPanel1.setButtonEnabled("Change Status",true);
                } else {
                    buttonPanel1.setButtonEnabled("Duplicate",false);
                    buttonPanel1.setButtonEnabled("State History",false);
                    buttonPanel1.setButtonEnabled("Modify",false);
                    buttonPanel1.setButtonEnabled("Delete",false);                
                    buttonPanel1.setButtonEnabled("Change Status",false);  
                    buttonPanel1.setButtonEnabled("Build VIC tree",false);
                    buttonPanel1.setButtonEnabled("Set to Default",false);
                }
                break;
            case "Default Templates":
                if (DefaultTemplatesPanel.getSelectedRowCount() > 1) {
                    multipleSelection=true;
                } else {
                    multipleSelection=false;
                }
                if (treeID > 0) {
                    if ((aTreeState.equals("idle") ||
                            aTreeState.equals("described") ||
                            aTreeState.equals("prepared") ||
                            aTreeState.equals("approved")) && !multipleSelection) {
                        buttonPanel1.setButtonEnabled("Duplicate",true);
                        buttonPanel1.setButtonEnabled("Modify",true);
                    } else {
                        buttonPanel1.setButtonEnabled("Duplicate",false);
                        buttonPanel1.setButtonEnabled("Modify",false);
                    }
                    if (multipleSelection) {
                        buttonPanel1.setButtonEnabled("State History",false);
                        buttonPanel1.setButtonEnabled("Duplicate",false);
                        buttonPanel1.setButtonEnabled("Modify",false);
                    } else {
                        buttonPanel1.setButtonEnabled("State History",true);
                        buttonPanel1.setButtonEnabled("Duplicate",true);
                        buttonPanel1.setButtonEnabled("Modify",true);
                    }
                    buttonPanel1.setButtonEnabled("Change Status",true);
                } else {
                    buttonPanel1.setButtonEnabled("Duplicate",false);
                    buttonPanel1.setButtonEnabled("Modify",false);
                    buttonPanel1.setButtonEnabled("State History",false);
                    buttonPanel1.setButtonEnabled("Change Status",false);
               }break;
            case "Components":
                if (ComponentsPanel.getSelectedRowCount() > 1) {
                    multipleSelection=true;
                } else {
                    multipleSelection=false;
                }
                if (componentID > 0 ) {
                    if (multipleSelection) {
                        buttonPanel1.setButtonEnabled("Modify",false);
                        buttonPanel1.setButtonEnabled("Query Panel",false);
                        buttonPanel1.setButtonEnabled("New",false);
                        buttonPanel1.setButtonEnabled("Refresh",false);
                        buttonPanel1.setButtonEnabled("Build TemplateTree",false);
                    } else {
                        buttonPanel1.setButtonEnabled("Modify",true);
                        buttonPanel1.setButtonEnabled("Query Panel",true);
                        buttonPanel1.setButtonEnabled("New",true);
                        buttonPanel1.setButtonEnabled("Refresh",true);
                        buttonPanel1.setButtonEnabled("Build TemplateTree",true);
                        try {
                            if (OtdbRmi.getRemoteMaintenance().isTopComponent(componentID)) {
                                buttonPanel1.setButtonEnabled("Build TemplateTree",true);
                            } else {
                                buttonPanel1.setButtonEnabled("Build TemplateTree",false);
                            }
                        } catch (RemoteException ex) {
                            String aS="Error checking isTopComponent";
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    }
                    buttonPanel1.setButtonEnabled("Delete",true);
                } else {
                    buttonPanel1.setButtonEnabled("Delete",false);
                    buttonPanel1.setButtonEnabled("Modify",false);
                    buttonPanel1.setButtonEnabled("Query Panel",false);
                    buttonPanel1.setButtonEnabled("New",false);
                    buttonPanel1.setButtonEnabled("Refresh",false);
                    buttonPanel1.setButtonEnabled("Build TemplateTree",false);
                }
                break;
            case "Query Results":
                break;
        }
    }
    
    private MainFrame                   itsMainFrame = null;
    private String                      itsTabFocus="PIC";
    private boolean                     buttonsInitialized=false;
    private LoadFileDialog              loadFileDialog = null;
    private TreeInfoDialog              treeInfoDialog = null;
    private MultiEditDialog             multiEditDialog = null;
    private CreateDefaultTemplateDialog defaultTemplateDialog = null;
    private TableDialog                 stateChangeHistoryDialog = null;
    private StateChangeHistoryTableModel itsStateChangeModel = null;
    private boolean                     changed=false;
    private boolean                     multipleSelection=false;
    
    // File to be loaded info
    File itsNewFile=null;
    String itsFileDescription="";
    String itsFileStatus = "";
            
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel AdminPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel ComponentsPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel DefaultTemplatesPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel PICPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel QueryResultsPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel TemplatesPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel VICPanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JTabbedPane jTabbedPane1;
    // End of variables declaration//GEN-END:variables
    
}
