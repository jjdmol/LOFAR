/*
 * bb_gui.java
 *
 * Created on October 18, 2005, 2:42 PM
 */

package bb_gui;
import java.awt.Cursor;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import ptolemy.plot.Plot;



/**
 *
 * @author  coolen
 */
public class bb_gui extends javax.swing.JFrame {
 
    private String itsSelectedFile = "None";
    private String itsPrograms[]   = {"../../test/run.SolvePoly CDR",
                                      "../../test/run.SolveFlux CDR",
                                      "../../test/run.SolveAP CDR"};
    private String itsTableName[]  = {"BBS3SolvePoly","BBS3SolveFlux","BBS3SolveAP"};
    private int itsSelectedPlotType= 0;
    private String itsSelectedDemo ="";
    private Plot itsPlot;
    private Runtime thisSystem;
    private String workDir         ="";
    private boolean tabAdded       = false;
    private boolean dryRun         = false;
    private Viewer aV              = null;
    
    private volatile WaitForFileThread itsWaitThread;
    private volatile RunProgram itsProgramThread;
    
    private Cursor hourglassCursor = new Cursor(Cursor.WAIT_CURSOR);
    private Cursor normalCursor = new Cursor(Cursor.DEFAULT_CURSOR);

    // Database vars
    private String itsDBName="CDR";
    private Connection itsDatabase;
    private ResultSet itsResultSet;
    private Statement itsStatement;


    class configFilter extends javax.swing.filechooser.FileFilter {
        
        public boolean accept(File file) {
            String filename = file.getName();
            if (file.isDirectory()) {
                return true;
            } else {
                return filename.endsWith(".input");
            }
        }
        
        public String getDescription() {
            return "BBS configuration files";
        }
    }
      
    class RunProgram extends Thread {
        String itsJobName="";
        Process itsSystemCall;
        boolean running=false;
        boolean stopped=true;
        
        public void setJobName(String aS) {
            itsJobName=aS;
        }
       
        public void stopThread() {
            running=false;
            itsSystemCall.destroy();            
        }
        
        public boolean isStopped() {
            return stopped;
        }

        public void run() {

        String anOutputStr;
        String aCmd=workDir+itsPrograms[itsSelectedPlotType];
        
        running=true;
        stopped=false;
       
        ActiveProgramLabel.setText("Running program "+ itsJobName);
        try {
            itsSystemCall=thisSystem.exec(aCmd);
            DataInputStream anInputStream = new DataInputStream(itsSystemCall.getInputStream());
	    try {
		while ((anOutputStr = anInputStream.readLine()) != null && running) {
		    OutputPaneTextArea.append(anOutputStr+"\n");
		}
                stopped=true;
            } catch (IOException e) {
            }           
         } catch (IOException ex) {
            System.out.println("Error in command: " + aCmd + " : " + ex);
            System.exit(1);
	 }   
       }
    }
    
    class WaitForFileThread extends Thread {
        String theJobName;

        public void setJobTerminationName(String aJobName) {
            theJobName=aJobName;
        }
        
        
	public void run () {
            removeReadyFile(theJobName+".ready");
            
            itsProgramThread = new RunProgram();
            itsProgramThread.setJobName(theJobName);
            itsProgramThread.start();
            
            //start waitloop to check if program is ready
            waitReadyFiles(this,theJobName);
        
        }
    }
      
    /** Creates new form bb_gui */
    public bb_gui() {
        try {
            workDir=new File(".").getCanonicalPath();
            workDir ="/home/coolen/LOFAR/CEP/BB/BBS3/build/gnu_opt/";
            // take system this Demo runs on
            thisSystem = Runtime.getRuntime();
            initComponents();
            Class.forName("org.postgresql.Driver");
            // remove DBView from Tab container, will be replaced after there is something to plot
            bb_guiTabbedPane.remove(DBViewPanel);
            connectDBServer();
            ConfigPaneTextArea.setEditable(false);
            OutputPaneTextArea.setEditable(false);
        } catch(Exception e) {
            System.out.println("Error getting running system");
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        ButtonPanel = new javax.swing.JPanel();
        DemoSelection = new javax.swing.JComboBox();
        StartButton = new javax.swing.JButton();
        StopButton = new javax.swing.JButton();
        ViewButton = new javax.swing.JButton();
        DryRunCheckBox = new javax.swing.JCheckBox();
        bb_guiTabbedPane = new javax.swing.JTabbedPane();
        ConfigScrollPane = new javax.swing.JScrollPane();
        ConfigPaneTextArea = new javax.swing.JTextArea();
        OutputScrollPane = new javax.swing.JScrollPane();
        OutputPaneTextArea = new javax.swing.JTextArea();
        DBViewPanel = new javax.swing.JPanel();
        DBViewScrollPane = new javax.swing.JScrollPane();
        IterationLabel = new javax.swing.JLabel();
        IterationLabel.setVisible(false);
        IterationSelection = new javax.swing.JComboBox();
        IterationSelection.setVisible(false);
        DBViewSelection = new javax.swing.JComboBox();
        StartDBPlotButton = new javax.swing.JButton();
        ActiveProgramLabel = new javax.swing.JLabel();
        BBGuiMenuBar = new javax.swing.JMenuBar();
        FileMenu = new javax.swing.JMenu();
        FileMenuOpenFile = new javax.swing.JMenuItem();
        jSeparator1 = new javax.swing.JSeparator();
        FileMenuExit = new javax.swing.JMenuItem();

        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        ButtonPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        DemoSelection.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "SolvePoly", "SolveFlux", "SolveAP" }));
        DemoSelection.setToolTipText("Choose what database points you want to plot");
        ButtonPanel.add(DemoSelection, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 70, 180, -1));

        StartButton.setText("Start");
        StartButton.setToolTipText("Start the Demo");
        StartButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StartButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 140, -1, -1));

        StopButton.setText("Stop");
        StopButton.setToolTipText("Stop the demo");
        StopButton.setEnabled(false);
        StopButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StopButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StopButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 170, -1, -1));

        ViewButton.setText("View Map");
        ViewButton.setToolTipText("View Map");
        ViewButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ViewButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(ViewButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 250, -1, -1));

        DryRunCheckBox.setText("Dryrun?");
        DryRunCheckBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DryRunCheckBoxActionPerformed(evt);
            }
        });

        ButtonPanel.add(DryRunCheckBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(50, 100, -1, -1));

        getContentPane().add(ButtonPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, 240, 500));

        ConfigScrollPane.setViewportView(ConfigPaneTextArea);

        bb_guiTabbedPane.addTab("Config", ConfigScrollPane);

        OutputScrollPane.setViewportView(OutputPaneTextArea);

        bb_guiTabbedPane.addTab("Log", OutputScrollPane);

        DBViewPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        DBViewScrollPane.setPreferredSize(new java.awt.Dimension(100, 100));
        DBViewPanel.add(DBViewScrollPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(170, 10, 420, 450));

        IterationLabel.setText("Iterations ?");
        DBViewPanel.add(IterationLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 70, 90, 20));

        IterationSelection.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" }));
        IterationSelection.setToolTipText("Nr of Iterations to plot");
        DBViewPanel.add(IterationSelection, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 70, 50, -1));

        DBViewSelection.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "StokesI.CP1", "StokesI.CP2" }));
        DBViewSelection.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DBViewSelectionActionPerformed(evt);
            }
        });

        DBViewPanel.add(DBViewSelection, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, 150, -1));

        StartDBPlotButton.setText("Start Plot");
        StartDBPlotButton.setToolTipText("Start Selected DBView Plot");
        StartDBPlotButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartDBPlotButtonActionPerformed(evt);
            }
        });

        DBViewPanel.add(StartDBPlotButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, 110, -1));

        bb_guiTabbedPane.addTab("DBView", DBViewPanel);

        getContentPane().add(bb_guiTabbedPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 10, 600, 490));

        getContentPane().add(ActiveProgramLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 510, 600, -1));

        FileMenu.setText("File");
        FileMenu.setToolTipText("Access to fileMenu");
        FileMenuOpenFile.setText("Open Config File");
        FileMenuOpenFile.setToolTipText("Open the Configuration File");
        FileMenuOpenFile.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                FileMenuOpenFileActionPerformed(evt);
            }
        });

        FileMenu.add(FileMenuOpenFile);

        FileMenu.add(jSeparator1);

        FileMenuExit.setText("Exit");
        FileMenuExit.setToolTipText("Exit the Program");
        FileMenuExit.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                FileMenuExitActionPerformed(evt);
            }
        });

        FileMenu.add(FileMenuExit);

        BBGuiMenuBar.add(FileMenu);

        setJMenuBar(BBGuiMenuBar);

        pack();
    }
    // </editor-fold>//GEN-END:initComponents

    private void DryRunCheckBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DryRunCheckBoxActionPerformed
        dryRun=DryRunCheckBox.isSelected();
        if (dryRun) {
            itsDBName="coolen";
        } else {
            itsDBName="CDR";
        }
    }//GEN-LAST:event_DryRunCheckBoxActionPerformed

    private void ViewButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ViewButtonActionPerformed
        displayImage();
    }//GEN-LAST:event_ViewButtonActionPerformed

    private void StopButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StopButtonActionPerformed
        stopProgram();
        StartButton.setEnabled(true);
        DemoSelection.setEnabled(true);
        StopButton.setEnabled(false);
    }//GEN-LAST:event_StopButtonActionPerformed

    private void DBViewSelectionActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DBViewSelectionActionPerformed

        if (DBViewSelection.getSelectedIndex() > 1 && itsSelectedDemo.equals("SolvePoly")) {
            IterationLabel.setVisible(true);
            IterationSelection.setVisible(true);
        } else {
            IterationLabel.setVisible(false);
            IterationSelection.setVisible(false);
        }
        // Clear plotwindow.
        DBViewScrollPane.setViewportView(null);

    }//GEN-LAST:event_DBViewSelectionActionPerformed

    private void StartDBPlotButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartDBPlotButtonActionPerformed
        startPlot();
    }//GEN-LAST:event_StartDBPlotButtonActionPerformed

    private void StartButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartButtonActionPerformed
        runProgram();
        if (!tabAdded) {
            bb_guiTabbedPane.addTab("DBView", DBViewPanel);
            tabAdded=true;
        }
        StartButton.setEnabled(false);
        DemoSelection.setEnabled(false);
        StopButton.setEnabled(true);        
    }//GEN-LAST:event_StartButtonActionPerformed

    private void FileMenuExitActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FileMenuExitActionPerformed
        try {
            if (!itsDatabase.isClosed()) {
                itsDatabase.close();        
            }
            if (itsStatement != null) {
                itsStatement.close();
            }
            if (itsResultSet != null) {
                itsResultSet.close();
            }
            
        } catch (SQLException ex) {
            System.out.println("Error closing DB connection: " + ex);
            System.exit(1);
        }
        System.exit(0);
    }//GEN-LAST:event_FileMenuExitActionPerformed

    private void FileMenuOpenFileActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FileMenuOpenFileActionPerformed
        bb_guiTabbedPane.setSelectedIndex(0);
        readFile();
    }//GEN-LAST:event_FileMenuOpenFileActionPerformed
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new bb_gui().setVisible(true);
            }
        });
    }
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel ActiveProgramLabel;
    private javax.swing.JMenuBar BBGuiMenuBar;
    private javax.swing.JPanel ButtonPanel;
    private javax.swing.JTextArea ConfigPaneTextArea;
    private javax.swing.JScrollPane ConfigScrollPane;
    private javax.swing.JPanel DBViewPanel;
    private javax.swing.JScrollPane DBViewScrollPane;
    private javax.swing.JComboBox DBViewSelection;
    private javax.swing.JComboBox DemoSelection;
    private javax.swing.JCheckBox DryRunCheckBox;
    private javax.swing.JMenu FileMenu;
    private javax.swing.JMenuItem FileMenuExit;
    private javax.swing.JMenuItem FileMenuOpenFile;
    private javax.swing.JLabel IterationLabel;
    private javax.swing.JComboBox IterationSelection;
    private javax.swing.JTextArea OutputPaneTextArea;
    private javax.swing.JScrollPane OutputScrollPane;
    private javax.swing.JButton StartButton;
    private javax.swing.JButton StartDBPlotButton;
    private javax.swing.JButton StopButton;
    private javax.swing.JButton ViewButton;
    private javax.swing.JTabbedPane bb_guiTabbedPane;
    private javax.swing.JSeparator jSeparator1;
    // End of variables declaration//GEN-END:variables

    
    private void readFile() {
        itsSelectedFile = "None";
        JFileChooser fc = new JFileChooser();
        fc.addChoosableFileFilter(new configFilter());
        int returnVal = fc.showOpenDialog(bb_gui.this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            File aFile = fc.getSelectedFile();
            itsSelectedFile=aFile.getName();
            try {
                BufferedReader in = new BufferedReader(new FileReader(aFile));
                String aStr;
                while ((aStr = in.readLine()) != null) {
                    ConfigPaneTextArea.append(aStr+"\n");
                }
                in.close();                     
            } catch (IOException e) {
                System.out.println("Error reading from file: "+aFile.getName());
                System.exit(1);
            }
        } else {
              JOptionPane.showMessageDialog(null,"You didn't select a file",
                      "Config selection warning",
                      JOptionPane.WARNING_MESSAGE);
        }
       ConfigPaneTextArea.setEditable(false);
    }
    
    private void changeIterationSelection() {
        try {
            IterationSelection.removeAllItems();
            if (itsSelectedDemo.equals("SolvePoly")) {
                String aQ="Select iteration from "+itsTableName[itsSelectedPlotType]+" where parmname='"+DBViewSelection.getSelectedItem().toString()+"';";
                if (doQuery(aQ) ) {
                    int i=0;
                    while (itsResultSet.next()) {
                        IterationSelection.addItem( String.valueOf(++i));
                    }
                } else {
                    System.out.println("Query "+aQ+"returned an Error");
                }
            }
         } catch (SQLException ex) {
            System.out.println("Error querying Database: "+ ex);
            System.exit(1);
         } 
    }
    
    private void changePlotPoints(int aT) {
        // set new values that can be plotted
        DBViewSelection.removeAllItems();
        switch (aT) {
            case 0:
                DBViewSelection.addItem("Fit");
                DBViewSelection.addItem("Rank");                
                DBViewSelection.addItem("StokesI.CP1");
                DBViewSelection.addItem("StokesI.CP2");
                break;
            case 1:
                DBViewSelection.addItem("Fit");
                DBViewSelection.addItem("Rank");                
                DBViewSelection.addItem("StokesI.CP1");
                DBViewSelection.addItem("StokesI.CP2");
                break;
            case 2:
                DBViewSelection.addItem("Fit");
                DBViewSelection.addItem("Rank");                
                DBViewSelection.addItem("EJ11.phase.SR2.SG1");
                DBViewSelection.addItem("EJ22.phase.SR2.SG1");
                DBViewSelection.addItem("EJ11.phase.SR3.SG1");
                DBViewSelection.addItem("EJ22.phase.SR3.SG1");
                DBViewSelection.addItem("EJ11.phase.SR4.SG1");
                DBViewSelection.addItem("EJ22.phase.SR4.SG1");
                DBViewSelection.addItem("EJ11.phase.SR5.SG1");
                DBViewSelection.addItem("EJ22.phase.SR5.SG1");
                DBViewSelection.addItem("EJ11.phase.SR6.SG1");
                DBViewSelection.addItem("EJ22.phase.SR6.SG1");
                DBViewSelection.addItem("EJ11.phase.SR7.SG1");
                DBViewSelection.addItem("EJ22.phase.SR7.SG1");
                DBViewSelection.addItem("EJ11.phase.SR8.SG1");
                DBViewSelection.addItem("EJ22.phase.SR8.SG1");
                DBViewSelection.addItem("EJ11.phase.SR9.SG1");
                DBViewSelection.addItem("EJ22.phase.SR9.SG1");
                DBViewSelection.addItem("EJ11.phase.SR10.SG1");
                DBViewSelection.addItem("EJ22.phase.SR10.SG1");
                DBViewSelection.addItem("EJ11.phase.SR11.SG1");
                DBViewSelection.addItem("EJ22.phase.SR11.SG1");
                DBViewSelection.addItem("EJ11.phase.SR12.SG1");
                DBViewSelection.addItem("EJ22.phase.SR12.SG1");
                DBViewSelection.addItem("EJ11.phase.SR13.SG1");
                DBViewSelection.addItem("EJ22.phase.SR13.SG1");
                DBViewSelection.addItem("EJ11.phase.SR14.SG1");
                DBViewSelection.addItem("EJ22.phase.SR14.SG1");
                break;
        } 
    }
    
   
    private void startPlot() {
        // Clear plotwindow.
        DBViewScrollPane.setViewportView(null);
        itsPlot = new Plot();
        if (DBViewSelection.getSelectedItem().toString().equals("Fit")) {
            plotFit();
        } else if (DBViewSelection.getSelectedItem().toString().equals("Rank")) {
            plotRank();
        } else {
            if (itsSelectedDemo.equals("SolvePoly")) {
                plotPolynome();
            } else if (itsSelectedDemo.equals("SolveFlux")) {
                plotFlux();
            } else {
                plotPhase();
            }                        
        }

        // insert new plot
        DBViewScrollPane.setViewportView(itsPlot);
        
     }

      private boolean stopRunningProgram() {
        String s1 = "Yes";
        String s2 = "No";
        Object[] options = {s1, s2};
        int n = JOptionPane.showOptionDialog(this,
                "The last test is still Running\nDo you really want to start a new one?",
                "Start Confirmation",
                JOptionPane.YES_NO_OPTION,
                JOptionPane.QUESTION_MESSAGE,
                null,
                options,
                s2);
        if (n == JOptionPane.YES_OPTION) {
            return true;
        } else {
            return false;
        }
    }  
    
     private void runProgram() {
         boolean startNew=true;

         if (!dryRun) {
             if (itsProgramThread != null) {
                 if (!itsProgramThread.isStopped()) {
                    if (stopRunningProgram()) {
                         stopProgram();
                    } else {
                         DemoSelection.setSelectedIndex(itsSelectedPlotType);
                         startNew=false;
                    }
                 }
             }
         }
         
         if (startNew) {
             itsSelectedDemo=DemoSelection.getSelectedItem().toString();
             itsSelectedPlotType=DemoSelection.getSelectedIndex();
             changePlotPoints(DemoSelection.getSelectedIndex());
             if (itsWaitThread!=null) itsWaitThread=null;
             
             if (!dryRun) {
                 itsWaitThread=new WaitForFileThread();
                 itsWaitThread.setJobTerminationName(itsSelectedDemo);
                 itsWaitThread.start();        
             }
             OutputPaneTextArea.setText("");
         }
     }
     
     private void stopProgram() {
         if (!dryRun) {
             if (itsProgramThread != null) {
                 if (!itsProgramThread.isStopped()) {
                    itsProgramThread.stopThread();
                 }
                 ActiveProgramLabel.setText("Busy trying to stop: "+itsSelectedDemo);
                 ActiveProgramLabel.updateUI();
                 setCursor(hourglassCursor);           
                 while (!itsProgramThread.isStopped());
                 setCursor(normalCursor);
                 removeReadyFile(itsSelectedDemo+".ready");
                 itsProgramThread.interrupt();
                 itsProgramThread=null;
             }
         }
         ActiveProgramLabel.setText(itsSelectedDemo + " Stopped");         
     }
     
     private void connectDBServer() {
         try {
            itsDatabase = DriverManager.getConnection("jdbc:postgresql://dop50/"+itsDBName, "postgres", "");
         } catch (SQLException ex) {
            System.out.println("Error opening database: "+ ex);
            System.exit(1);
         } 
     }
     
     private boolean doQuery(String aQuery) {
         try {
//             System.out.println("Query: "+ aQuery);
             if (itsDatabase != null) {
                if (itsResultSet != null) itsResultSet.close();
                if (itsStatement != null) itsStatement.close();
                itsStatement = itsDatabase.createStatement();
                itsResultSet = itsStatement.executeQuery(aQuery);
                return true;
             } else {
                System.out.println("Error doing a query, db connection wasn't open.");
                System.exit(1);
             }
         } catch (SQLException ex) {
             System.out.println("Error making a query: "+ex);
             System.exit(1);
        } 
        return false;
     }
     
     private void plotPolynome() {
        // connect to db and get 2 or 4 new values
        String aQ="Select coeff0,coeff1,coeff2,coeff3 from "+itsTableName[itsSelectedPlotType]+" where parmname='"+DBViewSelection.getSelectedItem().toString()+"' order by iteration;";
        if (doQuery(aQ)) {
            int set=0;
            try {
                while (itsResultSet.next() && set <= IterationSelection.getSelectedIndex()) {
                    double c[] = {0.0,0.0,0.0,0.0};
                    set++;
            
                    for (int i=0;i<4;i++){
                        c[i]=itsResultSet.getDouble(i+1);
                    }
                    double aTickStep=(1.-0.)/50.;

                    itsPlot.setXRange(1.7,1.8);
                    itsPlot.setButtons(true);
                    itsPlot.setTitle("Plot Polynome");
                    itsPlot.setXLabel("Frequency in Ghz");
                    itsPlot.setYLabel("Flux");
                    itsPlot.setMarksStyle("none");
                    itsPlot.setConnected(true);

                    double xStep=(1.8-1.7)/50;
                    double startX=1.7-xStep;
                    boolean first=true;
                    itsPlot.addLegend(set,"Set"+String.valueOf(set));
                    for (double aFreq=0;aFreq <= 1;aFreq+=aTickStep) {
                        itsPlot.addPoint(set,startX+=xStep,getFlux(c,aFreq),!first);
                        first=false;
                    }
                }
            } catch (SQLException ex) {
                System.out.println("Error making a Polynome plot: "+ex);
                System.exit(1);
            }
        } else {
            System.out.println("Query "+aQ+"returned an Error");
        }
     }
     
     private void plotPhase() {
         // get the highest iteration number from table
         String aQ;
         int lastIter=-1;
         aQ="Select iteration from "+itsTableName[itsSelectedPlotType]+" where parmname='" + DBViewSelection.getItemAt(2).toString()+"' order by iteration desc limit 1;";
         try {
            if (doQuery(aQ)) {
                while (itsResultSet.next()) {
                    lastIter=itsResultSet.getInt(1);
                }
            } else {
                System.out.println("Query "+aQ+"returned an Error");
            }
                 
             if (lastIter > -1) {
                aQ="Select coeff0 from "+itsTableName[itsSelectedPlotType]+" where parmname='"+ DBViewSelection.getSelectedItem().toString()+"' and iteration = " + lastIter + " order by endtime;";
                if (doQuery(aQ)) {
                    int index=1;
                    double c=0.0;
                    itsPlot.setButtons(true);
                    itsPlot.setTitle("Plot Phase");
                    itsPlot.setXLabel("Time Steps");
                    itsPlot.setYLabel("Phase");
                    itsPlot.setMarksStyle("none");
                    itsPlot.setConnected(true);
                    boolean first=true;
                    while (itsResultSet.next()) {
                        c = itsResultSet.getDouble(1);
                        itsPlot.addPoint(0,index,c,!first);
                        index++;
                        first=false;
                    }
                } else {
                    System.out.println("Query "+aQ+"returned an Error");
                }
            } else {
                System.out.println("ERROR: No lastIter found, can't get phases");
            }
         } catch (SQLException ex) {
            System.out.println("Error making a  fit plot: "+ex);
            System.exit(1);
         }        
     }
     
     private void plotFit() {
         String aQ="Select fit from "+itsTableName[itsSelectedPlotType]+" where parmname='"+ DBViewSelection.getItemAt(2).toString()+"' order by iteration,endtime;";
         if (doQuery(aQ)) {
             int index=1;
             double c=0.0;
             itsPlot.setButtons(true);
             itsPlot.setTitle("Plot Fit");
             if (itsSelectedDemo.equals("SolveAP")) {
                 itsPlot.setXLabel("All TimeSteps - All Iterations");
             } else {
                 itsPlot.setXLabel("Iterations");                 
             }
             itsPlot.setYLabel("Fit");
             itsPlot.setMarksStyle("none");
             itsPlot.setConnected(true);
             boolean first=true;
             try {
                while (itsResultSet.next()) {
                    c = itsResultSet.getDouble(1);
                    itsPlot.addPoint(0,index,c,!first);
                    index++;
                    first=false;
                }
             } catch (SQLException ex) {
                System.out.println("Error making a  fit plot: "+ex);
                System.exit(1);
             }
        } else {
            System.out.println("Query "+aQ+"returned an Error");
        }
     }
     
     private void plotRank() {
         String aQ="Select rank from "+itsTableName[itsSelectedPlotType]+" where parmname='"+ DBViewSelection.getItemAt(2).toString()+"' order by iteration;";
         if (doQuery(aQ) ) {
            int index=1;
            double c=0.0;
            itsPlot.setButtons(true);
            itsPlot.setTitle("Plot Rank");
            itsPlot.setXLabel("Iterations");
            itsPlot.setYLabel("Rank");
            itsPlot.setMarksStyle("none");
            itsPlot.setConnected(true);
            boolean first=true;
            try {
                while (itsResultSet.next()) {
                    c = itsResultSet.getDouble(1);
                    itsPlot.addPoint(0,index,c,!first);
                    index++;
                    first=false;
                }
            } catch (SQLException ex) {
                System.out.println("Error making a rank plot: "+ex);
            }
        } else {
            System.out.println("Query "+aQ+"returned an Error");
        }
     }
     
     private void plotFlux() {
         String aQ="Select coeff0 from "+itsTableName[itsSelectedPlotType]+" where parmname='"+ DBViewSelection.getSelectedItem().toString()+"' order by iteration;";
         if (doQuery(aQ) ) {
            int index=1;
            double c=0.0;
            itsPlot.setButtons(true);
            itsPlot.setTitle("Plot Flux");
            itsPlot.setXLabel("Iterations");
            itsPlot.setYLabel("Flux");
            itsPlot.setMarksStyle("none");
            itsPlot.setConnected(true);
            boolean first=true;
            try {
                while (itsResultSet.next()) {
                    c = itsResultSet.getDouble(1);
                    itsPlot.addPoint(0,index,c,!first);
                    index++;
                    first=false;
                }
            } catch (SQLException ex) {
                System.out.println("Error making a rank plot: "+ex);
                System.exit(1);
            }
        } else {
            System.out.println("Query "+aQ+"returned an Error");
        }
     }
     
     private double getFlux(double [] c, double freq) {
         double val=c[0];
         double f=freq;
         for (int i=1; i < c.length; i++) {
             if (c[i] != 0.0) {
                 val=val+(c[i]*f);
                 f=f*freq;
             }
         }
         return val;
     }
     
      public void waitReadyFiles(WaitForFileThread aThread,String aJobName) {
        // wait until the file aJobName.ready is available
        boolean isReady = false;
        while (! isReady) {
            File file1 = new File(workDir,aJobName+".ready");
            if (file1.exists()) {
                ActiveProgramLabel.setText(aJobName+" finished");
                isReady=true;
            }
            try {
                aThread.sleep(100);
            } catch (InterruptedException ex) {
            }
        }
    }   
      
    private void removeReadyFile(String aName) {
        File aFile = new File(workDir,aName);
        if (aFile.exists()) {
            aFile.delete();
        }        
    } 
    
    private void displayImage() {
        itsSelectedFile = "None";
        JFileChooser fc = new JFileChooser();
        int returnVal = fc.showOpenDialog(bb_gui.this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            new Viewer(fc.getSelectedFile().getAbsolutePath());
        } else {
              JOptionPane.showMessageDialog(null,"You didn't select a file",
                      "Config selection warning",
                      JOptionPane.WARNING_MESSAGE);
        }        
    }
}