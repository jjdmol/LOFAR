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
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.ListSelectionModel;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.tablemodels.PencilConfigurationTableModel;
import nl.astron.lofar.sas.otbcomponents.PencilDialog;
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
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) { 
        itsNode=(jOTDBnode)anObject;
        jOTDBparam aParam=null;



        try {
           
            //we need to get all the childs from this node.    
            Vector childs = OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1);
            
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
                } else if (LofarUtils.keyName(aNode.name).equals("DelayComp")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("OLAP_Conn")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("IONProc")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("StorageProc")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Correlator")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("PencilInfo")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }else if (LofarUtils.keyName(aNode.name).contains("Pencil") && !LofarUtils.keyName(aNode.name).equals("PencilInfo")) {
                    itsPencils.addElement(aNode);
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Stokes")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }
            //we also need the Virtual Instrument Storagenodes here
            Vector VIchilds = OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(),"VirtualInstrument");

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
        } catch (RemoteException ex) {
            logger.debug("Error during getComponentParam: "+ ex);
            return;
        }
        
        initPanel();
    }
    
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new OlapPanel();
    }
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
    public void createPopupMenu(Component aComponent,int x, int y) {
        JPopupMenu aPopupMenu=null;
        JMenuItem  aMenuItem=null;
        
        aPopupMenu= new JPopupMenu();
        // For VIC trees
        if (itsTreeType.equals("VHtree")) {
            //  Fill in menu as in the example above
            aMenuItem=new JMenuItem("Create ParSet File");        
            aMenuItem.addActionListener(new java.awt.event.ActionListener() {
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
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
         if (evt.getActionCommand().equals("Create ParSet File")) {
            logger.debug("Create ParSet File");
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
                    logger.debug("File written to: " + aFile.getPath());
                } catch (RemoteException ex) {
                    logger.debug("exportTree failed : " + ex);
                } catch (FileNotFoundException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
                } catch (IOException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
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
            Vector HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
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
            logger.debug("Error during retrieveAndDisplayChildDataForNode: "+ ex);
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
            logger.debug("Error during getParam: "+ ex);
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
            } else if (aKeyName.equals("coresPerPset")) {        
                inputCoresPerPset.setToolTipText(aParam.description);
                itsCoresPerPset=aNode;
                if (isRef && aParam != null) {
                    inputCoresPerPset.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputCoresPerPset.setText(aNode.limits);
                }
            } else if (aKeyName.equals("partition")) {
                inputPartition.setToolTipText(aParam.description);
                itsPartition=aNode;
                if (isRef && aParam != null) {
                    inputPartition.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputPartition.setText(aNode.limits);
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
            } else if (aKeyName.equals("nrRings")) {
                inputNrRings.setToolTipText(aParam.description);
                itsNrRings=aNode;
                if (isRef && aParam != null) {
                    inputNrRings.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrRings.setText(aNode.limits);
                }
            } else if (aKeyName.equals("ringSize")) {
                inputRingSize.setToolTipText(aParam.description);
                itsRingSize=aNode;
                if (isRef && aParam != null) {
                    inputRingSize.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputRingSize.setText(aNode.limits);
                }
            }
        } else if(parentName.equals("Stokes")){
            // OLAP PencilInfo params

            if (aKeyName.equals("which")) {
                inputWhich.setToolTipText(aParam.description);
                LofarUtils.setPopupComboChoices(inputWhich,aParam.limits);
                if (!aNode.limits.equals("")) {
                    inputWhich.setSelectedItem(aNode.limits);
                }
                itsWhich=aNode;
            } else if (aKeyName.equals("integrateChannels")) {
                inputIntegrateChannels.setToolTipText(aParam.description);
                itsIntegrateChannels=aNode;
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
                inputIntegrateChannels.setSelected(aSelection);

            } else if (aKeyName.equals("integrationSteps")) {
                inputIntegrationSteps.setToolTipText(aParam.description);
                itsIntegrationSteps=aNode;
                if (isRef && aParam != null) {
                    inputIntegrationSteps.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputIntegrationSteps.setText(aNode.limits);
                }
            }

        } else if(parentName.equals("DelayComp")){       
            // OLAP DelayComp params

            if (aKeyName.equals("nrCalcDelays")) {        
                inputNrCalcDelays.setToolTipText(aParam.description);
                itsNrCalcDelays=aNode;
                if (isRef && aParam != null) {
                    inputNrCalcDelays.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrCalcDelays.setText(aNode.limits);
                }
            } else if (aKeyName.equals("positionType")) {
                inputPositionType.setToolTipText(aParam.description);
                itsPositionType=aNode;
                if (isRef && aParam != null) {
                    inputPositionType.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputPositionType.setText(aNode.limits);
                }
            }

        } else if(parentName.equals("OLAP_Conn")){
            // OLAP OLAP_Conn params
            if (aKeyName.equals("IONProc_Storage_Ports")) {
                inputIONProcStoragePorts.setToolTipText(aParam.description);
                itsCNProcStoragePorts=aNode;
                if (isRef && aParam != null) {
                    inputIONProcStoragePorts.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputIONProcStoragePorts.setText(aNode.limits);
                }
            } else if (aKeyName.equals("IONProc_CNProc_Transport")) {        
                inputIONProcCNProcTransport.setToolTipText(aParam.description);
                LofarUtils.setPopupComboChoices(inputIONProcCNProcTransport,aParam.limits);
                if (!aNode.limits.equals("")) {
                    inputIONProcCNProcTransport.setSelectedItem(aNode.limits);
                }
                itsIONProcCNProcTransport=aNode;
            } else if (aKeyName.equals("rawDataOutputs")) {
                inputRawDataOutputs.setToolTipText(aParam.description);
                itsRawDataOutputs=aNode;
                if (isRef && aParam != null) {
                    inputRawDataOutputs.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputRawDataOutputs.setText(aNode.limits);
                }
            } else if (aKeyName.equals("rawDataOutputOnly")) {
                inputRawDataOutputOnly.setToolTipText(aParam.description);
                itsRawDataOutputOnly=aNode;
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
                inputRawDataOutputOnly.setSelected(aSelection);

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

            } else if (aKeyName.equals("storageStationNames")) {
                inputStorageStationNames.setToolTipText(aParam.description);
                itsStorageStationNames=aNode;
                if (isRef && aParam != null) {
                    inputStorageStationNames.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputStorageStationNames.setText(aNode.limits);
                }
            } else if (aKeyName.equals("realTime")) {
                inputRealTime.setToolTipText(aParam.description);
                itsRealTime=aNode;
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
                inputRealTime.setSelected(aSelection);
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
            } else if (aKeyName.equals("outputCorrelatedData")) {
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
            } else if (aKeyName.equals("outputFilteredData")) {
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
            } else if (aKeyName.equals("outputBeamFormedData")) {
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
            } else if (aKeyName.equals("outputCoherentStokes")) {
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
                
            } else if (aKeyName.equals("outputIncoherentStokes")) {
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
            } else if (aKeyName.equals("nrPencils")) {
                itsNrPencils=aNode;
            }
        } else if(parentName.contains("Pencil") && !parentName.equals("PencilInfo")){
            // Observation Pencil parameters
            if (aKeyName.equals("angle1")) {
                if (isRef && aParam != null) {
                    itsAngle1.add(aNode.limits + " : " + aParam.limits);
                } else {
                    itsAngle1.add(aNode.limits);
                }
            } else if (aKeyName.equals("angle2")) {
                if (isRef && aParam != null) {
                    itsAngle2.add(aNode.limits + " : " + aParam.limits);
                } else {
                    itsAngle2.add(aNode.limits);
                }
            }
        }   
    }

    // check all settings to make a choice about enabled/disables fields
    private void checkSettings() {
        if (inputOutputCorrelatedData.isSelected()) {
            inputIntegrationTime.setEnabled(true);
        } else {
            inputIntegrationTime.setEnabled(false);
        }

        if (inputOutputBeamFormedData.isSelected() || inputOutputCoherentStokes.isSelected()) {
            inputFlysEye.setEnabled(true);

            if (inputFlysEye.isSelected()) {
                addPencilButton.setEnabled(false);
                pencilConfigurationPanel.setEnabled(false);
                inputNrRings.setEnabled(false);
                inputRingSize.setEnabled(false);
            } else {
                addPencilButton.setEnabled(true);
                pencilConfigurationPanel.setEnabled(true);
                inputNrRings.setEnabled(true);
                inputRingSize.setEnabled(true);
            }
        } else {
            inputFlysEye.setEnabled(false);
            addPencilButton.setEnabled(false);
            pencilConfigurationPanel.setEnabled(false);
            inputNrRings.setEnabled(false);
            inputRingSize.setEnabled(false);
        }

        if (inputOutputCoherentStokes.isSelected() || inputOutputIncoherentStokes.isSelected()) {
            inputWhich.setEnabled(true);
            inputIntegrateChannels.setEnabled(true);
            inputIntegrationSteps.setEnabled(true);
        } else {
            inputWhich.setEnabled(false);
            inputIntegrateChannels.setEnabled(false);
            inputIntegrationSteps.setEnabled(false);
        }
    }
    
    private void restore() {
      boolean aB=false;

      // Olap Specific parameters
      aB=false;
      if (itsDelayCompensation.limits.equals("true")||itsDelayCompensation.limits.equals("TRUE")) {
          aB=true;
      }
      inputDelayCompensation.setSelected(aB);
      inputNrBitsPerSample.setText(itsNrBitsPerSample.limits);
      inputNrSecondsOfBuffer.setText(itsNrSecondsOfBuffer.limits);
      inputNrTimesInFrame.setText(itsNrTimesInFrame.limits);
      inputStorageStationNames.setText(itsStorageStationNames.limits);
      aB=false;
      if (itsRealTime.limits.equals("true")||itsRealTime.limits.equals("TRUE")) {
          aB=true;
      }
      inputRealTime.setSelected(aB);
      aB=false;
      if (itsCorrectBandPass.limits.equals("true")||itsCorrectBandPass.limits.equals("TRUE")) {
          aB=true;
      }
      inputCorrectBandPass.setSelected(aB);
      aB=false;
      if (itsOutputCorrelatedData.limits.equals("true")||itsOutputCorrelatedData.limits.equals("TRUE")) {
          aB=true;
      }
      inputOutputCorrelatedData.setSelected(aB);
      aB=false;
      if (itsOutputFilteredData.limits.equals("true")||itsOutputFilteredData.limits.equals("TRUE")) {
          aB=true;
      }
      inputOutputFilteredData.setSelected(aB);
      aB=false;
      if (itsOutputBeamFormedData.limits.equals("true")||itsOutputBeamFormedData.limits.equals("TRUE")) {
          aB=true;
      }
      inputOutputBeamFormedData.setSelected(aB);
      aB=false;
      if (itsOutputCoherentStokes.limits.equals("true")||itsOutputCoherentStokes.limits.equals("TRUE")) {
          aB=true;
      }
      inputOutputCoherentStokes.setSelected(aB);
      aB=false;
      if (itsOutputIncoherentStokes.limits.equals("true")||itsOutputIncoherentStokes.limits.equals("TRUE")) {
          aB=true;
      }
      inputOutputIncoherentStokes.setSelected(aB);
      inputMaxNetworkDelay.setText(itsMaxNetworkDelay.limits);
      inputNrSubbandsPerFrame.setText(itsNrSubbandsPerFrame.limits);

      //OLAP-CNProc      
      inputNrPPFTaps.setText(itsNrPPFTaps.limits);
      inputCoresPerPset.setText(itsCoresPerPset.limits);
      inputPartition.setText(itsPartition.limits);
      
      //OLAP-DelayComp
      inputPositionType.setText(itsPositionType.limits);
      inputNrCalcDelays.setText(itsNrCalcDelays.limits);
      
      //OLAP StorageProc
      inputSubbandsPerMS.setText(itsSubbandsPerMS.limits);
      
      //OLAP-OLAP_Conn
      inputIONProcStoragePorts.setText(itsCNProcStoragePorts.limits);
      inputIONProcCNProcTransport.setSelectedItem(itsIONProcCNProcTransport.limits);
      inputRawDataOutputs.setText(itsRawDataOutputs.limits);
      aB=false;
      if (itsRawDataOutputOnly.limits.equals("true")||itsRawDataOutputOnly.limits.equals("TRUE")) {
          aB=true;
      }
      inputRawDataOutputOnly.setSelected(aB);

      // Correlator
      inputIntegrationTime.setText(itsIntegrationTime.limits);

      // PencilInfo
      aB=false;
      if (itsFlysEye.limits.equals("true")||itsFlysEye.limits.equals("TRUE")) {
          aB=true;
      }
      inputFlysEye.setSelected(aB);

      inputRingSize.setText(itsRingSize.limits);
      inputNrRings.setText(itsNrRings.limits);

      // Stokes
      inputWhich.setSelectedItem(itsWhich.limits);
      aB=false;
      if (itsIntegrateChannels.limits.equals("true")||itsIntegrateChannels.limits.equals("TRUE")) {
          aB=true;
      }
      inputIntegrateChannels.setSelected(aB);
      inputIntegrationSteps.setText(itsIntegrationSteps.limits);

      // Pencils
      // set table back to initial values
      itsPencilConfigurationTableModel.fillTable(itsTreeType,itsAngle1,itsAngle2);

      checkSettings();
    }
     
    private void initialize() {
        itsPencilConfigurationTableModel = new PencilConfigurationTableModel();
        pencilConfigurationPanel.setTableModel(itsPencilConfigurationTableModel);
        pencilConfigurationPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        pencilConfigurationPanel.setColumnSize("angle 1",20);
        pencilConfigurationPanel.setColumnSize("angle 2",20);
        pencilConfigurationPanel.repaint();
        buttonPanel1.addButton("Restore");
        buttonPanel1.addButton("Apply");
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
                logger.debug("OlapPanel: Error getting treeInfo/treetype" + ex);
                itsTreeType="";
            }         } else {
            logger.debug("ERROR:  no node given");
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
            logger.debug("Error: saveNode failed : " + ex);
        } 
    }   
    
   
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {

        // always do this last to keep up with panel settings regardless of user settings
        checkSettings();
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
    }        
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {

        // always do this last to keep up with panel settings regardless of user settings
        checkSettings();
    }
    
    private void saveInput() {
        boolean hasChanged = false;

        int i=1;

        //delete all Pencils from the table (excluding the Default one);
        // Keep the 1st one, it's the default Pencil
        try {
            for (i=1; i< itsPencils.size(); i++) {
                OtdbRmi.getRemoteMaintenance().deleteNode(itsPencils.elementAt(i));
            }
        } catch (RemoteException ex) {
            logger.error("Error during deletion of PencilNode: "+ex);
        }

        // now that all Nodes are deleted we should collect the tables input and create new Beams to save to the database.
        itsPencilConfigurationTableModel.getTable(itsAngle1,itsAngle2);
        // keep default Beams
        jOTDBnode aDefaultNode= itsPencils.elementAt(0);
        try {
            // for all elements
            for (i=1; i < itsAngle1.size();i++) {

                // make a dupnode from the default node, give it the next number in the count,get the elements and fill all values from the elements
                // with the values from the set fields and save the elements again
                //
                // Duplicates the given node (and its parameters and children)
                int aN = OtdbRmi.getRemoteMaintenance().dupNode(itsNode.treeID(),aDefaultNode.nodeID(),(short)(i-1));
                if (aN <= 0) {
                    logger.error("Something went wrong with duplicating tree no ("+i+") will try to save remainder");
                } else {
                    // we got a new duplicate whos children need to be filled with the settings from the panel.
                    jOTDBnode aNode = OtdbRmi.getRemoteMaintenance().getNode(itsNode.treeID(),aN);
                    // store new duplicate in itsPencilss.
                    itsPencils.add(aNode);

                    Vector HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
                    // get all the params per child
                    Enumeration e1 = HWchilds.elements();
                    while( e1.hasMoreElements()  ) {
                        jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                        String aKeyName = LofarUtils.keyName(aHWNode.name);
                        if (aKeyName.equals("angle1")) {
                            aHWNode.limits=itsAngle1.elementAt(i);
                        } else if (aKeyName.equals("angle2")) {
                            aHWNode.limits=itsAngle2.elementAt(i);
                        }
                        saveNode(aHWNode);
                    }
                }
            }

            // store new number of instances in baseSetting
            aDefaultNode.instances=(short)(itsAngle1.size()-1); // - default at -1
            saveNode(aDefaultNode);

        } catch (RemoteException ex) {
            logger.error("Error during duplication and save : " + ex);
        }

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
        if (itsStorageStationNames != null && !inputStorageStationNames.getText().equals(itsStorageStationNames.limits)) {
            itsStorageStationNames.limits = inputStorageStationNames.getText();
            saveNode(itsStorageStationNames);
        }
        if ((!inputRealTime.isSelected() &  
                (itsRealTime.limits.equals("TRUE") ||itsRealTime.limits.equals("true") )) ||
            (inputRealTime.isSelected() &  
                (itsRealTime.limits.equals("FALSE") ||itsRealTime.limits.equals("false") )))        
        {  
            String rt="true";
            if (!inputRealTime.isSelected()) {
                rt="false";
            }
            itsRealTime.limits = rt;
            saveNode(itsRealTime);
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

        if (itsNrPencils != null && !Integer.toString(pencilConfigurationPanel.getTableModel().getRowCount()).equals(itsNrPencils.limits)) {
            itsNrPencils.limits = Integer.toString(pencilConfigurationPanel.getTableModel().getRowCount());
            saveNode(itsNrPencils);
        }

        
        // OLAP-CNProc
        if (itsNrPPFTaps != null && !inputNrPPFTaps.getText().equals(itsNrPPFTaps.limits)) {
            itsNrPPFTaps.limits = inputNrPPFTaps.getText();
            saveNode(itsNrPPFTaps);
        }
        if (itsCoresPerPset != null && !inputCoresPerPset.getText().equals(itsCoresPerPset.limits)) {
            itsCoresPerPset.limits = inputCoresPerPset.getText();
            saveNode(itsCoresPerPset);
        }
        if (itsPartition != null && !inputPartition.getText().equals(itsPartition.limits)) {
            itsPartition.limits = inputPartition.getText();
            saveNode(itsPartition);
        }

        //Olap-DelayComp
        if (itsNrCalcDelays!= null && !inputNrCalcDelays.getText().equals(itsNrCalcDelays.limits)) {
            itsNrCalcDelays.limits = inputNrCalcDelays.getText();
            saveNode(itsNrCalcDelays);
        }
        if (itsPositionType != null && !inputPositionType.getText().equals(itsPositionType.limits)) {
            itsPositionType.limits = inputPositionType.getText();
            saveNode(itsPositionType);
        }
        
        // OLAP-OLAP_Conn
        if (itsCNProcStoragePorts != null && !inputIONProcStoragePorts.getText().equals(itsCNProcStoragePorts.limits)) {
            itsCNProcStoragePorts.limits = inputIONProcStoragePorts.getText();
            saveNode(itsCNProcStoragePorts);
        }
        if (itsIONProcCNProcTransport!= null && !inputIONProcCNProcTransport.getSelectedItem().toString().equals(itsIONProcCNProcTransport.limits)) {  
            itsIONProcCNProcTransport.limits = inputIONProcCNProcTransport.getSelectedItem().toString();
            saveNode(itsIONProcCNProcTransport);
        }
        if (itsRawDataOutputs != null && !inputRawDataOutputs.getText().equals(itsRawDataOutputs.limits)) {
            itsRawDataOutputs.limits = inputRawDataOutputs.getText();
            saveNode(itsRawDataOutputs);
        }
        if ((!inputRawDataOutputOnly.isSelected() &
                (itsRawDataOutputOnly.limits.equals("TRUE") ||itsRawDataOutputOnly.limits.equals("true") )) ||
            (inputRawDataOutputOnly.isSelected() &
                (itsRawDataOutputOnly.limits.equals("FALSE") ||itsRawDataOutputOnly.limits.equals("false") )))
        {
            String rt="true";
            if (!inputRawDataOutputOnly.isSelected()) {
                rt="false";
            }
            itsRawDataOutputOnly.limits = rt;
            saveNode(itsRawDataOutputOnly);
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

        if (itsNrRings != null && !inputNrRings.getText().equals(itsNrRings.limits)) {
            itsNrRings.limits = inputNrRings.getText();
            saveNode(itsNrRings);
        }

        if (itsRingSize != null && !inputRingSize.getText().equals(itsRingSize.limits)) {
            itsRingSize.limits = inputRingSize.getText();
            saveNode(itsRingSize);
        }

        // Stokes

        if (itsWhich!= null && !inputWhich.getSelectedItem().toString().equals(itsWhich.limits)) {
            itsWhich.limits = inputWhich.getSelectedItem().toString();
            saveNode(itsWhich);
        }

        if (itsIntegrationSteps != null && !inputIntegrationSteps.getText().equals(itsIntegrationSteps.limits)) {
            itsIntegrationSteps.limits = inputIntegrationSteps.getText();
            saveNode(itsIntegrationSteps);
        }
        if ((!inputIntegrateChannels.isSelected() &
                (itsIntegrateChannels.limits.equals("TRUE") ||itsIntegrateChannels.limits.equals("true") )) ||
            (inputIntegrateChannels.isSelected() &
                (itsIntegrateChannels.limits.equals("FALSE") ||itsIntegrateChannels.limits.equals("false") )))
        {
            String rt="true";
            if (!inputIntegrateChannels.isSelected()) {
                rt="false";
            }
            itsIntegrateChannels.limits = rt;
            saveNode(itsIntegrateChannels);
        }
    }

    private void deletePencil() {
        int row = pencilConfigurationPanel.getSelectedRow();

        if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this Pencil ?","Delete Pencil",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
            if (row > -1) {
                itsPencilConfigurationTableModel.removeRow(row);
                // No selection anymore after delete, so buttons disabled again
                this.editPencilButton.setEnabled(false);
                this.deletePencilButton.setEnabled(false);


            }
        }

      if (pencilConfigurationPanel.getTableModel().getRowCount() == 8) {
        this.addPencilButton.setEnabled(false);
      } else {
        this.addPencilButton.setEnabled(true);
      }
    }

    private void addPencil() {

        itsSelectedRow=-1;
        // set selection to defaults.
        String [] selection = {itsAngle1.elementAt(0),itsAngle2.elementAt(0)};
        if (editting) {
            itsSelectedRow = pencilConfigurationPanel.getSelectedRow();
            selection = itsPencilConfigurationTableModel.getSelection(itsSelectedRow);

            // if no row is selected, nothing to be done
            if (selection == null || selection[0].equals("")) {
                return;
            }
        }
        pencilDialog = new PencilDialog(itsMainFrame,true,selection);
        pencilDialog.setLocationRelativeTo(this);
        if (editting) {
            pencilDialog.setBorderTitle("edit Pencil");
        } else {
            pencilDialog.setBorderTitle("add new Pencil");
        }
        pencilDialog.setVisible(true);

        // check if something has changed
        if (pencilDialog.hasChanged()) {
            String[] newRow = pencilDialog.getBeam();
            // check if we are editting an entry or adding a new entry
            if (editting) {
                itsPencilConfigurationTableModel.updateRow(newRow,itsSelectedRow);
                // set editting = false
                editting=false;
            } else {
                itsPencilConfigurationTableModel.addRow(newRow);
            }
        }

        this.editPencilButton.setEnabled(false);
        this.deletePencilButton.setEnabled(false);
        this.addPencilButton.setEnabled(true);

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
        labelStorageStationNames = new javax.swing.JLabel();
        inputStorageStationNames = new javax.swing.JTextField();
        labelMaxNetworkDelay = new javax.swing.JLabel();
        inputMaxNetworkDelay = new javax.swing.JTextField();
        labelNrSubbandsPerFrame = new javax.swing.JLabel();
        inputNrSubbandsPerFrame = new javax.swing.JTextField();
        labelRealTime = new javax.swing.JLabel();
        inputRealTime = new javax.swing.JCheckBox();
        labelCorrectBandPass = new javax.swing.JLabel();
        inputCorrectBandPass = new javax.swing.JCheckBox();
        subbandsPerFrameDerefText = new javax.swing.JTextField();
        labelOutputCorrelatedData = new javax.swing.JLabel();
        jLabel3 = new javax.swing.JLabel();
        inputOutputCorrelatedData = new javax.swing.JCheckBox();
        labelOutputFilteredData = new javax.swing.JLabel();
        inputOutputFilteredData = new javax.swing.JCheckBox();
        labelOutputBeamFormedData = new javax.swing.JLabel();
        inputOutputBeamFormedData = new javax.swing.JCheckBox();
        labelOutputCoherentStokes = new javax.swing.JLabel();
        inputOutputCoherentStokes = new javax.swing.JCheckBox();
        labelOutputIncoherentStokes = new javax.swing.JLabel();
        inputOutputIncoherentStokes = new javax.swing.JCheckBox();
        jPanel6 = new javax.swing.JPanel();
        labelIONProcCNProcTransport = new javax.swing.JLabel();
        inputIONProcCNProcTransport = new javax.swing.JComboBox();
        labelIONProcStoragePorts = new javax.swing.JLabel();
        inputIONProcStoragePorts = new javax.swing.JTextField();
        labelRawDataOutputs = new javax.swing.JLabel();
        inputRawDataOutputs = new javax.swing.JTextField();
        labelRawDataOutputOnly = new javax.swing.JLabel();
        inputRawDataOutputOnly = new javax.swing.JCheckBox();
        jPanel7 = new javax.swing.JPanel();
        labelNrPPFTaps = new javax.swing.JLabel();
        inputNrPPFTaps = new javax.swing.JTextField();
        labelPartition = new javax.swing.JLabel();
        labelCoresPerPset = new javax.swing.JLabel();
        inputCoresPerPset = new javax.swing.JTextField();
        inputPartition = new javax.swing.JTextField();
        jPanel11 = new javax.swing.JPanel();
        labelSubbandsPerMs = new javax.swing.JLabel();
        inputSubbandsPerMS = new javax.swing.JTextField();
        jPanel5 = new javax.swing.JPanel();
        labelNrCalcDelays = new javax.swing.JLabel();
        labelPositionType = new javax.swing.JLabel();
        inputNrCalcDelays = new javax.swing.JTextField();
        inputPositionType = new javax.swing.JTextField();
        PencilInfoPanel = new javax.swing.JPanel();
        labelFlysEye = new javax.swing.JLabel();
        inputFlysEye = new javax.swing.JCheckBox();
        labelNrRings = new javax.swing.JLabel();
        inputNrRings = new javax.swing.JTextField();
        labelRingSize = new javax.swing.JLabel();
        inputRingSize = new javax.swing.JTextField();
        CorrelatorPanel = new javax.swing.JPanel();
        labelIntegrationTime = new javax.swing.JLabel();
        inputIntegrationTime = new javax.swing.JTextField();
        jLabel2 = new javax.swing.JLabel();
        StokesPanel = new javax.swing.JPanel();
        labelWhich = new javax.swing.JLabel();
        inputWhich = new javax.swing.JComboBox();
        labelIntegrateChannel = new javax.swing.JLabel();
        inputIntegrateChannels = new javax.swing.JCheckBox();
        labelIntegrationsteps = new javax.swing.JLabel();
        inputIntegrationSteps = new javax.swing.JTextField();
        jPanel3 = new javax.swing.JPanel();
        pencilConfigurationPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        addPencilButton = new javax.swing.JButton();
        editPencilButton = new javax.swing.JButton();
        deletePencilButton = new javax.swing.JButton();
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

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("OLAP Details");
        jPanel1.add(jLabel1, java.awt.BorderLayout.CENTER);

        add(jPanel1, java.awt.BorderLayout.NORTH);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(""));
        jPanel2.setPreferredSize(new java.awt.Dimension(5240, 3000));

        jPanel8.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Olap", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        labelNrBitsPerSample.setText("# Bits per Sample:");

        labelNrTimesInFrame.setText("# Times In Frame:");

        labelDelayCompensation.setText("Delay Compensation:");

        inputDelayCompensation.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputDelayCompensation.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelNrSecondsOfBuffer.setText("# SecondsOfBuffer:");

        labelStorageStationNames.setText("storageStationNames:");

        labelMaxNetworkDelay.setText("Max Network Delay:");

        labelNrSubbandsPerFrame.setText("# SubbandsPerFrame:");

        inputNrSubbandsPerFrame.setToolTipText("");

        labelRealTime.setText("RealTime:");

        inputRealTime.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputRealTime.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelCorrectBandPass.setText("Correct BandPass:");

        inputCorrectBandPass.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputCorrectBandPass.setMargin(new java.awt.Insets(0, 0, 0, 0));

        subbandsPerFrameDerefText.setEditable(false);
        subbandsPerFrameDerefText.setEnabled(false);

        labelOutputCorrelatedData.setText("Correlated Data:");

        jLabel3.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel3.setText("Output:");

        inputOutputCorrelatedData.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputCorrelatedDataActionPerformed(evt);
            }
        });

        labelOutputFilteredData.setText("Filtered Data:");

        labelOutputBeamFormedData.setText("Beamformed Data:");

        inputOutputBeamFormedData.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputBeamFormedDataActionPerformed(evt);
            }
        });

        labelOutputCoherentStokes.setText("Coherent Stokes:");

        inputOutputCoherentStokes.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOutputCoherentStokesActionPerformed(evt);
            }
        });

        labelOutputIncoherentStokes.setText("Incoherent Stokes:");

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
                    .addComponent(labelStorageStationNames, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelRealTime, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelDelayCompensation, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelNrBitsPerSample, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelNrTimesInFrame, javax.swing.GroupLayout.DEFAULT_SIZE, 117, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(inputDelayCompensation)
                            .addComponent(inputNrBitsPerSample, javax.swing.GroupLayout.PREFERRED_SIZE, 143, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(inputNrTimesInFrame, javax.swing.GroupLayout.PREFERRED_SIZE, 144, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(inputRealTime))
                        .addGap(30, 30, 30)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(labelNrSubbandsPerFrame, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(labelMaxNetworkDelay, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(labelNrSecondsOfBuffer, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(labelCorrectBandPass, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                    .addComponent(inputStorageStationNames, javax.swing.GroupLayout.PREFERRED_SIZE, 128, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(inputNrSubbandsPerFrame)
                            .addComponent(inputMaxNetworkDelay)
                            .addComponent(inputNrSecondsOfBuffer, javax.swing.GroupLayout.DEFAULT_SIZE, 137, Short.MAX_VALUE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addComponent(subbandsPerFrameDerefText, javax.swing.GroupLayout.PREFERRED_SIZE, 119, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(23, 23, 23)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addComponent(labelOutputCorrelatedData, javax.swing.GroupLayout.PREFERRED_SIZE, 110, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                        .addComponent(inputOutputCorrelatedData))
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addComponent(labelOutputFilteredData, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                        .addComponent(inputOutputFilteredData))
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addComponent(labelOutputBeamFormedData, javax.swing.GroupLayout.PREFERRED_SIZE, 110, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                        .addComponent(inputOutputBeamFormedData))
                                    .addGroup(javax.swing.GroupLayout.Alignment.LEADING, jPanel8Layout.createSequentialGroup()
                                        .addComponent(labelOutputCoherentStokes, javax.swing.GroupLayout.PREFERRED_SIZE, 110, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                        .addComponent(inputOutputCoherentStokes))
                                    .addGroup(jPanel8Layout.createSequentialGroup()
                                        .addComponent(labelOutputIncoherentStokes, javax.swing.GroupLayout.PREFERRED_SIZE, 110, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                        .addComponent(inputOutputIncoherentStokes))))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(231, 231, 231)
                                .addComponent(jLabel3))))
                    .addComponent(inputCorrectBandPass))
                .addContainerGap(75, Short.MAX_VALUE))
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE, false)
                    .addComponent(labelNrTimesInFrame)
                    .addComponent(labelNrSecondsOfBuffer)
                    .addComponent(inputNrSecondsOfBuffer, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel3)
                    .addComponent(inputNrTimesInFrame, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE, false)
                    .addComponent(labelNrBitsPerSample)
                    .addComponent(labelMaxNetworkDelay)
                    .addComponent(inputMaxNetworkDelay, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelOutputCorrelatedData)
                    .addComponent(inputOutputCorrelatedData)
                    .addComponent(inputNrBitsPerSample, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                    .addComponent(labelNrSubbandsPerFrame)
                                    .addComponent(inputNrSubbandsPerFrame, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(subbandsPerFrameDerefText, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(inputCorrectBandPass, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(labelCorrectBandPass)))
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                    .addComponent(inputDelayCompensation, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(labelDelayCompensation))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(labelRealTime)
                                    .addComponent(inputRealTime, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))))
                        .addGap(12, 12, 12)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(labelStorageStationNames)
                            .addComponent(inputStorageStationNames, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE, false)
                            .addComponent(labelOutputFilteredData)
                            .addComponent(inputOutputFilteredData))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE, false)
                            .addComponent(labelOutputBeamFormedData)
                            .addComponent(inputOutputBeamFormedData))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE, false)
                            .addComponent(labelOutputCoherentStokes)
                            .addComponent(inputOutputCoherentStokes))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE, false)
                            .addComponent(labelOutputIncoherentStokes)
                            .addComponent(inputOutputIncoherentStokes)))))
        );

        jPanel6.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Olap Conn", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        jPanel6.setToolTipText("Olap Conn");

        labelIONProcCNProcTransport.setText("IONProc->CNProc Transport:");

        inputIONProcCNProcTransport.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));

        labelIONProcStoragePorts.setText("IONProc->Storage Ports:");

        labelRawDataOutputs.setText("rawDataOutputs:");

        labelRawDataOutputOnly.setText("Raw data input only:");

        inputRawDataOutputOnly.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputRawDataOutputOnly.setMargin(new java.awt.Insets(0, 0, 0, 0));

        javax.swing.GroupLayout jPanel6Layout = new javax.swing.GroupLayout(jPanel6);
        jPanel6.setLayout(jPanel6Layout);
        jPanel6Layout.setHorizontalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(labelRawDataOutputOnly, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelRawDataOutputs, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelIONProcStoragePorts, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelIONProcCNProcTransport, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputIONProcStoragePorts, javax.swing.GroupLayout.DEFAULT_SIZE, 142, Short.MAX_VALUE)
                    .addComponent(inputIONProcCNProcTransport, 0, 142, Short.MAX_VALUE)
                    .addComponent(inputRawDataOutputs, javax.swing.GroupLayout.DEFAULT_SIZE, 142, Short.MAX_VALUE)
                    .addComponent(inputRawDataOutputOnly))
                .addContainerGap())
        );
        jPanel6Layout.setVerticalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIONProcCNProcTransport)
                    .addComponent(inputIONProcCNProcTransport, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIONProcStoragePorts)
                    .addComponent(inputIONProcStoragePorts, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelRawDataOutputs)
                    .addComponent(inputRawDataOutputs, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(labelRawDataOutputOnly)
                    .addComponent(inputRawDataOutputOnly, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );

        jPanel7.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "CN Proc", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        jPanel7.setToolTipText("BGLProc");

        labelNrPPFTaps.setText("# PPFTaps:");

        labelPartition.setText("Partition:");

        labelCoresPerPset.setText("# Cores per Pset:");

        javax.swing.GroupLayout jPanel7Layout = new javax.swing.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addGroup(jPanel7Layout.createSequentialGroup()
                        .addGap(2, 2, 2)
                        .addComponent(labelCoresPerPset, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(labelPartition, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelNrPPFTaps, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(inputPartition)
                    .addComponent(inputCoresPerPset)
                    .addComponent(inputNrPPFTaps, javax.swing.GroupLayout.PREFERRED_SIZE, 154, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(30, Short.MAX_VALUE))
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelNrPPFTaps)
                    .addComponent(inputNrPPFTaps, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelPartition)
                    .addComponent(inputPartition, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelCoresPerPset)
                    .addComponent(inputCoresPerPset, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(42, Short.MAX_VALUE))
        );

        jPanel11.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Storage Proc", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        labelSubbandsPerMs.setText("#subbands per MS:");

        javax.swing.GroupLayout jPanel11Layout = new javax.swing.GroupLayout(jPanel11);
        jPanel11.setLayout(jPanel11Layout);
        jPanel11Layout.setHorizontalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addComponent(labelSubbandsPerMs)
                .addGap(18, 18, 18)
                .addComponent(inputSubbandsPerMS, javax.swing.GroupLayout.PREFERRED_SIZE, 125, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(60, Short.MAX_VALUE))
        );
        jPanel11Layout.setVerticalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addGroup(jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelSubbandsPerMs)
                    .addComponent(inputSubbandsPerMS, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(12, Short.MAX_VALUE))
        );

        jPanel5.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Delay Compensation", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        jPanel5.setToolTipText("Delay Compensation");

        labelNrCalcDelays.setText("# Calc. Delays:");

        labelPositionType.setText("PositionType:");

        javax.swing.GroupLayout jPanel5Layout = new javax.swing.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel5Layout.createSequentialGroup()
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelNrCalcDelays, javax.swing.GroupLayout.PREFERRED_SIZE, 87, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelPositionType))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(inputPositionType)
                    .addComponent(inputNrCalcDelays, javax.swing.GroupLayout.DEFAULT_SIZE, 145, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel5Layout.createSequentialGroup()
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelNrCalcDelays)
                    .addComponent(inputNrCalcDelays, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelPositionType)
                    .addComponent(inputPositionType, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
        );

        PencilInfoPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "PencilInfo", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        PencilInfoPanel.setToolTipText("PencilInfo");

        labelFlysEye.setText("Flyseye:");

        inputFlysEye.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputFlysEye.setEnabled(false);
        inputFlysEye.setMargin(new java.awt.Insets(0, 0, 0, 0));
        inputFlysEye.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputFlysEyeActionPerformed(evt);
            }
        });

        labelNrRings.setText("# rings:");

        inputNrRings.setEnabled(false);

        labelRingSize.setText("Ring size:");

        inputRingSize.setEnabled(false);

        javax.swing.GroupLayout PencilInfoPanelLayout = new javax.swing.GroupLayout(PencilInfoPanel);
        PencilInfoPanel.setLayout(PencilInfoPanelLayout);
        PencilInfoPanelLayout.setHorizontalGroup(
            PencilInfoPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(PencilInfoPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(PencilInfoPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(labelRingSize, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelFlysEye, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelNrRings, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(PencilInfoPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputFlysEye)
                    .addComponent(inputNrRings, javax.swing.GroupLayout.PREFERRED_SIZE, 99, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(inputRingSize, javax.swing.GroupLayout.PREFERRED_SIZE, 99, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(134, Short.MAX_VALUE))
        );
        PencilInfoPanelLayout.setVerticalGroup(
            PencilInfoPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(PencilInfoPanelLayout.createSequentialGroup()
                .addGroup(PencilInfoPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputFlysEye, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelFlysEye))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(PencilInfoPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelNrRings)
                    .addComponent(inputNrRings, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(PencilInfoPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelRingSize)
                    .addComponent(inputRingSize, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
        );

        CorrelatorPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Correlator", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        CorrelatorPanel.setToolTipText("Correlator");

        labelIntegrationTime.setText("integration Time:");

        inputIntegrationTime.setEnabled(false);

        jLabel2.setText("sec");

        javax.swing.GroupLayout CorrelatorPanelLayout = new javax.swing.GroupLayout(CorrelatorPanel);
        CorrelatorPanel.setLayout(CorrelatorPanelLayout);
        CorrelatorPanelLayout.setHorizontalGroup(
            CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, CorrelatorPanelLayout.createSequentialGroup()
                .addComponent(labelIntegrationTime, javax.swing.GroupLayout.DEFAULT_SIZE, 168, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputIntegrationTime, javax.swing.GroupLayout.PREFERRED_SIZE, 63, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(36, 36, 36)
                .addComponent(jLabel2)
                .addContainerGap())
        );
        CorrelatorPanelLayout.setVerticalGroup(
            CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(CorrelatorPanelLayout.createSequentialGroup()
                .addGroup(CorrelatorPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIntegrationTime)
                    .addComponent(jLabel2)
                    .addComponent(inputIntegrationTime, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(46, Short.MAX_VALUE))
        );

        StokesPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Stokes", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        StokesPanel.setToolTipText("Stokes");

        labelWhich.setText("which");

        inputWhich.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputWhich.setEnabled(false);

        labelIntegrateChannel.setText("integrate Channels:");

        inputIntegrateChannels.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputIntegrateChannels.setEnabled(false);
        inputIntegrateChannels.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelIntegrationsteps.setText("integration Steps:");

        inputIntegrationSteps.setEnabled(false);

        javax.swing.GroupLayout StokesPanelLayout = new javax.swing.GroupLayout(StokesPanel);
        StokesPanel.setLayout(StokesPanelLayout);
        StokesPanelLayout.setHorizontalGroup(
            StokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(StokesPanelLayout.createSequentialGroup()
                .addGroup(StokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addComponent(labelIntegrationsteps, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelIntegrateChannel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelWhich, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 103, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(StokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(StokesPanelLayout.createSequentialGroup()
                        .addComponent(inputIntegrateChannels)
                        .addContainerGap())
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, StokesPanelLayout.createSequentialGroup()
                        .addGroup(StokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(inputIntegrationSteps, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 121, Short.MAX_VALUE)
                            .addComponent(inputWhich, 0, 121, Short.MAX_VALUE))
                        .addGap(48, 48, 48))))
        );
        StokesPanelLayout.setVerticalGroup(
            StokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(StokesPanelLayout.createSequentialGroup()
                .addGroup(StokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelWhich)
                    .addComponent(inputWhich, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(StokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelIntegrateChannel)
                    .addComponent(inputIntegrateChannels, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(StokesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIntegrationsteps)
                    .addComponent(inputIntegrationSteps, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
        );

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Pencil Configuration", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        jPanel3.setPreferredSize(new java.awt.Dimension(200, 125));
        jPanel3.setRequestFocusEnabled(false);
        jPanel3.setVerifyInputWhenFocusTarget(false);

        pencilConfigurationPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                pencilConfigurationPanelMouseClicked(evt);
            }
        });

        addPencilButton.setText("add pencil");
        addPencilButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addPencilButtonActionPerformed(evt);
            }
        });

        editPencilButton.setText("edit pencil");
        editPencilButton.setEnabled(false);
        editPencilButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                editPencilButtonActionPerformed(evt);
            }
        });

        deletePencilButton.setText("delete pencil");
        deletePencilButton.setEnabled(false);
        deletePencilButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deletePencilButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(pencilConfigurationPanel, javax.swing.GroupLayout.DEFAULT_SIZE, 904, Short.MAX_VALUE)
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(addPencilButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(editPencilButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(deletePencilButton)))
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addComponent(pencilConfigurationPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 134, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(editPencilButton)
                    .addComponent(addPencilButton)
                    .addComponent(deletePencilButton))
                .addGap(20, 20, 20))
        );

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(jPanel3, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 930, Short.MAX_VALUE)
                    .addGroup(jPanel4Layout.createSequentialGroup()
                        .addComponent(CorrelatorPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(StokesPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(PencilInfoPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(javax.swing.GroupLayout.Alignment.LEADING, jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                        .addComponent(jPanel8, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGroup(javax.swing.GroupLayout.Alignment.LEADING, jPanel4Layout.createSequentialGroup()
                            .addComponent(jPanel6, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(jPanel7, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(jPanel11, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(jPanel5, javax.swing.GroupLayout.DEFAULT_SIZE, 313, Short.MAX_VALUE)))))
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addComponent(jPanel8, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel4Layout.createSequentialGroup()
                        .addComponent(jPanel5, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jPanel11, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(jPanel7, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel6, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 144, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(StokesPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(PencilInfoPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(CorrelatorPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel3, javax.swing.GroupLayout.PREFERRED_SIZE, 213, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(4264, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(2341, Short.MAX_VALUE))
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
        checkSettings();
    }//GEN-LAST:event_inputOutputBeamFormedDataActionPerformed

    private void inputOutputCoherentStokesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputCoherentStokesActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputOutputCoherentStokesActionPerformed

    private void inputOutputIncoherentStokesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOutputIncoherentStokesActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputOutputIncoherentStokesActionPerformed

    private void pencilConfigurationPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_pencilConfigurationPanelMouseClicked
        if (addPencilButton.isEnabled()) {
            editPencilButton.setEnabled(true);
            deletePencilButton.setEnabled(true);
        }
}//GEN-LAST:event_pencilConfigurationPanelMouseClicked

    private void addPencilButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addPencilButtonActionPerformed
        addPencil();
}//GEN-LAST:event_addPencilButtonActionPerformed

    private void editPencilButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_editPencilButtonActionPerformed
        editting=true;
        addPencil();
}//GEN-LAST:event_editPencilButtonActionPerformed

    private void deletePencilButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deletePencilButtonActionPerformed
        deletePencil();
}//GEN-LAST:event_deletePencilButtonActionPerformed

    private void inputFlysEyeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputFlysEyeActionPerformed
       checkSettings();
    }//GEN-LAST:event_inputFlysEyeActionPerformed
    
    private jOTDBnode    itsNode      = null;
    private MainFrame    itsMainFrame = null;
    private String       itsTreeType  = "";
    private JFileChooser fc           = null;  
    private boolean  editting = false;
    private int      itsSelectedRow = -1;
    private PencilConfigurationTableModel       itsPencilConfigurationTableModel = null;
    private PencilDialog                        pencilDialog = null;

    //Olap specific parameters
    private jOTDBnode itsDelayCompensation=null;
    private jOTDBnode itsNrBitsPerSample=null;
    private jOTDBnode itsNrSecondsOfBuffer=null;
    private jOTDBnode itsNrTimesInFrame=null;
    private jOTDBnode itsStorageStationNames=null;
    private jOTDBnode itsMaxNetworkDelay=null;
    private jOTDBnode itsNrSubbandsPerFrame=null;
    private jOTDBnode itsRealTime=null;
    private jOTDBnode itsCorrectBandPass=null;
    private jOTDBnode itsOutputCorrelatedData=null;
    private jOTDBnode itsOutputFilteredData=null;
    private jOTDBnode itsOutputBeamFormedData=null;
    private jOTDBnode itsOutputCoherentStokes=null;
    private jOTDBnode itsOutputIncoherentStokes=null;

    
    // OLAP-CNProc parameters
    private jOTDBnode itsNrPPFTaps=null;
    private jOTDBnode itsPartition=null;
    private jOTDBnode itsCoresPerPset=null;
    private jOTDBnode itsRawDataOutputOnly=null;

    
    // OLAP-DelayComp parameters
    private jOTDBnode itsNrCalcDelays=null;
    private jOTDBnode itsPositionType=null;
    
    // OLAP-Conn parameters
    private jOTDBnode itsCNProcStoragePorts=null;
    private jOTDBnode itsIONProcCNProcTransport=null;
    private jOTDBnode itsRawDataOutputs=null;
   
    // OLAP-StorageProc parameters
    private jOTDBnode itsSubbandsPerMS=null;

    //Correlator
    private jOTDBnode itsIntegrationTime=null;

    // Stokes
    private jOTDBnode itsWhich=null;
    private jOTDBnode itsIntegrateChannels=null;
    private jOTDBnode itsIntegrationSteps=null;

    // PencilInfo
    private jOTDBnode itsFlysEye=null;
    private jOTDBnode itsNrRings=null;
    private jOTDBnode itsRingSize=null;

    // Pencil
    private Vector<jOTDBnode> itsPencils          = new Vector<jOTDBnode>();
    // Olap Pencil parameters
    private Vector<String>    itsAngle1         = new Vector<String>();
    private Vector<String>    itsAngle2         = new Vector<String>();

    private jOTDBnode itsNrPencils=null;

    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel CorrelatorPanel;
    private javax.swing.JPanel PencilInfoPanel;
    private javax.swing.JPanel StokesPanel;
    private javax.swing.JButton addPencilButton;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton deletePencilButton;
    private javax.swing.JButton editPencilButton;
    private javax.swing.JTextField inputCoresPerPset;
    private javax.swing.JCheckBox inputCorrectBandPass;
    private javax.swing.JCheckBox inputDelayCompensation;
    private javax.swing.JCheckBox inputFlysEye;
    private javax.swing.JComboBox inputIONProcCNProcTransport;
    private javax.swing.JTextField inputIONProcStoragePorts;
    private javax.swing.JCheckBox inputIntegrateChannels;
    private javax.swing.JTextField inputIntegrationSteps;
    private javax.swing.JTextField inputIntegrationTime;
    private javax.swing.JTextField inputMaxNetworkDelay;
    private javax.swing.JTextField inputNrBitsPerSample;
    private javax.swing.JTextField inputNrCalcDelays;
    private javax.swing.JTextField inputNrPPFTaps;
    private javax.swing.JTextField inputNrRings;
    private javax.swing.JTextField inputNrSecondsOfBuffer;
    private javax.swing.JTextField inputNrSubbandsPerFrame;
    private javax.swing.JTextField inputNrTimesInFrame;
    private javax.swing.JCheckBox inputOutputBeamFormedData;
    private javax.swing.JCheckBox inputOutputCoherentStokes;
    private javax.swing.JCheckBox inputOutputCorrelatedData;
    private javax.swing.JCheckBox inputOutputFilteredData;
    private javax.swing.JCheckBox inputOutputIncoherentStokes;
    private javax.swing.JTextField inputPartition;
    private javax.swing.JTextField inputPositionType;
    private javax.swing.JCheckBox inputRawDataOutputOnly;
    private javax.swing.JTextField inputRawDataOutputs;
    private javax.swing.JCheckBox inputRealTime;
    private javax.swing.JTextField inputRingSize;
    private javax.swing.JTextField inputStorageStationNames;
    private javax.swing.JTextField inputSubbandsPerMS;
    private javax.swing.JComboBox inputWhich;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel11;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanel9;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel labelCoresPerPset;
    private javax.swing.JLabel labelCorrectBandPass;
    private javax.swing.JLabel labelDelayCompensation;
    private javax.swing.JLabel labelFlysEye;
    private javax.swing.JLabel labelIONProcCNProcTransport;
    private javax.swing.JLabel labelIONProcStoragePorts;
    private javax.swing.JLabel labelIntegrateChannel;
    private javax.swing.JLabel labelIntegrationTime;
    private javax.swing.JLabel labelIntegrationsteps;
    private javax.swing.JLabel labelMaxNetworkDelay;
    private javax.swing.JLabel labelNrBitsPerSample;
    private javax.swing.JLabel labelNrCalcDelays;
    private javax.swing.JLabel labelNrPPFTaps;
    private javax.swing.JLabel labelNrRings;
    private javax.swing.JLabel labelNrSubbandsPerFrame;
    private javax.swing.JLabel labelNrTimesInFrame;
    private javax.swing.JLabel labelOutputBeamFormedData;
    private javax.swing.JLabel labelOutputCoherentStokes;
    private javax.swing.JLabel labelOutputCorrelatedData;
    private javax.swing.JLabel labelOutputFilteredData;
    private javax.swing.JLabel labelOutputIncoherentStokes;
    private javax.swing.JLabel labelPartition;
    private javax.swing.JLabel labelPositionType;
    private javax.swing.JLabel labelRawDataOutputOnly;
    private javax.swing.JLabel labelRawDataOutputs;
    private javax.swing.JLabel labelRealTime;
    private javax.swing.JLabel labelRingSize;
    private javax.swing.JLabel labelStorageStationNames;
    private javax.swing.JLabel labelSubbandsPerMs;
    private javax.swing.JLabel labelWhich;
    private nl.astron.lofar.sas.otbcomponents.TablePanel pencilConfigurationPanel;
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
