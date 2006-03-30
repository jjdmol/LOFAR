package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import javax.swing.JPanel;
import nl.astron.lofar.mac.apl.gui.jrsp.Board;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.panels.IPluginPanel;

/**
 *
 * @author  balken
 */
public class MainPanel extends JPanel implements IPluginPanel, Runnable
{
    /** MainFrame */
    private MainFrame mainFrame;
    
    /** Board */
    private Board board;
    
    /** Refresh Thread - Thread used to refresh the displayed panel. */
    private Thread refreshThread;
    
    /** RefreshRates - used to store the refreshRates of the different panels. */
    private int[] refreshRates;
    
    /** Name */
    private static final String name = "jRSP";
    
    /** Changed */
    private boolean changed;
    
    /** 
     * Creates new form MainPanel.
     */
    public MainPanel() 
    {
        mainFrame = null;
        board = null;
        refreshThread = null;
        changed = false;
        
        initComponents();        
        controlPanel.setMainPanel(this);
        
        refreshRates = new int[jTabbedPane.getTabCount()];
    }
    
    /**
     * Initializes the plugin and receives a refrence to the mainFrame.
     * @param   mainFrame   Refrence to the MainFrame.
     * @return  true        Letting the MainFrame know there are no problems.
     */
    public boolean initializePlugin(MainFrame mainFrame)
    {
        this.mainFrame = mainFrame;
        
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
    public void checkChanged()
    {
        
    }
    
    /**
     * This method is called to update the current panel, for example when the 
     * Board has changed.
     */
    public void updateCurrentPanel()
    {
        Class selectedClass = jTabbedPane.getSelectedComponent().getClass();
        
        // StatusPanel
        // @TODO - change which status is returned based on the selected board!
        if(selectedClass.equals(StatusPanel.class))
        {
           ((StatusPanel)jTabbedPane.getSelectedComponent()).initFields(board.getStatus()[0]);
        }
    }
    
    /**
     * This method is invoked when the hostname is entered in the controlpanel
     * and makes a new Board instance based on the hostname.
     * @param   hostname    The hostname which will be used in the connection.
     * @param   refreshRate The number of seconds between the updates of current panel.
     *
     * @TODO Implement a error message when the connection fails.
     */
    public void initBoard(String hostname, int refreshRate)
    {
        // Do nothing if the hostname and refreshrate haven't changed.
        if(board != null && hostname.equals(board.getHostname()) && refreshRate == refreshRates[jTabbedPane.getSelectedIndex()])
        {
            return;
        }
        
        // The refreshrate should be 0 or higher.
        // @TODO add error message
        if(refreshRate < 0)
        {
            return;
        }        
        
        // If the board is null; there hasn't been a board yet, construct a new
        // board. Else change the current board.
        if(board == null)
        {
            System.out.println("new");
            board = new Board(hostname);
        }
        else if(!hostname.equals(board.getHostname()))
        {
            System.out.println("not equal");
            board.setHostname(hostname);
        }
                
        refreshRates[jTabbedPane.getSelectedIndex()] = refreshRate;
        if(refreshRate == 0)
        {
            updateCurrentPanel();
        }
        else
        {
            // start thread that updates the board.
            startRefreshThread();
        }
    }
    
    /**
     * Starts the refresh thread.
     */
    public void startRefreshThread()
    {
        if(refreshThread == null)
        {
            refreshThread = new Thread(this, "RefreshThread");
            refreshThread.start();
        }
    }
    
    /**
     * Stops the refresh thread.
     */
    public void stopRefreshThread()
    {
        refreshThread = null;
    }
    
    /**
     * Run method.
     */
    public void run()
    {
        Thread thread = Thread.currentThread();
        while(thread == refreshThread)
        {
            try
            {
                Thread.sleep(refreshRates[jTabbedPane.getSelectedIndex()] * 1000);
            }
            catch(InterruptedException e)
            {
                // Just ignore it!
            }
            updateCurrentPanel();
            System.out.println("hallo kijk mij eens wat doen!");
        }
    }
        
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        treePanel = new nl.astron.lofar.sas.otbcomponents.TreePanel();
        jTabbedPane = new javax.swing.JTabbedPane();
        statusPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.StatusPanel();
        controlPanel = new nl.astron.lofar.mac.apl.gui.jrsp.panels.ControlPanel();

        jTabbedPane.addTab("Status", statusPanel);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(treePanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 136, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jTabbedPane))
            .add(controlPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 3071, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jTabbedPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 581, Short.MAX_VALUE)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, treePanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 581, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(controlPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.ControlPanel controlPanel;
    private javax.swing.JTabbedPane jTabbedPane;
    private nl.astron.lofar.mac.apl.gui.jrsp.panels.StatusPanel statusPanel;
    private nl.astron.lofar.sas.otbcomponents.TreePanel treePanel;
    // End of variables declaration//GEN-END:variables
    
}
