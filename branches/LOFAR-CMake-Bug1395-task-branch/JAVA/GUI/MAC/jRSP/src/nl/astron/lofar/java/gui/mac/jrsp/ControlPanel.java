/*
 * ControlPanel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.java.gui.mac.jrsp;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JPanel;
import javax.swing.event.EventListenerList;

/**
 * The ControlPanel is a component used by the MainPanel to control the board.
 *
 * @author  balken
 */
public class ControlPanel extends JPanel implements ActionListener
{
    /** "Constant" variables used to identify the action required. When an
        action has occured. */
    public static final int STOP = 0;
    public static final int UPDATE = 1;
    public static final int REFRESH = 2;
    
    /** Variables used to identify the title that should be displayed on the
        connect-button (btnConnect). */
    public static final int TITLE_CONNECT = 0;
    public static final int TITLE_REFRESH = 1;
    
    /** List of ActionListeners listening to this object. */
    private EventListenerList listenerList; 
    
    /** 
     * Creates new form ControlPanel.
     */
    public ControlPanel() 
    {
        listenerList = new EventListenerList();
        
        initComponents();
        
        txtHostname.addActionListener(this);
        txtRefreshRate.addActionListener(this);
        btnConnect.addActionListener(this);
        btnStop.addActionListener(this);
        
        /*
         * Disable stop-button on default.
         */
        setStopButtonEnabled( false );
    }
    
    public void addActionListener(ActionListener l)
    {
        listenerList.add(ActionListener.class, l);
    }
    
    public void removeActionListener(ActionListener l)
    {
        listenerList.remove(ActionListener.class, l);
    }
    
    /**
     * Notify all listeners of the action that has occured.
     */
    public void fireActionPerformed(ActionEvent e)
    {
        Object[] listeners = listenerList.getListenerList();
        
        for (int i = listeners.length - 2; i >= 0; i -= 2)
        {
            if (listeners[i] == ActionListener.class)
            {
                ((ActionListener) listeners[i+1]).actionPerformed(e);
            }
        }       
    }
          
    /**
     * Invoked when an action occurs.
     * @param   e                       ActionEvent
     */
    public void actionPerformed(ActionEvent e)
    {        
        fireActionPerformed(e);
    }
    
    /**
     * Returns the action required by the source object.
     * This method is called by the MainPanel, when an action occured and wil 
     * return a int representing the action that has to performed: stopping
     * the refresh of the current panel in the MainPanel or updating the panel.
     * @param   o   Object that has "thrown" the ActionEvent, received by the MainPanel.
     * @return      A int representing the action that has to be taken.
     */
    public int getSourceAction(Object o)
    {
       if(o.equals(btnStop)) 
       {
           // the stop button has been pressed
           return STOP;
       } 
       else if("".equals(txtRefreshRate.getText().trim())) 
       {
           // the refreshrate textbox is empty -> single update
           return UPDATE;
       }
       else
       {
           // the refreshrate was set but it can be wrong data!
           return REFRESH;
       }
    }
        
    /**
     * Return the value of txtHostname.
     * @return      txtHostname.getText();
     */
    public String getHostname()
    {
        return txtHostname.getText();
    }
    
    /**
     * Sets the value of txtHostname.
     * @param   s   The new value for txtHostname
     */
    public void setHostname(String hostname)
    {
        txtHostname.setText(hostname);
    }
    
    /**
     * Return the value of txtRefreshrate. If txtRefreshrate doesn't hold a
     * valid number, it will return -1, indicating the error.
     */
    public int getRefreshRate()
    {        
        try
        {
            return Integer.parseInt(txtRefreshRate.getText());
        }
        catch(NumberFormatException e)
        {
            return -1;
        }
    }
    
    /**
     * Sets the text of txtRefreshrate.
     * @param   refreshRate     The refreshRate to set
     */
    public void setRefreshRate(int refreshRate)
    {
        txtRefreshRate.setText(Integer.toString(refreshRate));
    }
    
    /**
     * Changes the title on the connect button based on the title passed.
     * @param title          The title that determines if "Refresh" or
     *                          "Connect" should be shown.
     */
    public void setConnectButtonTitle(int title) {
        switch(title) {
            case TITLE_CONNECT:
                btnConnect.setText("Connect");
                break;
            case TITLE_REFRESH:
                btnConnect.setText("Refresh");
        }
    }
    
    /**
     * Enables or disables the stop button.
     * @param   b           Boolean value. Enable when true, disable when false.
     */
    public void setStopButtonEnabled(boolean b) {
        btnStop.setEnabled(b);
    }    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        txtHostname = new javax.swing.JTextField();
        lblHostname = new javax.swing.JLabel();
        btnConnect = new javax.swing.JButton();
        lblRefreshRate = new javax.swing.JLabel();
        txtRefreshRate = new javax.swing.JTextField();
        btnStop = new javax.swing.JButton();

        setBorder(javax.swing.BorderFactory.createEtchedBorder());

        lblHostname.setText("Hostname:");

        btnConnect.setText("Connect");

        lblRefreshRate.setText("Refresh rate:");

        btnStop.setText("Stop");

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(lblHostname)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(txtHostname, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 113, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(6, 6, 6)
                .add(lblRefreshRate)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(txtRefreshRate, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 39, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(btnConnect)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(btnStop)
                .addContainerGap(38, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                .add(lblHostname)
                .add(txtHostname, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(lblRefreshRate)
                .add(txtRefreshRate, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(btnConnect)
                .add(btnStop))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton btnConnect;
    private javax.swing.JButton btnStop;
    private javax.swing.JLabel lblHostname;
    private javax.swing.JLabel lblRefreshRate;
    private javax.swing.JTextField txtHostname;
    private javax.swing.JTextField txtRefreshRate;
    // End of variables declaration//GEN-END:variables
    
}
