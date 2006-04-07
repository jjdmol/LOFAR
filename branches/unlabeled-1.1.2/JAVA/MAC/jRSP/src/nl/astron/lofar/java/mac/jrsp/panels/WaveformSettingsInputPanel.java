package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JPanel;
import javax.swing.event.EventListenerList;

/**
 * The input panel for the WaveformSettingsPanel.
 *
 * @author  balken
 */
public class WaveformSettingsInputPanel extends JPanel implements ActionListener
{
    /** Used to store the listeners to this class */
    private EventListenerList listenerList;
    
    /** 
     * Creates new form WaveformSettingsInputPanel.
     */
    public WaveformSettingsInputPanel() 
    {
        listenerList = new EventListenerList();
        
        initComponents();
        
        btnSubmit.addActionListener(this);
    }
    
    /**
     * Returns the value of the mode textfield.
     */
    public String getMode()
    {
        return txtMode.getText();
    }
    
    /**
     * Returns the value of the frequency textfield.
     */
    public String getFrequency()
    {
        return txtFrequency.getText();
    }
    
    /**
     * Returns the value of the amplitude textfield.
     */
    public String getAmplitude()
    {
        return txtAmplitude.getText();
    }
        
    /**
     * Invoked when a action occurs; when btnSubmit is pressed.
     */
    public void actionPerformed(ActionEvent e)
    {
        fireActionPerformed(e);
    }
    
    /**
     * Adds listener to the listeners list.
     */
    public void addActionListener(ActionListener l)
    {
        listenerList.add(ActionListener.class, l);
    }
    
    /**
     * Removes listener from the listeners list.
     */
    public void removeActionListener(ActionListener l)
    {
        listenerList.remove(ActionListener.class, l);
    }
    
    /**
     * Notify all listeners that a action had been performed.
     */
    public void fireActionPerformed(ActionEvent e) 
    {
            // Guaranteed to return a non-null array
            Object[] listeners = listenerList.getListenerList();

            // Process the listeners last to first, notifying
            // those that are interested in this event
            for (int i = listeners.length-2; i>=0; i-=2) 
            {
                    if (listeners[i]==ActionListener.class) 
                    {
                            ((ActionListener)listeners[i+1]).actionPerformed(e);
                    }
            }
    }
        
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        lblMode = new javax.swing.JLabel();
        lblFrequency = new javax.swing.JLabel();
        lblAmplitude = new javax.swing.JLabel();
        txtMode = new javax.swing.JTextField();
        txtFrequency = new javax.swing.JTextField();
        txtAmplitude = new javax.swing.JTextField();
        btnSubmit = new javax.swing.JButton();

        setBorder(javax.swing.BorderFactory.createTitledBorder("Input"));
        lblMode.setText("Mode");

        lblFrequency.setText("Frequency");

        lblAmplitude.setText("Amplitude");

        btnSubmit.setText("OK");

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(lblMode)
                            .add(lblFrequency)
                            .add(lblAmplitude))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                            .add(txtAmplitude)
                            .add(txtFrequency)
                            .add(txtMode, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 97, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, btnSubmit))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtMode, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblMode))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtFrequency, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblFrequency))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtAmplitude, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblAmplitude))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(btnSubmit)
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
        
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton btnSubmit;
    private javax.swing.JLabel lblAmplitude;
    private javax.swing.JLabel lblFrequency;
    private javax.swing.JLabel lblMode;
    private javax.swing.JTextField txtAmplitude;
    private javax.swing.JTextField txtFrequency;
    private javax.swing.JTextField txtMode;
    // End of variables declaration//GEN-END:variables
}