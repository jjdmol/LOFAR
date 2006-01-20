/*
 * MainPanel.java
 *
 * Created on January 13, 2006, 2:58 PM
 */

package nl.astron.lofar.otb.panels;

import nl.astron.lofar.otb.*;
import nl.astron.lofar.otb.util.*;
import org.apache.log4j.Logger;

/**
 *
 * @author  blaakmeer
 */
public class MainPanel extends javax.swing.JPanel 
                       implements IPluginPanel {

    static Logger logger = Logger.getLogger(MainPanel.class);
    static String name = "Main";

    /** Creates new form BeanForm */
    public MainPanel() {
        initComponents();
        initialize();
    }
        
    public void initialize() {
        buttonPanel1.addButton("Query Panel");
        buttonPanel1.addButton("New");
        buttonPanel1.addButton("Delete");
        buttonPanel1.addButton("Duplicate");
        buttonPanel1.addButton("Info");
        buttonPanel1.addButton("Quit");
    }

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
        
    }
    
    public String getFriendlyName() {
        return getFriendlyNameStatic();
    }

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
        buttonPanel1 = new nl.astron.lofar.otbcomponents.ButtonPanel();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        tablePanel1 = new nl.astron.lofar.otbcomponents.TablePanel();
        tablePanel2 = new nl.astron.lofar.otbcomponents.TablePanel();
        tablePanel3 = new nl.astron.lofar.otbcomponents.TablePanel();
        tablePanel4 = new nl.astron.lofar.otbcomponents.TablePanel();
        tablePanel5 = new nl.astron.lofar.otbcomponents.TablePanel();

        setLayout(new java.awt.BorderLayout());

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

        jTabbedPane1.addTab("tab1", tablePanel1);

        jTabbedPane1.addTab("tab2", tablePanel2);

        jTabbedPane1.addTab("tab3", tablePanel3);

        jTabbedPane1.addTab("tab4", tablePanel4);

        jTabbedPane1.addTab("tab5", tablePanel5);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

    }
    // </editor-fold>//GEN-END:initComponents

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt);
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private MainFrame itsMainFrame;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JTabbedPane jTabbedPane1;
    private nl.astron.lofar.otbcomponents.TablePanel tablePanel1;
    private nl.astron.lofar.otbcomponents.TablePanel tablePanel2;
    private nl.astron.lofar.otbcomponents.TablePanel tablePanel3;
    private nl.astron.lofar.otbcomponents.TablePanel tablePanel4;
    private nl.astron.lofar.otbcomponents.TablePanel tablePanel5;
    // End of variables declaration//GEN-END:variables
    
}
