/*
 * ParamViewPanel.java
 *
 * Created on 26 januari 2006, 15:47
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.util.Vector;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.PlotPanel;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.SharedVars;
import org.apache.log4j.Logger;

/**
 *
 * @author  pompert
 */
public class ParmDBPlotPanel extends javax.swing.JPanel {
    
    static Logger logger = Logger.getLogger(ParameterViewPanel.class);
    JScrollPane legendPane;
    PlotPanel paramPanel;
    /** Creates new form BeanForm based upon aParameter
     *
     * @params  aParam   Param to obtain the info from
     *
     */
    public ParmDBPlotPanel(MainFrame aMainFrame,String paramName) {
        initComponents();
        itsMainFrame = aMainFrame;
        
        itsParamName = paramName;
        
        initPanel(paramName);
    }
    
    /** Creates new form BeanForm */
    public ParmDBPlotPanel() {
        initComponents();
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            
            
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    private void initPanel(String aParamName) {
        if (aParamName != null) {
            if(aParamName.equalsIgnoreCase("ParmDB")){
                itsParamName = "*";
            }else{
                itsParamName=aParamName.substring(7);
                itsParamName += "*";
            }
            
            paramPanel = new PlotPanel();
            try{
                String[] passToDataAccess = new String[7];
                
                Vector paramValues;
                paramValues = SharedVars.getJParmFacade().getRange(itsParamName);
                double startx = Double.parseDouble(paramValues.get(0).toString());
                double endx =Double.parseDouble(paramValues.get(1).toString());
                double starty = Double.parseDouble(paramValues.get(2).toString());
                double endy = Double.parseDouble(paramValues.get(3).toString());
                int numx = Integer.parseInt("5");
                int numy = Integer.parseInt("5");
                
                passToDataAccess[0] = itsParamName;
                passToDataAccess[1] = ""+startx;
                passToDataAccess[2] = ""+endx;
                passToDataAccess[3] = ""+numx;
                passToDataAccess[4] = ""+starty;
                passToDataAccess[5] = ""+endy;
                passToDataAccess[6] = ""+numy;
                
                paramPanel.createPlot(PlotConstants.PLOT_XYLINE,true,passToDataAccess);
                plotPanel.removeAll();
                plotPanel.add(paramPanel,BorderLayout.CENTER);
                legendPane = new JScrollPane(paramPanel.getLegendForPlot());
                legendPane.setPreferredSize(new Dimension(paramPanel.getLegendForPlot().getWidth()-20,120));
                legendPane.setBackground(Color.WHITE);
                legendPane.getViewport().setBackground(Color.WHITE);
                plotPanel.add(legendPane,BorderLayout.SOUTH);
                
                
            }catch(Exception ex){
                JOptionPane.showMessageDialog(itsMainFrame, ex.getMessage(),
                        "Error detected",
                        JOptionPane.ERROR_MESSAGE);
                logger.error("Plotter created an exception :"+ex.getMessage(),ex);
            }
        } else {
            logger.debug("ERROR:  no Param Name given");
        }
    }
    public void setParam(String aParam) {
        if (aParam != null) {
            itsParamName=aParam;
            setParamName(aParam);
            initPanel(aParam);
        } else {
            logger.debug("No param supplied");
        }
    }
    
    /** Returns the Given Name for this Param */
    public String getParamName() {
        return this.ParamNameText.getText();
    }
    
    private void setParamName(String aS) {
        this.ParamNameText.setText(aS);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableParamName(boolean enabled) {
        this.ParamNameText.setEnabled(enabled);
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.ParamApplyButton.setEnabled(enabled);
        this.ParamCancelButton.setEnabled(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.ParamApplyButton.setVisible(visible);
        this.ParamCancelButton.setVisible(visible);
    }
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableParamName(enabled);
        
    }
    
    
    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean hasChanged = false;
        
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jScrollPane1 = new javax.swing.JScrollPane();
        jTextArea1 = new javax.swing.JTextArea();
        ParamNameLabel = new javax.swing.JLabel();
        ParamNameText = new javax.swing.JTextField();
        ParamCancelButton = new javax.swing.JButton();
        ParamApplyButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        plotPanel = new javax.swing.JPanel();
        jScrollPane2 = new javax.swing.JScrollPane();
        jTextArea2 = new javax.swing.JTextArea();

        jTextArea1.setColumns(20);
        jTextArea1.setRows(5);
        jScrollPane1.setViewportView(jTextArea1);

        setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        ParamNameLabel.setText("Name :");
        add(ParamNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 50, 80, -1));

        ParamNameText.setText("None");
        ParamNameText.setToolTipText("Name for this Node");
        ParamNameText.setEnabled(false);
        ParamNameText.setMaximumSize(new java.awt.Dimension(440, 19));
        ParamNameText.setMinimumSize(new java.awt.Dimension(440, 19));
        ParamNameText.setPreferredSize(new java.awt.Dimension(440, 19));
        add(ParamNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 50, 430, -1));

        ParamCancelButton.setText("Cancel");
        ParamCancelButton.setEnabled(false);
        ParamCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamCancelButtonActionPerformed(evt);
            }
        });

        add(ParamCancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(470, 640, -1, -1));

        ParamApplyButton.setText("Apply");
        ParamApplyButton.setEnabled(false);
        ParamApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamApplyButtonActionPerformed(evt);
            }
        });

        add(ParamApplyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(550, 640, 70, -1));

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("ParmDB Plot Panel");
        add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(114, 10, 320, 20));

        plotPanel.setLayout(new java.awt.BorderLayout());

        plotPanel.setBackground(new java.awt.Color(255, 255, 255));
        add(plotPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 70, 610, 450));

        jTextArea2.setBackground(javax.swing.UIManager.getDefaults().getColor("scrollbar"));
        jTextArea2.setColumns(20);
        jTextArea2.setEditable(false);
        jTextArea2.setRows(5);
        jTextArea2.setText("To zoom: Click and hold the left mouse button and select a rectangle.\nTo reset the zoom: Press CTRL-LeftMouseButton to reset the zoom.\nTo change colors/etc: Double-Click on a line in the legend.\nTo change plot/axis labels and tics/etc: Click on an axis or title and press the right mouse button.");
        jScrollPane2.setViewportView(jTextArea2);

        add(jScrollPane2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 530, 610, 80));

    }// </editor-fold>//GEN-END:initComponents
    
    private void ParamApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_ParamApplyButtonActionPerformed
    
    private void ParamCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamCancelButtonActionPerformed
        initPanel(itsParamName);
    }//GEN-LAST:event_ParamCancelButtonActionPerformed
    
    private MainFrame  itsMainFrame;
    private String itsParamName;
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton ParamApplyButton;
    private javax.swing.JButton ParamCancelButton;
    private javax.swing.JLabel ParamNameLabel;
    private javax.swing.JTextField ParamNameText;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTextArea jTextArea1;
    private javax.swing.JTextArea jTextArea2;
    private javax.swing.JPanel plotPanel;
    // End of variables declaration//GEN-END:variables
    
    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList listenerList =  null;
    
    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {
        
        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {
        
        listenerList.remove(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {
        
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed(event);
            }
        }
    }
    
    
    
    
    
}
