/*
 * OlapPanel.java
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
 */

package nl.astron.lofar.sas.otbcomponents.userpanels;

import java.awt.Component;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * Panel for OLAP specific configuration
 *
 * @author  Coolen
 *
 * Created on 19 april 2007, 20:54
 *
 * @version $Id$
 */
public class OlapPanel extends javax.swing.JPanel implements IViewPanel{

    static Logger logger = Logger.getLogger(OlapPanel.class);    
    static String name = "OlapPanel";
    
    /** Creates new form OlapPanel */
    public OlapPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public OlapPanel() {
        initComponents();
        initialize();
    }   
    
    @Override
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    @Override
    public String getShortName() {
        return name;
    }
    
    @Override
    public void setContent(Object anObject) { 
        itsNode=(jOTDBnode)anObject;
        jOTDBparam aParam=null;



        try {
           
            //we need to get all the childs from this node.    
            Vector<jOTDBnode> childs = OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1);
            
            // get all the params per child
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                aParam=null;
            
                jOTDBnode aNode = (jOTDBnode)e.nextElement();
                        
                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode,aParam,aNode);
                    
                                    
                //we need to get all the childs from the following nodes as well.
                }else if (LofarUtils.keyName(aNode.name).equals("CNProc")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("IONProc")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("StorageProc")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Correlator")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("PencilInfo")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("CNProc_CoherentStokes")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("CNProc_IncoherentStokes")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }
            //we also need the Virtual Instrument Storagenodes here
            Vector<jOTDBnode> VIchilds = OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(),"%VirtualInstrument");

            // get all the params per child
            Enumeration eVI = VIchilds.elements();
            while( eVI.hasMoreElements()  ) {
                aParam=null;

                jOTDBnode aNode = (jOTDBnode)eVI.nextElement();

                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode,aParam,aNode);
                }else if (LofarUtils.keyName(aNode.name).equals("VirtualInstrument")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }

            //we also need the Output_ nodes from the Dataproducts here
            Vector<jOTDBnode> OUchilds = OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(),"%Output_%");

            // get all the params per child
            Enumeration eOU = OUchilds.elements();
            while( eOU.hasMoreElements()  ) {
                aParam=null;

                jOTDBnode aNode = (jOTDBnode)eOU.nextElement();

                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode,aParam,aNode);
                }else if (LofarUtils.keyName(aNode.name).equals("Output_Beamformed")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }else if (LofarUtils.keyName(aNode.name).equals("Output_CoherentStokes")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }else if (LofarUtils.keyName(aNode.name).equals("Output_Correlated")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }else if (LofarUtils.keyName(aNode.name).equals("Output_Filtered")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }else if (LofarUtils.keyName(aNode.name).equals("Output_IncoherentStokes")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }

        } catch (RemoteException ex) {
            String aS="Error during getComponentParam: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }
        
        initPanel();
    }
    
    @Override
    public boolean isSingleton() {
        return false;
    }
    
    @Override
    public JPanel getInstance() {
        return new OlapPanel();
    }
    @Override
    public boolean hasPopupMenu() {
        return true;
    }    
    
    /** create popup menu for this panel
     *
     *  // build up the menu
     *  aPopupMenu= new JPopupMenu();
     *  aMenuItem=new JMenuItem("Choice 1");        
     *  aMenuItem.addActionListener(new java.awt.event.ActionListener() {
     *      public void actionPerformed(java.awt.event.ActionEvent evt) {
     *          popupMenuHandler(evt);
     *      }
     *  });
     *  aMenuItem.setActionCommand("Choice 1");
     *  aPopupMenu.add(aMenuItem);
     *  aPopupMenu.setOpaque(true);
     *
     *
     *  aPopupMenu.show(aComponent, x, y );        
     */
    @Override
    public void createPopupMenu(Component aComponent,int x, int y) {
        JPopupMenu aPopupMenu=null;
        JMenuItem  aMenuItem=null;
        
        aPopupMenu= new JPopupMenu();
        // For VIC trees
        if (itsTreeType.equals("VHtree")) {
            //  Fill in menu as in the example above
            aMenuItem=new JMenuItem("Create ParSet File");        
            aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                @Override
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    popupMenuHandler(evt);
                }
            });
            aMenuItem.setActionCommand("Create ParSet File");
            aPopupMenu.add(aMenuItem);
            
        // For template trees
        } else if (itsTreeType.equals("VItemplate")) {
                
        }
        
        aPopupMenu.setOpaque(true);
        aPopupMenu.show(aComponent, x, y );       
    }
    
    /** handles the choice from the popupmenu 
     *
     * depending on the choices that are possible for this panel perform the action for it
     *
     *      if (evt.getActionCommand().equals("Choice 1")) {
     *          perform action
     *      }  
     */
    @Override
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
         if (evt.getActionCommand().equals("Create ParSet File")) {
            logger.trace("Create ParSet File");
            int aTreeID=itsMainFrame.getSharedVars().getTreeID();
            if (fc == null) {
                fc = new JFileChooser();
                fc.setApproveButtonText("Apply");
            }
            // try to get a new filename to write the parsetfile to
            if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
                try {
                    File aFile = fc.getSelectedFile();
                    
                    // create filename that can be used at the remote site    
                    String aRemoteFileName="/tmp/"+aTreeID+"-"+itsNode.name+"_"+itsMainFrame.getUserAccount().getUserName()+".ParSet";
                    
                    // write the parset
                    OtdbRmi.getRemoteMaintenance().exportTree(aTreeID,itsNode.nodeID(),aRemoteFileName,2,false); 
                    
                    //obtain the remote file
                    byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);

                    BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile));
                    output.write(dldata,0,dldata.length);
                    output.flush();
                    output.close();
                    logger.trace("File written to: " + aFile.getPath());
                } catch (RemoteException ex) {
                    String aS="exportTree failed : " + ex;
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
        }       
    } 
    
    /** 
     * Helper method that retrieves the child nodes for a given jOTDBnode, 
     * and triggers setField() accordingly.
     * @param aNode the node to retrieve and display child data of.
     */
    private void retrieveAndDisplayChildDataForNode(jOTDBnode aNode){
        jOTDBparam aParam=null;
        try {
            Vector<jOTDBnode> HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            // get all the params per child
            Enumeration e1 = HWchilds.elements();
            while( e1.hasMoreElements()  ) {
                
                jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                aParam=null;
                // We need to keep all the params needed by this panel
                if (aHWNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aHWNode);
                }
                setField(aNode,aParam,aHWNode);
            }
        } catch (RemoteException ex) {
            String aS="Error during retrieveAndDisplayChildDataForNode: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }
    }


    
    /**
     * Sets the different fields in the GUI, using the names of the nodes provided
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void setField(jOTDBnode parent,jOTDBparam aParam, jOTDBnode aNode) {
        if (aParam==null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);
        String parentName = LofarUtils.keyName(String.valueOf(parent.name));
        /* Set's the different fields in the GUI */

        // Generic OLAP
        if (aParam==null) {
            return;
        }
        logger.debug("setField for: "+ aNode.name);
        try {
            if (OtdbRmi.getRemoteTypes().getParamType(aParam.type).substring(0,1).equals("p")) {
               // Have to get new param because we need the unresolved limits field.
               aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode.treeID(),aNode.paramDefID());                
            }
        } catch (RemoteException ex) {
            String aS="Error during getParam: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }
        
        if(parentName.equals("CNProc")){
            // OLAP-CNProc params
            if (aKeyName.equals("nrPPFTaps")) {
                inputNrPPFTaps.setToolTipText(aParam.description);
                itsNrPPFTaps=aNode;
                if (isRef && aParam != null) {
                    inputNrPPFTaps.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrPPFTaps.setText(aNode.limits);
                }
            }
            
        } else if(parentName.equals("Correlator")){
            // OLAP Correlator params

            if (aKeyName.equals("integrationTime")) {
                inputIntegrationTime.setToolTipText(aParam.description);
                itsIntegrationTime=aNode;
                if (isRef && aParam != null) {
                    inputIntegrationTime.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputIntegrationTime.setText(aNode.limits);
                }
            }
        } else if(parentName.equals("PencilInfo")){
            // OLAP PencilInfo params

            if (aKeyName.equals("flysEye")) {
                inputFlysEye.setToolTipText(aParam.description);
                itsFlysEye=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputFlysEye.setSelected(aSelection);
                checkSettings();
            }
        } else if(parentName.equals("CNProc_CoherentStokes")){
            // OLAP PencilInfo params

            if (aKeyName.equals("which")) {
                inputWhichCoherent.setToolTipText(aParam.description);
                LofarUtils.setPopupComboChoices(inputWhichCoherent,aParam.limits);
                if (!aNode.limits.equals("")) {
                    inputWhichCoherent.setSelectedItem(aNode.limits);
                }
                itsWhichCoherent=aNode;
            } else if (aKeyName.equals("integrateChannels")) {
                inputIntegrateChannelsCoherent.setToolTipText(aParam.description);
                itsIntegrateChannelsCoherent=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputIntegrateChannelsCoherent.setSelected(aSelection);

            } else if (aKeyName.equals("integrationSteps")) {
                inputIntegrationStepsCoherent.setToolTipText(aParam.description);
                itsIntegrationStepsCoherent=aNode;
                if (isRef && aParam != null) {
                    inputIntegrationStepsCoherent.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputIntegrationStepsCoherent.setText(aNode.limits);
                }
            }

        } else if(parentName.equals("CNProc_IncoherentStokes")){
            // OLAP PencilInfo params

            if (aKeyName.equals("which")) {
                inputWhichIncoherent.setToolTipText(aParam.description);
                LofarUtils.setPopupComboChoices(inputWhichIncoherent,aParam.limits);
                if (!aNode.limits.equals("")) {
                    inputWhichIncoherent.setSelectedItem(aNode.limits);
                }
                itsWhichIncoherent=aNode;
            } else if (aKeyName.equals("integrateChannels")) {
                inputIntegrateChannelsIncoherent.setToolTipText(aParam.description);
                itsIntegrateChannelsIncoherent=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputIntegrateChannelsIncoherent.setSelected(aSelection);

            } else if (aKeyName.equals("integrationSteps")) {
                inputIntegrationStepsIncoherent.setToolTipText(aParam.description);
                itsIntegrationStepsIncoherent=aNode;
                if (isRef && aParam != null) {
                    inputIntegrationStepsIncoherent.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputIntegrationStepsIncoherent.setText(aNode.limits);
                }
            }
        } else if(parentName.equals("StorageProc")){
            // OLAP StorageProc params
            if (aKeyName.equals("subbandsPerMS")) {
                inputSubbandsPerMS.setToolTipText(aParam.description);
                itsSubbandsPerMS=aNode;
                if (isRef && aParam != null) {
                    inputSubbandsPerMS.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputSubbandsPerMS.setText(aNode.limits);
                }
            }
        } else if(parentName.equals("OLAP")){
            // Olap Specific parameters
            if (aKeyName.equals("maxNetworkDelay")) {
                inputMaxNetworkDelay.setToolTipText(aParam.description);
                itsMaxNetworkDelay=aNode;
                if (isRef && aParam != null) {
                    inputMaxNetworkDelay.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputMaxNetworkDelay.setText(aNode.limits);
                }
            } else if (aKeyName.equals("nrSubbandsPerFrame")) {
                inputNrSubbandsPerFrame.setToolTipText(aParam.description);
                itsNrSubbandsPerFrame=aNode;
                if (isRef && aParam != null) {
                    inputNrSubbandsPerFrame.setText(aNode.limits);
                    subbandsPerFrameDerefText.setText(aParam.limits);
                } else {
                    inputNrSubbandsPerFrame.setText(aNode.limits);
                }
            } else if (aKeyName.equals("delayCompensation")) {
                inputDelayCompensation.setToolTipText(aParam.description);
                itsDelayCompensation=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputDelayCompensation.setSelected(aSelection);
            } else if (aKeyName.equals("nrBitsPerSample")) {
                inputNrBitsPerSample.setToolTipText(aParam.description);
                itsNrBitsPerSample=aNode;
                if (isRef && aParam != null) {
                    inputNrBitsPerSample.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrBitsPerSample.setText(aNode.limits);
                }
            } else if (aKeyName.equals("nrSecondsOfBuffer")) {
                inputNrSecondsOfBuffer.setToolTipText(aParam.description);
                itsNrSecondsOfBuffer=aNode;
                if (isRef && aParam != null) {
                    inputNrSecondsOfBuffer.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrSecondsOfBuffer.setText(aNode.limits);
                }
            } else if (aKeyName.equals("nrTimesInFrame")) {
                inputNrTimesInFrame.setToolTipText(aParam.description);
                itsNrTimesInFrame=aNode;
                if (isRef && aParam != null) {
                    inputNrTimesInFrame.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrTimesInFrame.setText(aNode.limits);
                }

            } else if (aKeyName.equals("correctBandPass")) {
                inputCorrectBandPass.setToolTipText(aParam.description);
                itsCorrectBandPass=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputCorrectBandPass.setSelected(aSelection);
            }
        } else if (parentName.equals("Output_Beamformed")) {
            if (aKeyName.equals("enabled")) {
                inputOutputBeamFormedData.setToolTipText(aParam.description);
                itsOutputBeamFormedData=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputOutputBeamFormedData.setSelected(aSelection);
                checkSettings();
            }
        } else if (parentName.equals("Output_Correlated")) {
            if (aKeyName.equals("enabled")) {
                inputOutputCorrelatedData.setToolTipText(aParam.description);
                itsOutputCorrelatedData=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputOutputCorrelatedData.setSelected(aSelection);
                inputIntegrationTime.setEnabled(aSelection);
                checkSettings();
            }
        } else if (parentName.equals("Output_Filtered")) {
            if (aKeyName.equals("enabled")) {
                inputOutputFilteredData.setToolTipText(aParam.description);
                itsOutputFilteredData=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputOutputFilteredData.setSelected(aSelection);
                checkSettings();
            }
        } else if (parentName.equals("Output_CoherentStokes")) {
            if (aKeyName.equals("enabled")) {
                inputOutputCoherentStokes.setToolTipText(aParam.description);
                itsOutputCoherentStokes=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputOutputCoherentStokes.setSelected(aSelection);
                checkSettings();
            }
        } else if (parentName.equals("Output_IncoherentStokes")) {
            if (aKeyName.equals("enabled")) {
                inputOutputIncoherentStokes.setToolTipText(aParam.description);
                itsOutputIncoherentStokes=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputOutputIncoherentStokes.setSelected(aSelection);
                checkSettings();
            }
        }
    }

    // check all settings to make a choice about enabled/disables fields
    private void checkSettings() {
        if (inputOutputCorrelatedData.isSelected() ) {
            inputIntegrationTime.setEnabled(true);
        } else {
            inputIntegrationTime.setEnabled(false);
        }


        if (inputOutputBeamFormedData.isSelected() || inputOutputCoherentStokes.isSelected()) {
            if (inputOutputBeamFormedData.isSelected()) {
                inputOutputCoherentStokes.setSelected(false);
                inputOutputCoherentStokes.setEnabled(false);
            } else if (inputOutputCoherentStokes.isSelected()) {
                inputOutputBeamFormedData.setSelected(false);
                inputOutputBeamFormedData.setEnabled(false);
            }
            inputFlysEye.setEnabled(true);

        } else {
            inputFlysEye.setEnabled(false);
        }

        if (inputOutputCoherentStokes.isSelected() ) { 
            inputWhichCoherent.setEnabled(true);
            inputIntegrateChannelsCoherent.setEnabled(true);
            inputIntegrationStepsCoherent.setEnabled(true);
        } else {
            inputWhichCoherent.setEnabled(false);
            inputIntegrateChannelsCoherent.setEnabled(false);
            inputIntegrationStepsCoherent.setEnabled(false);
        }
        if (inputOutputIncoherentStokes.isSelected() ) { 
            inputWhichIncoherent.setEnabled(true);
            inputIntegrateChannelsIncoherent.setEnabled(true);
            inputIntegrationStepsIncoherent.setEnabled(true);
        } else {
            inputWhichIncoherent.setEnabled(false);
            inputIntegrateChannelsIncoherent.setEnabled(false);
            inputIntegrationStepsIncoherent.setEnabled(false);
        }

    }
    
    private void restore() {
      boolean aB=false;

      // Olap Specific parameters
      if (itsDelayCompensation!=null) {
        aB=false;
        if (itsDelayCompensation.limits.equals("true")||itsDelayCompensation.limits.equals("TRUE")) {
            aB=true;
        }
        inputDelayCompensation.setSelected(aB);
      }
      if (itsNrBitsPerSample!=null) {
        inputNrBitsPerSample.setText(itsNrBitsPerSample.limits);
      }
      if (itsNrSecondsOfBuffer!=null) {
        inputNrSecondsOfBuffer.setText(itsNrSecondsOfBuffer.limits);
      }
      if (itsNrTimesInFrame!=null) {
        inputNrTimesInFrame.setText(itsNrTimesInFrame.limits);
      }
      if (itsCorrectBandPass!=null) {
        aB=false;
        if (itsCorrectBandPass.limits.equals("true")||itsCorrectBandPass.limits.equals("TRUE")) {
          aB=true;
        }
        inputCorrectBandPass.setSelected(aB);
      }
      if (itsOutputCorrelatedData!=null) {
        aB=false;
        if (itsOutputCorrelatedData.limits.equals("true")||itsOutputCorrelatedData.limits.equals("TRUE")) {
          aB=true;
        }
        inputOutputCorrelatedData.setSelected(aB);
      }
      if (itsOutputFilteredData!=null) {
        aB=false;
        if (itsOutputFilteredData.limits.equals("true")||itsOutputFilteredData.limits.equals("TRUE")) {
          aB=true;
        }
        inputOutputFilteredData.setSelected(aB);
      }
      if (itsOutputBeamFormedData!=null) {
        aB=false;
        if (itsOutputBeamFormedData.limits.equals("true")||itsOutputBeamFormedData.limits.equals("TRUE")) {
          aB=true;
        }
        inputOutputBeamFormedData.setSelected(aB);
      }
      if (itsOutputCoherentStokes!=null) {
        aB=false;
        if (itsOutputCoherentStokes.limits.equals("true")||itsOutputCoherentStokes.limits.equals("TRUE")) {
          aB=true;
        }
        inputOutputCoherentStokes.setSelected(aB);
      }
      if (itsOutputIncoherentStokes!=null) {
        aB=false;
        if (itsOutputIncoherentStokes.limits.equals("true")||itsOutputIncoherentStokes.limits.equals("TRUE")) {
          aB=true;
        }
        inputOutputIncoherentStokes.setSelected(aB);
      }
      if (itsMaxNetworkDelay!=null) {
        inputMaxNetworkDelay.setText(itsMaxNetworkDelay.limits);
      }
      if (itsNrSubbandsPerFrame!=null) {
        inputNrSubbandsPerFrame.setText(itsNrSubbandsPerFrame.limits);
      }

      //OLAP-CNProc
      if (itsNrPPFTaps!=null) {
        inputNrPPFTaps.setText(itsNrPPFTaps.limits);
      }
      
      
      //OLAP StorageProc
      if (itsSubbandsPerMS!=null) {
        inputSubbandsPerMS.setText(itsSubbandsPerMS.limits);
      }

      // Correlator
      if (itsIntegrationTime!=null) {
        inputIntegrationTime.setText(itsIntegrationTime.limits);
      }

      // PencilInfo
      if (itsFlysEye!=null) {
        aB=false;
        if (itsFlysEye.limits.equals("true")||itsFlysEye.limits.equals("TRUE")) {
          aB=true;
        }
        inputFlysEye.setSelected(aB);
      }


      // CNProc_CoherentStokes
      if(itsWhichCoherent!=null) {
        inputWhichCoherent.setSelectedItem(itsWhichCoherent.limits);
      }
      if (itsIntegrateChannelsCoherent!=null) {
        aB=false;
        if (itsIntegrateChannelsCoherent.limits.equals("true")||itsIntegrateChannelsCoherent.limits.equals("TRUE")) {
          aB=true;
        }
        inputIntegrateChannelsCoherent.setSelected(aB);
      }
      if (itsIntegrationStepsCoherent!=null) {
        inputIntegrationStepsCoherent.setText(itsIntegrationStepsCoherent.limits);
      }

      // CNProc_IncoherentStokes
      if(itsWhichIncoherent!=null) {
        inputWhichIncoherent.setSelectedItem(itsWhichIncoherent.limits);
      }
      if (itsIntegrateChannelsIncoherent!=null) {
        aB=false;
        if (itsIntegrateChannelsIncoherent.limits.equals("true")||itsIntegrateChannelsIncoherent.limits.equals("TRUE")) {
          aB=true;
        }
        inputIntegrateChannelsIncoherent.setSelected(aB);
      }
      if (itsIntegrationStepsIncoherent!=null) {
        inputIntegrationStepsIncoherent.setText(itsIntegrationStepsIncoherent.limits);
      }

     
      checkSettings();
    }
     
    private void initialize() {
        buttonPanel1.addButton("Restore");
        buttonPanel1.setButtonIcon("Restore",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_undo.png")));
        buttonPanel1.addButton("Apply");
        buttonPanel1.setButtonIcon("Apply",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png")));
   }
    
    private void initPanel() {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();

        // for now:
        setAllEnabled(true);
        
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }
        

         if (itsNode != null) {
            try {
                //figure out the caller
                jOTDBtree aTree = OtdbRmi.getRemoteOTDB().getTreeInfo(itsNode.treeID(),false);
                itsTreeType=OtdbRmi.getTreeType().get(aTree.type);
            } catch (RemoteException ex) {
                String aS="OlapPanel: Error getting treeInfo/treetype" + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                itsTreeType="";
            }
         } else {
            logger.error("no node given");
        }

        restore();
    }   
    
 
    /** saves the given node back to the database
     */
    private void saveNode(jOTDBnode aNode) {
        if (aNode == null) {
            return;
        }
        try {
            OtdbRmi.getRemoteMaintenance().saveNode(aNode); 
        } catch (RemoteException ex) {
            String aS="Error: saveNode failed : " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        } 
    }   
    
   
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    @Override
    public void enableButtons(boolean enabled) {

        // always do this last to keep up with panel settings regardless of user settings
        checkSettings();
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    @Override
    public void setButtonsVisible(boolean visible) {
    }        
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    @Override
    public void setAllEnabled(boolean enabled) {

        // always do this last to keep up with panel settings regardless of user settings
        checkSettings();
    }
    
    private boolean saveInput() {
        boolean hasChanged = false;

        // Generic OLAP       
        if ((!inputDelayCompensation.isSelected() &  
                (itsDelayCompensation.limits.equals("TRUE") ||itsDelayCompensation.limits.equals("true") )) ||
            (inputDelayCompensation.isSelected() &  
                (itsDelayCompensation.limits.equals("FALSE") ||itsDelayCompensation.limits.equals("false") )))        
        {  
            String delay="true";
            if (!inputDelayCompensation.isSelected()) {
                delay="false";
            }
            itsDelayCompensation.limits = delay;
            saveNode(itsDelayCompensation);
        }
        if (itsMaxNetworkDelay != null && !inputMaxNetworkDelay.getText().equals(itsMaxNetworkDelay.limits)) {
            itsMaxNetworkDelay.limits = inputMaxNetworkDelay.getText();
            saveNode(itsMaxNetworkDelay);
        }
        if (itsNrSubbandsPerFrame != null && !inputNrSubbandsPerFrame.getText().equals(itsNrSubbandsPerFrame.limits)) {
            itsNrSubbandsPerFrame.limits = inputNrSubbandsPerFrame.getText();
            saveNode(itsNrSubbandsPerFrame);
        }      
        if (itsNrBitsPerSample != null && !inputNrBitsPerSample.getText().equals(itsNrBitsPerSample.limits)) {
            itsNrBitsPerSample.limits = inputNrBitsPerSample.getText();
            saveNode(itsNrBitsPerSample);
        }
        if (itsNrSecondsOfBuffer != null && !inputNrSecondsOfBuffer.getText().equals(itsNrSecondsOfBuffer.limits)) {
            itsNrSecondsOfBuffer.limits = inputNrSecondsOfBuffer.getText();
            saveNode(itsNrSecondsOfBuffer);
        }
        if (itsNrTimesInFrame != null && !inputNrTimesInFrame.getText().equals(itsNrTimesInFrame.limits)) {
            itsNrTimesInFrame.limits = inputNrTimesInFrame.getText();
            saveNode(itsNrTimesInFrame);
        }
        if ((!inputCorrectBandPass.isSelected() &  
                (itsCorrectBandPass.limits.equals("TRUE") ||itsCorrectBandPass.limits.equals("true") )) ||
            (inputCorrectBandPass.isSelected() &  
                (itsCorrectBandPass.limits.equals("FALSE") ||itsCorrectBandPass.limits.equals("false") )))        
        {  
            String bp="true";
            if (!inputCorrectBandPass.isSelected()) {
                bp="false";
            }
            itsCorrectBandPass.limits = bp;
            saveNode(itsCorrectBandPass);
        }

        if ((!inputOutputCorrelatedData.isSelected() &
                (itsOutputCorrelatedData.limits.equals("TRUE") ||itsOutputCorrelatedData.limits.equals("true") )) ||
            (inputOutputCorrelatedData.isSelected() &
                (itsOutputCorrelatedData.limits.equals("FALSE") ||itsOutputCorrelatedData.limits.equals("false") )))
        {
            String bp="true";
            if (!inputOutputCorrelatedData.isSelected()) {
                bp="false";
            }
            itsOutputCorrelatedData.limits = bp;
            saveNode(itsOutputCorrelatedData);
        }
        if ((!inputOutputFilteredData.isSelected() &
                (itsOutputFilteredData.limits.equals("TRUE") ||itsOutputFilteredData.limits.equals("true") )) ||
            (inputOutputFilteredData.isSelected() &
                (itsOutputFilteredData.limits.equals("FALSE") ||itsOutputFilteredData.limits.equals("false") )))
        {
            String bp="true";
            if (!inputOutputFilteredData.isSelected()) {
                bp="false";
            }
            itsOutputFilteredData.limits = bp;
            saveNode(itsOutputFilteredData);
        }
        if ((!inputOutputBeamFormedData.isSelected() &
                (itsOutputBeamFormedData.limits.equals("TRUE") ||itsOutputBeamFormedData.limits.equals("true") )) ||
            (inputOutputBeamFormedData.isSelected() &
                (itsOutputBeamFormedData.limits.equals("FALSE") ||itsOutputBeamFormedData.limits.equals("false") )))
        {
            String bp="true";
            if (!inputOutputBeamFormedData.isSelected()) {
                bp="false";
            }
            itsOutputBeamFormedData.limits = bp;
            saveNode(itsOutputBeamFormedData);
        }
        if ((!inputOutputCoherentStokes.isSelected() &
                (itsOutputCoherentStokes.limits.equals("TRUE") ||itsOutputCoherentStokes.limits.equals("true") )) ||
            (inputOutputCoherentStokes.isSelected() &
                (itsOutputCoherentStokes.limits.equals("FALSE") ||itsOutputCoherentStokes.limits.equals("false") )))
        {
            String bp="true";
            if (!inputOutputCoherentStokes.isSelected()) {
                bp="false";
            }
            itsOutputCoherentStokes.limits = bp;
            saveNode(itsOutputCoherentStokes);
        }
        if ((!inputOutputIncoherentStokes.isSelected() &
                (itsOutputIncoherentStokes.limits.equals("TRUE") ||itsOutputIncoherentStokes.limits.equals("true") )) ||
            (inputOutputIncoherentStokes.isSelected() &
                (itsOutputIncoherentStokes.limits.equals("FALSE") ||itsOutputIncoherentStokes.limits.equals("false") )))
        {
            String bp="true";
            if (!inputOutputIncoherentStokes.isSelected()) {
                bp="false";
            }
            itsOutputIncoherentStokes.limits = bp;
            saveNode(itsOutputIncoherentStokes);
        }

        
        // OLAP-CNProc
        if (itsNrPPFTaps != null && !inputNrPPFTaps.getText().equals(itsNrPPFTaps.limits)) {
            itsNrPPFTaps.limits = inputNrPPFTaps.getText();
            saveNode(itsNrPPFTaps);
        }


        //OLAP-StorageProc
        if (itsSubbandsPerMS != null && !inputSubbandsPerMS.getText().equals(itsSubbandsPerMS.limits)) {
            itsSubbandsPerMS.limits = inputSubbandsPerMS.getText();
            saveNode(itsSubbandsPerMS);
        }

        // Correlator
        if (itsIntegrationTime != null && !inputIntegrationTime.getText().equals(itsIntegrationTime.limits)) {
            itsIntegrationTime.limits = inputIntegrationTime.getText();
            saveNode(itsIntegrationTime);
        }

        // PencilInfo
        if ((!inputFlysEye.isSelected() &
                (itsFlysEye.limits.equals("TRUE") ||itsFlysEye.limits.equals("true") )) ||
            (inputFlysEye.isSelected() &
                (itsFlysEye.limits.equals("FALSE") ||itsFlysEye.limits.equals("false") )))
        {
            String rt="true";
            if (!inputFlysEye.isSelected()) {
                rt="false";
            }
            itsFlysEye.limits = rt;
            saveNode(itsFlysEye);
        }

        // CNProc_CoherentStokes
        if (itsWhichCoherent!= null && !inputWhichCoherent.getSelectedItem().toString().equals(itsWhichCoherent.limits)) {
            itsWhichCoherent.limits = inputWhichCoherent.getSelectedItem().toString();
            saveNode(itsWhichCoherent);
        }

        if (itsIntegrationStepsCoherent != null && !inputIntegrationStepsCoherent.getText().equals(itsIntegrationStepsCoherent.limits)) {
            itsIntegrationStepsCoherent.limits = inputIntegrationStepsCoherent.getText();
            saveNode(itsIntegrationStepsCoherent);
        }
        if ((!inputIntegrateChannelsCoherent.isSelected() &
                (itsIntegrateChannelsCoherent.limits.equals("TRUE") ||itsIntegrateChannelsCoherent.limits.equals("true") )) ||
            (inputIntegrateChannelsCoherent.isSelected() &
                (itsIntegrateChannelsCoherent.limits.equals("FALSE") ||itsIntegrateChannelsCoherent.limits.equals("false") )))
        {
            String rt="true";
            if (!inputIntegrateChannelsCoherent.isSelected()) {
                rt="false";
            }
            itsIntegrateChannelsCoherent.limits = rt;
            saveNode(itsIntegrateChannelsCoherent);
        }

        // CNProc_IncoherentStokes
        if (itsWhichIncoherent!= null && !inputWhichIncoherent.getSelectedItem().toString().equals(itsWhichIncoherent.limits)) {
            itsWhichIncoherent.limits = inputWhichIncoherent.getSelectedItem().toString();
            saveNode(itsWhichIncoherent);
        }

        if (itsIntegrationStepsIncoherent != null && !inputIntegrationStepsIncoherent.getText().equals(itsIntegrationStepsIncoherent.limits)) {
            itsIntegrationStepsIncoherent.limits = inputIntegrationStepsIncoherent.getText();
            saveNode(itsIntegrationStepsIncoherent);
        }
        if ((!inputIntegrateChannelsIncoherent.isSelected() &
                (itsIntegrateChannelsIncoherent.limits.equals("TRUE") ||itsIntegrateChannelsIncoherent.limits.equals("true") )) ||
            (inputIntegrateChannelsIncoherent.isSelected() &
                (itsIntegrateChannelsIncoherent.limits.equals("FALSE") ||itsIntegrateChannelsIncoherent.limits.equals("false") )))
        {
            String rt="true";
            if (!inputIntegrateChannelsIncoherent.isSelected()) {
                rt="false";
            }
            itsIntegrateChannelsIncoherent.limits = rt;
            saveNode(itsIntegrateChannelsIncoherent);
        }
        
        return true;
    }

    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel9 = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jPanel2 = new javax.swing.JPanel();
        jPanel4 = new javax.swing.JPanel();
        jPanel8 = new javax.swing.JPanel();
        labelNrBitsPerSample = new javax.swing.JLabel();
        inputNrBitsPerSample = new javax.swing.JTextField();
        labelNrTimesInFrame = new javax.swing.JLabel();
        inputNrTimesInFrame = new javax.swing.JTextField();
        labelDelayCompensation = new javax.swing.JLabel();
        inputDelayCompensation = new javax.swing.JCheckBox();
        javax.swing.JLabel labelNrSecondsOfBuffer = new javax.swing.JLabel();
        inputNrSecondsOfBuffer = new javax.swing.JTextField();
        labelMaxNetworkDelay = new javax.swing.JLabel();
        inputMaxNetworkDelay = new javax.swing.JTextField();
        labelNrSubbandsPerFrame = new javax.swing.JLabel();
        inputNrSubbandsPerFrame = new javax.swing.JTextField();
        inputCorrectBandPass = new javax.swing.JCheckBox();
        subbandsPerFrameDerefText = new javax.swing.JTextField();
        jLabel3 = new javax.swing.JLabel();
        inputOutputCorrelatedData = new javax.swing.JCheckBox();
        inputOutputFilteredData = new javax.swing.JCheckBox();
        inputOutputBeamFormedData = new javax.swing.JCheckBox();
        inputOutputCoherentStokes = new javax.swing.JCheckBox();
        inputOutputIncoherentStokes = new javax.swing.JCheckBox();
        jPanel7 = new javax.swing.JPanel();
        labelNrPPFTaps = new javax.swing.JLabel();
        inputNrPPFTaps = new javax.swing.JTextField();
        jPanel11 = new javax.swing.JPanel();
        labelSubbandsPerMs = new javax.swing.JLabel();
        inputSubbandsPerMS = new javax.swing.JTextField();
        CorrelatorPanel = new javax.swing.JPanel();
        labelIntegrationTime = new javax.swing.JLabel();
        inputIntegrationTime = new javax.swing.JTextField();
        jLabel2 = new javax.swing.JLabel();
        CoherentStokesPanel = new javax.swing.JPanel();
        labelWhichCoherent = new javax.swing.JLabel();
        inputWhichCoherent = new javax.swing.JComboBox();
        inputIntegrateChannelsCoherent = new javax.swing.JCheckBox();
        labelIntegrationstepsCoherent = new javax.swing.JLabel();
        inputIntegrationStepsCoherent = new javax.swing.JTextField();
        IncoherentStokesPanel = new javax.swing.JPanel();
        labelWhichIncoherent = new javax.swing.JLabel();
        inputWhichIncoherent = new javax.swing.JComboBox();
        inputIntegrateChannelsIncoherent = new javax.swing.JCheckBox();
        labelIntegrationstepsIncoherent = new javax.swing.JLabel();
        inputIntegrationStepsIncoherent = new javax.swing.JTextField();
        PencilInfoPanel = new javax.swing.JPanel();
        inputFlysEye = new javax.swing.JCheckBox();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        javax.swing.GroupLayout jPanel9Layout = new javax.swing.GroupLayout(jPanel9);
        jPanel9.setLayout(jPanel9Layout);
        jPanel9Layout.setHorizontalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 100, Short.MAX_VALUE)
        );
        jPanel9Layout.setVerticalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 100, Short.MAX_VALUE)
        );

        setMinimumSize(new java.awt.Dimension(800, 600));
        setLayout(new java.awt.BorderLayout());

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(""));
        jPanel1.setPreferredSize(new java.awt.Dimension(100, 25));
        jPanel1.setLayout(new java.awt.BorderLayout());

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11)); // NOI18N
        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("OLAP Details");
        jPanel1.add(jLabel1, java.awt.BorderLayout.CENTER);

        add(jPanel1, java.awt.BorderLayout.NORTH);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(""));
        jPanel2.setPreferredSize(new java.awt.Dimension(5240, 3000));

        jPanel8.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Olap", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N

        labelNrBitsPerSample.setText("# Bits per Sample:");

        labelNrTimesInFrame.setText("# Times In Frame:");

        labelDelayCompensation.setText("Delay Compensation:");

        inputDelayCompensation.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputDelayCompensation.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelNrSecondsOfBuffer.setText("# SecondsOfBuffer:");

        labelMaxNetworkDelay.setText("Max Network Delay:");

        labelNrSubbandsPerFrame.setText("# SubbandsPerFrame:");

        inputNrSubbandsPerFrame.setToolTipText("");

        inputCorrectBandPass.setText("Correct BandPass");
        inputCorrectBandPass.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputCorrectBandPass.setMargin(new java.awt.Insets(0, 0, 0, 0));

        subbandsPerFrameDerefText.setEditable(false);
        subbandsPerFrameDerefText.setEnabled(false);

        jLabel3.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel3.setText("Output:");

        inputOutputCorrelatedData.setText("Correlated Data");
        inputOutputCorrelatedData.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputCorrelatedDataActionPerformed(evt);
            }
        });

        inputOutputFilteredData.setText("Filtered Data");

        inputOutputBeamFormedData.setText("Beamformed Data");
        inputOutputBeamFormedData.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputBeamFormedDataActionPerformed(evt);
            }
        });

        inputOutputCoherentStokes.setText("Coherent Stokes");
        inputOutputCoherentStokes.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputCoherentStokesActionPerformed(evt);
            }
        });

        inputOutputIncoherentStokes.setText("Incoherent Stokes");
        inputOutputIncoherentStokes.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputIncoherentStokesActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel8Layout = new javax.swing.GroupLayout(jPanel8);
        jPanel8.setLayout(jPanel8Layout);
        jPanel8Layout.setHorizontalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(labelNrSubbandsPerFrame, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelDelayCompensation, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelNrBitsPerSample, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelNrTimesInFrame, javax.swing.GroupLayout.DEFAULT_SIZE, 117, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(inputNrTimesInFrame, javax.swing.GroupLayout.PREFERRED_SIZE, 144, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                .addComponent(inputNrSubbandsPerFrame, javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(inputNrBitsPerSample, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 143, Short.MAX_VALUE)))
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(50, 50, 50)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                    .addComponent(labelMaxNetworkDelay, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                    .addComponent(labelNrSecondsOfBuffer, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 136, Short.MAX_VALUE)))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                .addComponent(subbandsPerFrameDerefText)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                .addComponent(inputNrSecondsOfBuffer, javax.swing.GroupLayout.PREFERRED_SIZE, 171, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addComponent(inputMaxNetworkDelay))
                            .addComponent(inputCorrectBandPass)))
                    .addComponent(inputDelayCompensation))
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGap(18, 18, 18)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(inputOutputCorrelatedData)
                            .addComponent(inputOutputFilteredData)
                            .addComponent(inputOutputBeamFormedData)
                            .addComponent(inputOutputCoherentStokes)
                            .addComponent(inputOutputIncoherentStokes)))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGap(43, 43, 43)
                        .addComponent(jLabel3)))
                .addContainerGap(127, Short.MAX_VALUE))
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE, false)
                            .addComponent(labelNrTimesInFrame)
                            .addComponent(inputNrTimesInFrame, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(labelNrSecondsOfBuffer)
                            .addComponent(inputNrSecondsOfBuffer, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE, false)
                            .addComponent(labelNrBitsPerSample)
                            .addComponent(inputMaxNetworkDelay, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(inputNrBitsPerSample, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(labelMaxNetworkDelay))
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addGroup(javax.swing.GroupLayout.Alignment.LEADING, jPanel8Layout.createSequentialGroup()
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGap(23, 23, 23)
                                        .addComponent(labelDelayCompensation))
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                            .addComponent(labelNrSubbandsPerFrame)
                                            .addComponent(inputNrSubbandsPerFrame, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                            .addComponent(subbandsPerFrameDerefText, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(inputCorrectBandPass, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))))
                            .addGroup(javax.swing.GroupLayout.Alignment.LEADING, jPanel8Layout.createSequentialGroup()
                                .addGap(31, 31, 31)
                                .addComponent(inputDelayCompensation, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(jLabel3)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputOutputCorrelatedData)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputOutputFilteredData)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputOutputBeamFormedData)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputOutputCoherentStokes)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputOutputIncoherentStokes)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel7.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "CN Proc", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        jPanel7.setToolTipText("BGLProc");

        labelNrPPFTaps.setText("# PPFTaps:");

        javax.swing.GroupLayout jPanel7Layout = new javax.swing.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addComponent(labelNrPPFTaps)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputNrPPFTaps, javax.swing.GroupLayout.PREFERRED_SIZE, 154, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(47, Short.MAX_VALUE))
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelNrPPFTaps)
                    .addComponent(inputNrPPFTaps, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(59, Short.MAX_VALUE))
        );

        jPanel11.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Storage Proc", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N

        labelSubbandsPerMs.setText("#subbands per MS:");

        javax.swing.GroupLayout jPanel11Layout = new javax.swing.GroupLayout(jPanel11);
        jPanel11.setLayout(jPanel11Layout);
        jPanel11Layout.setHorizontalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addComponent(labelSubbandsPerMs)
                .addGap(18, 18, 18)
                .addComponent(inputSubbandsPerMS, javax.swing.GroupLayout.PREFERRED_SIZE, 125, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(24, Short.MAX_VALUE))
        );
        jPanel11Layout.setVerticalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addGroup(jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelSubbandsPerMs)
                    .addComponent(inputSubbandsPerMS, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(52, Short.MAX_VALUE))
        );

        CorrelatorPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Correlator", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        CorrelatorPanel.setToolTipText("Correlator");

        labelIntegrationTime.setText("integration Time:");

        inputIntegrationTime.setEnabled(false);

        jLabel2.setText("sec");

        javax.swing.GroupLayout CorrelatorPanelLayout = new javax.swing.GroupLayout(CorrelatorPanel);
        CorrelatorPanel.setLayout(CorrelatorPanelLayout);
        CorrelatorPanelLayout.setHorizontalGroup(
            CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(CorrelatorPanelLayout.createSequentialGroup()
                .addComponent(labelIntegrationTime, javax.swing.GroupLayout.PREFERRED_SIZE, 143, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(24, 24, 24)
                .addComponent(inputIntegrationTime, javax.swing.GroupLayout.PREFERRED_SIZE, 54, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel2)
                .addContainerGap(28, Short.MAX_VALUE))
        );
        CorrelatorPanelLayout.setVerticalGroup(
            CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(CorrelatorPanelLayout.createSequentialGroup()
                .addGroup(CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIntegrationTime)
                    .addComponent(inputIntegrationTime, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel2))
                .addContainerGap(69, Short.MAX_VALUE))
        );

        CoherentStokesPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Coherent Stokes", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        CoherentStokesPanel.setToolTipText("Stokes");

        labelWhichCoherent.setText("which");

        inputWhichCoherent.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputWhichCoherent.setEnabled(false);

        inputIntegrateChannelsCoherent.setText("integrate Channels");
        inputIntegrateChannelsCoherent.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputIntegrateChannelsCoherent.setEnabled(false);
        inputIntegrateChannelsCoherent.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelIntegrationstepsCoherent.setText("integration Steps:");

        inputIntegrationStepsCoherent.setEnabled(false);

        javax.swing.GroupLayout CoherentStokesPanelLayout = new javax.swing.GroupLayout(CoherentStokesPanel);
        CoherentStokesPanel.setLayout(CoherentStokesPanelLayout);
        CoherentStokesPanelLayout.setHorizontalGroup(
            CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(CoherentStokesPanelLayout.createSequentialGroup()
                .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputIntegrateChannelsCoherent)
                    .addGroup(CoherentStokesPanelLayout.createSequentialGroup()
                        .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                            .addComponent(labelIntegrationstepsCoherent, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(labelWhichCoherent, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 103, Short.MAX_VALUE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(inputIntegrationStepsCoherent)
                            .addComponent(inputWhichCoherent, 0, 173, Short.MAX_VALUE))))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        CoherentStokesPanelLayout.setVerticalGroup(
            CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(CoherentStokesPanelLayout.createSequentialGroup()
                .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelWhichCoherent)
                    .addComponent(inputWhichCoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(8, 8, 8)
                .addGroup(CoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIntegrationstepsCoherent)
                    .addComponent(inputIntegrationStepsCoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(inputIntegrateChannelsCoherent, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        IncoherentStokesPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Incoherent Stokes", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        IncoherentStokesPanel.setToolTipText("Stokes");

        labelWhichIncoherent.setText("which");

        inputWhichIncoherent.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputWhichIncoherent.setEnabled(false);

        inputIntegrateChannelsIncoherent.setText("integrate Channels");
        inputIntegrateChannelsIncoherent.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputIntegrateChannelsIncoherent.setEnabled(false);
        inputIntegrateChannelsIncoherent.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelIntegrationstepsIncoherent.setText("integration Steps:");

        inputIntegrationStepsIncoherent.setEnabled(false);

        javax.swing.GroupLayout IncoherentStokesPanelLayout = new javax.swing.GroupLayout(IncoherentStokesPanel);
        IncoherentStokesPanel.setLayout(IncoherentStokesPanelLayout);
        IncoherentStokesPanelLayout.setHorizontalGroup(
            IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(IncoherentStokesPanelLayout.createSequentialGroup()
                .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputIntegrateChannelsIncoherent)
                    .addGroup(IncoherentStokesPanelLayout.createSequentialGroup()
                        .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                            .addComponent(labelIntegrationstepsIncoherent, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(labelWhichIncoherent, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 103, Short.MAX_VALUE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(inputIntegrationStepsIncoherent)
                            .addComponent(inputWhichIncoherent, 0, 173, Short.MAX_VALUE))))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        IncoherentStokesPanelLayout.setVerticalGroup(
            IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(IncoherentStokesPanelLayout.createSequentialGroup()
                .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelWhichIncoherent)
                    .addComponent(inputWhichIncoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(8, 8, 8)
                .addGroup(IncoherentStokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIntegrationstepsIncoherent)
                    .addComponent(inputIntegrationStepsIncoherent, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(inputIntegrateChannelsIncoherent, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        PencilInfoPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "PencilInfo", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N
        PencilInfoPanel.setToolTipText("PencilInfo");

        inputFlysEye.setText("Flyseye");
        inputFlysEye.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputFlysEye.setEnabled(false);
        inputFlysEye.setMargin(new java.awt.Insets(0, 0, 0, 0));
        inputFlysEye.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputFlysEyeActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout PencilInfoPanelLayout = new javax.swing.GroupLayout(PencilInfoPanel);
        PencilInfoPanel.setLayout(PencilInfoPanelLayout);
        PencilInfoPanelLayout.setHorizontalGroup(
            PencilInfoPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(PencilInfoPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(inputFlysEye)
                .addContainerGap(210, Short.MAX_VALUE))
        );
        PencilInfoPanelLayout.setVerticalGroup(
            PencilInfoPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(PencilInfoPanelLayout.createSequentialGroup()
                .addComponent(inputFlysEye, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(59, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addComponent(jPanel8, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.LEADING, jPanel4Layout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jPanel11, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(jPanel7, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(18, 18, 18)
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(PencilInfoPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(CorrelatorPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addGap(18, 18, 18)
                        .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(IncoherentStokesPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(CoherentStokesPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
                .addContainerGap(25, Short.MAX_VALUE))
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel8, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel4Layout.createSequentialGroup()
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 6, Short.MAX_VALUE)
                        .addComponent(CoherentStokesPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(CorrelatorPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel7, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(IncoherentStokesPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(PencilInfoPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel11, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addGap(24, 24, 24))
        );

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(4293, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(2564, Short.MAX_VALUE))
        );

        jScrollPane1.setViewportView(jPanel2);

        add(jScrollPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);
    }// </editor-fold>//GEN-END:initComponents

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand().equals("Apply")) {
            saveInput();
        } else if(evt.getActionCommand().equals("Restore")) {
            restore();
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed

    private void inputOutputCorrelatedDataActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputCorrelatedDataActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputOutputCorrelatedDataActionPerformed

    private void inputOutputBeamFormedDataActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputBeamFormedDataActionPerformed
        inputOutputCoherentStokes.setEnabled(!inputOutputBeamFormedData.isSelected());
        inputOutputCoherentStokes.setSelected(false);
        checkSettings();
    }//GEN-LAST:event_inputOutputBeamFormedDataActionPerformed

    private void inputOutputCoherentStokesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputCoherentStokesActionPerformed
        inputOutputBeamFormedData.setEnabled(!inputOutputCoherentStokes.isSelected());
        inputOutputBeamFormedData.setSelected(false);
        checkSettings();
    }//GEN-LAST:event_inputOutputCoherentStokesActionPerformed

    private void inputOutputIncoherentStokesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputIncoherentStokesActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputOutputIncoherentStokesActionPerformed

    private void inputFlysEyeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputFlysEyeActionPerformed
       checkSettings();
    }//GEN-LAST:event_inputFlysEyeActionPerformed
    
    private jOTDBnode    itsNode      = null;
    private MainFrame    itsMainFrame = null;
    private String       itsTreeType  = "";
    private JFileChooser fc           = null;  
    private boolean  editting = false;
    private int      itsSelectedRow = -1;

    //Olap specific parameters
    private jOTDBnode itsDelayCompensation=null;
    private jOTDBnode itsNrTimesInFrame=null;
    private jOTDBnode itsNrSubbandsPerFrame=null;
    private jOTDBnode itsNrBitsPerSample=null;
    private jOTDBnode itsNrSecondsOfBuffer=null;
    private jOTDBnode itsMaxNetworkDelay=null;
    private jOTDBnode itsCorrectBandPass=null;
    
    
    // _Output params
    private jOTDBnode itsOutputCorrelatedData=null;
    private jOTDBnode itsOutputFilteredData=null;
    private jOTDBnode itsOutputBeamFormedData=null;
    private jOTDBnode itsOutputCoherentStokes=null;
    private jOTDBnode itsOutputIncoherentStokes=null;

    
    // OLAP-CNProc parameters
    private jOTDBnode itsNrPPFTaps=null;
 
    // OLAP-StorageProc parameters
    private jOTDBnode itsSubbandsPerMS=null;

    //Correlator
    private jOTDBnode itsIntegrationTime=null;

    // CNProc_Coherentstokes
    private jOTDBnode itsWhichCoherent=null;
    private jOTDBnode itsIntegrateChannelsCoherent=null;
    private jOTDBnode itsIntegrationStepsCoherent=null;

    // CNProc_Incoherentstokes
    private jOTDBnode itsWhichIncoherent=null;
    private jOTDBnode itsIntegrateChannelsIncoherent=null;
    private jOTDBnode itsIntegrationStepsIncoherent=null;

    // PencilInfo
    private jOTDBnode itsFlysEye=null;


    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel CoherentStokesPanel;
    private javax.swing.JPanel CorrelatorPanel;
    private javax.swing.JPanel IncoherentStokesPanel;
    private javax.swing.JPanel PencilInfoPanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JCheckBox inputCorrectBandPass;
    private javax.swing.JCheckBox inputDelayCompensation;
    private javax.swing.JCheckBox inputFlysEye;
    private javax.swing.JCheckBox inputIntegrateChannelsCoherent;
    private javax.swing.JCheckBox inputIntegrateChannelsIncoherent;
    private javax.swing.JTextField inputIntegrationStepsCoherent;
    private javax.swing.JTextField inputIntegrationStepsIncoherent;
    private javax.swing.JTextField inputIntegrationTime;
    private javax.swing.JTextField inputMaxNetworkDelay;
    private javax.swing.JTextField inputNrBitsPerSample;
    private javax.swing.JTextField inputNrPPFTaps;
    private javax.swing.JTextField inputNrSecondsOfBuffer;
    private javax.swing.JTextField inputNrSubbandsPerFrame;
    private javax.swing.JTextField inputNrTimesInFrame;
    private javax.swing.JCheckBox inputOutputBeamFormedData;
    private javax.swing.JCheckBox inputOutputCoherentStokes;
    private javax.swing.JCheckBox inputOutputCorrelatedData;
    private javax.swing.JCheckBox inputOutputFilteredData;
    private javax.swing.JCheckBox inputOutputIncoherentStokes;
    private javax.swing.JTextField inputSubbandsPerMS;
    private javax.swing.JComboBox inputWhichCoherent;
    private javax.swing.JComboBox inputWhichIncoherent;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel11;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanel9;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel labelDelayCompensation;
    private javax.swing.JLabel labelIntegrationTime;
    private javax.swing.JLabel labelIntegrationstepsCoherent;
    private javax.swing.JLabel labelIntegrationstepsIncoherent;
    private javax.swing.JLabel labelMaxNetworkDelay;
    private javax.swing.JLabel labelNrBitsPerSample;
    private javax.swing.JLabel labelNrPPFTaps;
    private javax.swing.JLabel labelNrSubbandsPerFrame;
    private javax.swing.JLabel labelNrTimesInFrame;
    private javax.swing.JLabel labelSubbandsPerMs;
    private javax.swing.JLabel labelWhichCoherent;
    private javax.swing.JLabel labelWhichIncoherent;
    private javax.swing.JTextField subbandsPerFrameDerefText;
    // End of variables declaration//GEN-END:variables
 
    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList myListenerList =  null;

    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    @Override
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {

        if (myListenerList == null ) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    @Override
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        myListenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    } 
}
