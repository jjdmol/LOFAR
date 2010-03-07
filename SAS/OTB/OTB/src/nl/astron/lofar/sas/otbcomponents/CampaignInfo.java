/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * CampaignInfo.java
 *
 * Created on 28-feb-2010, 19:42:09
 */

package nl.astron.lofar.sas.otbcomponents;

import java.rmi.RemoteException;
import java.util.Iterator;
import java.util.Vector;
import java.util.logging.Level;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jCampaignInfo;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;


/**
 *
 * @author Coolen
 */
public class CampaignInfo extends javax.swing.JPanel {

    static Logger logger = Logger.getLogger(CampaignInfo.class);
    static String name = "CampaignInfo";

    /** Creates new form BeanForm */
    public CampaignInfo() {
        initComponents();
        itsMainFrame=null;
    }

         /** Creates new form BeanForm with mainFrame info
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public CampaignInfo(MainFrame aMainFrame) {
        initComponents();
        itsMainFrame = aMainFrame;
        isEditable=false;
        initPanel();
    }

    public void setMainFrame(MainFrame aMainFrame,boolean edit) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            isEditable=edit;
            initPanel();
        } else {
            logger.debug("No Mainframe supplied");
        }
    }

    public void refresh() {
        initPanel();
    }

    private void initPanel() {

        itsMainFrame.setHourglassCursor();

        if (isEditable && itsCampaignName.equalsIgnoreCase("No Campaign")) {
            this.inputNameCombo.setVisible(isEditable);
            this.inputNameTxt.setVisible(!isEditable);
        } else {
            this.inputNameCombo.setVisible(isEditable);
            this.inputNameTxt.setVisible(!isEditable);
        }
        this.inputNameTxt.setText("");
        this.inputTitle.setText("");
        this.inputPI.setText("");
        this.inputCO_I.setText("");
        this.inputContact.setText("");

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


         if (itsMainFrame != null) {
            try {

                itsTree=OtdbRmi.getRemoteOTDB().getTreeInfo(itsMainFrame.getSharedVars().getTreeID(),false);
                //get the CampaignName from the tree
                itsCampaignName=itsTree.campaign;

                // get all existing campaigns to fill the combobox, set default to original CampainName
                // only possible for Campaigns that have NoCampaign
                if (isEditable && itsCampaignName.equalsIgnoreCase("No Campaign")) {
                    Vector<jCampaignInfo> aCampaignList=OtdbRmi.getRemoteCampaign().getCampaignList();
                    if (aCampaignList.size() > 0) {
                        Iterator itr = aCampaignList.iterator();
                        while (itr.hasNext()){
                            this.inputNameCombo.addItem(((jCampaignInfo)itr.next()).itsName);
                        }
                        this.inputNameCombo.setSelectedIndex(0);
                    }
                } else {
                    this.inputNameTxt.setText(itsCampaignName);
                }
                // get the CampaignInfo from the tree
                itsCampaignInfo = OtdbRmi.getRemoteCampaign().getCampaign(itsCampaignName);
                if  (itsCampaignInfo != null) {
                    this.inputTitle.setText(itsCampaignInfo.itsTitle);
                    this.inputPI.setText(itsCampaignInfo.itsPI);
                    this.inputCO_I.setText(itsCampaignInfo.itsCO_I);
                    this.inputContact.setText(itsCampaignInfo.itsContact);
                } else {
                    this.inputNameTxt.setText("");
                    this.inputTitle.setText("");
                    this.inputPI.setText("");
                    this.inputCO_I.setText("");
                    this.inputContact.setText("");
                }
            } catch (RemoteException ex) {
                logger.debug("ObservationPanel: Error getting treeInfo/campaignInfo" + ex);
                this.inputNameTxt.setText("");
                this.inputTitle.setText("");
                this.inputPI.setText("");
                this.inputCO_I.setText("");
                this.inputContact.setText("");
            }
         } else {
            logger.debug("ERROR:  no MainFrame given");
        }
        itsMainFrame.setNormalCursor();

    }

    public boolean hasChanged() {
        if (isEditable) {
            if (!itsCampaignName.equals(inputNameCombo.getSelectedItem())) {
                return true;
            }
        }
        return false;
    }

    public void saveCampaign() {
        if (isEditable && hasChanged()) {
            itsCampaignInfo.itsName = (String) inputNameCombo.getSelectedItem();
            try {
                OtdbRmi.getRemoteCampaign().saveCampaign(itsCampaignInfo);
            } catch (RemoteException ex) {
                logger.debug("ObservationPanel: Error saving changed campaignInfo" + ex);            }
        }
    }

    private MainFrame       itsMainFrame    = null;
    private String          itsCampaignName = "";
    private jOTDBtree       itsTree=null;
    private jCampaignInfo   itsCampaignInfo = null;
    private boolean         isEditable=false;

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        jLabel3 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        inputTitle = new javax.swing.JTextField();
        inputPI = new javax.swing.JTextField();
        inputCO_I = new javax.swing.JTextField();
        inputContact = new javax.swing.JTextField();
        jLabel6 = new javax.swing.JLabel();
        inputNameCombo = new javax.swing.JComboBox();
        inputNameTxt = new javax.swing.JTextField();

        setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jLabel1.setText("Name");
        add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 60, 80, -1));

        jLabel2.setText("Title");
        add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 90, 80, -1));

        jLabel3.setText("Principal Investigator(s)");
        add(jLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 120, -1, -1));

        jLabel4.setText("Co Investigator(s)");
        add(jLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 150, -1, -1));

        jLabel5.setText("Contact");
        add(jLabel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 180, -1, -1));

        inputTitle.setEditable(false);
        add(inputTitle, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 90, 360, -1));

        inputPI.setEditable(false);
        add(inputPI, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 120, 360, -1));

        inputCO_I.setEditable(false);
        add(inputCO_I, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 150, 360, -1));

        inputContact.setEditable(false);
        add(inputContact, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 180, 360, -1));

        jLabel6.setFont(new java.awt.Font("Tahoma", 1, 14));
        jLabel6.setText("Campaign Info");
        add(jLabel6, new org.netbeans.lib.awtextra.AbsoluteConstraints(230, 10, -1, -1));

        add(inputNameCombo, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 60, 360, -1));

        inputNameTxt.setText("jTextField1");
        add(inputNameTxt, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 60, 360, -1));
    }// </editor-fold>//GEN-END:initComponents



    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField inputCO_I;
    private javax.swing.JTextField inputContact;
    private javax.swing.JComboBox inputNameCombo;
    private javax.swing.JTextField inputNameTxt;
    private javax.swing.JTextField inputPI;
    private javax.swing.JTextField inputTitle;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    // End of variables declaration//GEN-END:variables

}
