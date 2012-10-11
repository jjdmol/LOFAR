/*
 * MainPanel.java
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

import java.awt.Cursor;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import nl.astron.lofar.java.mac.jrsp.Board;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.panels.IPluginPanel;
import org.apache.log4j.Logger;

/**
 * The MainPanel is the GUI entrypoint for the jRSP project. It has a ListPanel
 * that lists all the available boards, a ControlPanel for editing the hostname
 * and refreshrate, and a tabbedpane containing the detailed panels for
 * controlling the boards.
 * 
 * To support refresh a thread is needed and therefore MainPanel implements
 * Runnable. The functions startRefresh and stopRefresh are used
 * to start and stop the thread. When the thread is started it will, depending
 * on the refreshrate, refresh the current panel using the method
 * updateCurrentPabel.
 * 
 * The MainPanel implements a ListSelectionListener to react to changes in the
 * ListPanel and a ChangeListener to react to changes in the tabbedpane.
 * 
 * 
 * @author balken
 */
public class MainPanel extends JPanel implements IPluginPanel, Runnable
{
    /** MainFrame */
    private MainFrame itsMainFrame;
    
    /** Board */
    private Board itsBoard;
    
    /** Refresh Thread - Thread used to refresh the displayed panel. */
    private Thread refreshThread;
    
    /** RefreshRates - used to store the refreshRates of the different panels. */
    private int[] refreshRates;
    
    /** Name */
    private static final String name = "jRSP";
    
    /** Changed */
    private boolean changed;
            
    /** Logger */
    private Logger itsLogger;
        
    /** 
     * Creates new form MainPanel.
     */
    public MainPanel() 
    {
        itsLogger = Logger.getLogger(getClass());
        itsLogger.info("Constructor");
        
        itsMainFrame = null;
        itsBoard = new Board();
        refreshThread = null;
        changed = false;      
        
        initComponents();        
                
        /*
         * Call the init() of every component in the jTabbedPane.
         */
        for (int i = 0; i < jTabbedPane.getComponentCount(); i++) {
            ((ITabPanel)jTabbedPane.getComponentAt(i)).init(this);
        }
                        
        refreshRates = new int[jTabbedPane.getTabCount()];
    }
    
    /**
     * Initializes the plugin and receives a refrence to the itsMainFrame.
     * 
     * @param itsMainFrame   Refrence to the MainFrame.
     * @return true        Letting the MainFrame know there are no problems.
     */
    public boolean initializePlugin(MainFrame mainFrame)
    {
        this.itsMainFrame = mainFrame;
        
        return true;
    }
    
    /**
     * Returns the friendly name of this panel.
     * @return  name    The friendly name.
     */
    public String getFriendlyName()
    {
        return name;
    }
    
    /**
     * Same as the above function except this function is static.
     * @return  name    The name in a static friendly way.
     */
    public static String getFriendlyNameStatic()
    {
        return name;
    }
    
    /**
     * Returns if this panel has changed,
     * @return  changed
     */
    public boolean hasChanged()
    {
        return changed;
    }
    
    /**
     * Set the changed value.
     * @param   changed     The new value for changed.
     */
    public void setChanged(boolean changed)
    {
        this.changed = changed;
    }
    
    /**
     * It is has to be implemented...
     */
    public void checkChanged() { }
      
    /**
     * Returns the itsBoard.
     * 
     * @return itsBoard
     */
    public Board getBoard()
    {
        return itsBoard;
    }
    
    /**
     * Returns the index of the current selected itsBoard in the ListPanel.
     * 
     * @return index
     */
    public int getSelectedBoardIndex()
    {
        return listPanel.getSelectedIndex();
    }
    
    /**
     * Sets the refresh rate of the currently selected panel.
     * @param   refreshRat      The new refreshRate for the current panel.
     */
    public void setCurrentRefreshRate(int refreshRate)
    {
        refreshRates[jTabbedPane.getSelectedIndex()] = refreshRate;
    }  
    
    /**
     * Updates or initializes the board by changing the hostname. This method
     * first checks if the hostname is legal (not equal to ""). The hostname
     * should be the same as the current, that would result in a unnecassery
     * connection.
     */
    public void updateBoard()
    {
        /*
         * Retrieve hostname.
         */
        String hostname = controlPanel.getHostname();
        
        /*
         * Perform checks before connecting: 
         * - hostname can't be empty
         * - board should'nt be connected with the same host(name). this check
         *   prevents the app to reconnect to the board when switching tabs.
         */
        if ("".equals(hostname.trim())) {            
            JOptionPane.showMessageDialog(this, "The hostname can't be empty.", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }

        if (itsBoard.isConnected() && hostname.equals(itsBoard.getHostname())) {            
            return;
        }
        
        /*
         * Check if the board is already connected. If so: disconnect.
         */
        if (itsBoard.isConnected()) {
            itsBoard.disconnect();
        }
        
        /*
         * Try to connect. On error display message. On success fill the 
         * listpanel.
         */
        try {
            // display wait cursor to make clear we're connecting
            setCursor(new Cursor(Cursor.WAIT_CURSOR));
            itsBoard.connect(hostname);
            // board is connected, so display default cursor
            setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
            
            /*
             * Construct a String array for the listpanel.
             */
            int nofBoards = itsBoard.getNrRSPBoards();
            String[] listItems = new String[nofBoards];
            for (int i = 0; i < nofBoards; i++) {
                listItems[i] = Integer.toString(i);
            }
        
            listPanel.newList(listItems);            
        } catch (Exception e) {
            // show default cursor, identifying we're not connecting anymore
            setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
            
            JOptionPane.showMessageDialog(this, "Could not connect to the board.", "Error", JOptionPane.ERROR_MESSAGE);
            /*
             * Connection failed. Update panels to disable everything.
             */
            listPanel.newList(new String[0]); // empty listPanel
            ((ITabPanel)jTabbedPane.getSelectedComponent()).update(ITabPanel.REQUIRED_UPDATE);
        } 
    }    
    
    /**
     * This method is called to update the current panel.
     * Note: Board has to be set, to be using this function! Quick check is
     * performed at the beginning of the method.
     */
    public void updateCurrentPanel(int updateType)
    {
        /*
         * The itsBoard has to be connected to update the panels.
         */
        if (!itsBoard.isConnected())
        {
            return;
        }
        
        /** The index of the selected board in the list panel. */
        int index = listPanel.getSelectedIndex();
        
        if(index == -1)
        {
            /* 
             * When the index is -1. The selected index will be changed. That will
             * generate a valueChangedEvent. Through the method valueChanged, that
             * responses to this event, this (updateCurrentPanel() function is 
             * called again. 
             */
            index = 0;
            listPanel.setSelectedIndex(0);
            return;
        }
        
        /*
         * Update the selected panel.
         */
        ((ITabPanel) jTabbedPane.getSelectedComponent()).update(updateType);
    }
    
    /**
     * Updates the list of listItems of the listPanel.
     * Note: Board has to be set, before you can use this function!
     */
    private void updateListPanel()
    {
        String[] listItems = new String[itsBoard.getNrRSPBoards()];
        for(int i=0; i<listItems.length; i++)
        {
            listItems[i] = Integer.toString(i);
        }
        listPanel.newList(listItems);
        
        // Sets the selected index to 0 on default.
        listPanel.setSelectedIndex(0);
    }
    
        
    /**
     * Starts the refresh thread.
     */
    private void startRefresh()
    {
        if (refreshThread == null)
        {
            refreshThread = new Thread(this, "RefreshThread");
            refreshThread.start();    
        }
    }
    
    /**
     * Stops the refresh thread.
     */
    private void stopRefresh()
    {
        refreshThread = null;
    }
    
    /**
     * Run method.
     */
    public void run()
    {        
        boolean updated = false; // true on updates following the first.
        
        Thread thread = Thread.currentThread();
        /*
         * First perform the update of the panel and then wait. In this order
         * the user doesn't need to wait for the first batch of data.
         */
        while(thread == refreshThread)
        {            
            /*
             * The first and the following updates are of different types.
             * Updated is set to true after the first run. That way on the next
             * update we know we got to refresh and don't need to update every-
             * thing.
             */
            if (!updated) {
                updateCurrentPanel(ITabPanel.REQUIRED_UPDATE);
                updated = true;
            } else {
                updateCurrentPanel(ITabPanel.REFRESH_UPDATE);
            }
            
            /*
             * If the refreshRate is smaller than 1, call stopRefresh
             */
            if (refreshRates[jTabbedPane.getSelectedIndex()] < 1)
            {
                stopRefresh();
            }
            
            try
            {
                Thread.sleep(refreshRates[jTabbedPane.getSelectedIndex()] * 1000);
            }
            catch(InterruptedException e)
            {
                // Just ignore it!
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
        jTabbedPane = new javax.swing.JTabbedPane();
        statusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.StatusPanel();
        subbandStatsPanel = new nl.astron.lofar.java.gui.mac.jrsp.subbandstats.SubbandStatsPanel();
        itsRSPControlPanel = new nl.astron.lofar.java.gui.mac.jrsp.control.RSPControlPanel();
        beamletStatsPanel1 = new nl.astron.lofar.java.gui.mac.jrsp.beamletstats.BeamletStatsPanel();
        controlPanel = new nl.astron.lofar.java.gui.mac.jrsp.ControlPanel();
        listPanel = new nl.astron.lofar.java.gui.mac.jrsp.ListPanel();

        jTabbedPane.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                jTabbedPaneStateChanged(evt);
            }
        });

        jTabbedPane.addTab("Status", statusPanel);

        jTabbedPane.addTab("Subband Statistics", subbandStatsPanel);

        jTabbedPane.addTab("Control", itsRSPControlPanel);

        jTabbedPane.addTab("Beamlet Statistics", beamletStatsPanel1);

        controlPanel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                controlPanelActionPerformed(evt);
            }
        });

        listPanel.setTitle("Boards");
        listPanel.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                listPanelValueChanged(evt);
            }
        });

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(listPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 139, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jTabbedPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 1125, Short.MAX_VALUE))
            .add(controlPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 1270, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(12, 12, 12)
                        .add(jTabbedPane, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 577, Short.MAX_VALUE))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, listPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 589, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(controlPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
        );
    }// </editor-fold>//GEN-END:initComponents

    /**
     * Invoked when another tab is selected.
     * @param   evt     ChangeEvent
     */
    private void jTabbedPaneStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_jTabbedPaneStateChanged
        /*
         * Refresh can be only possible if the itsBoard is connected. If not, the 0 
         * in the refreshrate textfield looks silly.
         */
        if (!itsBoard.isConnected())
        {
            ((ITabPanel)jTabbedPane.getSelectedComponent()).enablePanel(false);
            return;
        }
        
        /*
         * First check if there the refreshThread is running. If so: KILL IT!
         */
        refreshThread = null;
                
        /*
         * If the refresh rate of the selected panel is higher than 0, start the
         * refreshThread. If refreshrate is 0, then the current panel isn't
         * updated!
         */
        if (refreshRates[jTabbedPane.getSelectedIndex()] > 0)
        {
            startRefresh();
        }
        
        /*
         * Update controlPanel.
         */         
        controlPanel.setRefreshRate(refreshRates[jTabbedPane.getSelectedIndex()]);
    }//GEN-LAST:event_jTabbedPaneStateChanged

    /**
     * Called when an action occured on the control panel.
     * @param   evt     ActionEvent
     */
    private void controlPanelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_controlPanelActionPerformed
        switch(controlPanel.getSourceAction(evt.getSource()))
        {
            case ControlPanel.UPDATE:
                /*
                 * Connect/update the itsBoard and the current panel.
                 */
                updateBoard();
                /*
                 * Only continue if the board is connected.
                 */
                if (itsBoard.isConnected()) {
                    controlPanel.setRefreshRate(0);
                    controlPanel.setConnectButtonTitle(ControlPanel.TITLE_REFRESH);
                    /*
                     * By calling startRefresh we make sure the buttons don't get 
                     * "locked" because we are waiting for the updateCurrentPanel()
                     * call that takes time.
                     */
                    startRefresh();
                } else {
                    controlPanel.setConnectButtonTitle(ControlPanel.TITLE_CONNECT);
                }
                break;
            case ControlPanel.REFRESH:
                /*
                 * Connect to itsBoard and update according to the refreshrate.
                 * Check if refresh rate is valid. If so set the refreshrate in 
                 * refreshRates[] and call startRefresh().
                 */
                if(controlPanel.getRefreshRate() < 0)
                {
                    /*
                     * The RefreshRate isn't a valid number (either error (-1) 
                     * or negative number). Display error and quit.
                     */
                    JOptionPane.showMessageDialog(this, "Refreshrate may only contain positive numbers and zero.", "Error", JOptionPane.ERROR_MESSAGE);
                    return;
                }                
                updateBoard();
                /*
                 * Only continue if the board is connected.
                 */
                if (itsBoard.isConnected()) {
                    refreshRates[jTabbedPane.getSelectedIndex()] = controlPanel.getRefreshRate();
                    controlPanel.setConnectButtonTitle(ControlPanel.TITLE_REFRESH);
                    controlPanel.setStopButtonEnabled(true);
                    startRefresh();
                } else {
                    controlPanel.setConnectButtonTitle(ControlPanel.TITLE_CONNECT);
                    controlPanel.setStopButtonEnabled(false);
                }                    
                break;
            case ControlPanel.STOP:
                /*
                 * Stop the refresh.
                 */
                stopRefresh();
                controlPanel.setStopButtonEnabled(false);
                break;
            default:
                /*
                 * Do nothing.
                 */
                break;                
        }        
    }//GEN-LAST:event_controlPanelActionPerformed

    /**
     * Invoked when another board is selected on the listPanel.
     * @param   evt     ListSelectionEvent
     */    
    private void listPanelValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_listPanelValueChanged
        updateCurrentPanel(ITabPanel.REQUIRED_UPDATE);           
    }//GEN-LAST:event_listPanelValueChanged
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.java.gui.mac.jrsp.beamletstats.BeamletStatsPanel beamletStatsPanel1;
    private nl.astron.lofar.java.gui.mac.jrsp.ControlPanel controlPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.control.RSPControlPanel itsRSPControlPanel;
    private javax.swing.JTabbedPane jTabbedPane;
    private nl.astron.lofar.java.gui.mac.jrsp.ListPanel listPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.StatusPanel statusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.subbandstats.SubbandStatsPanel subbandStatsPanel;
    // End of variables declaration//GEN-END:variables
    
}
